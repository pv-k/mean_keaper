#include "PendingConsumer.h"
#include "consts.h"
#include <algorithm>
#include <boost\foreach.hpp>
#include <math.h>

ResourceConsumer* MK_FREE_RESOURCE_OWNER = 0;

bool is_zero(double x)
{
	return abs(x)<MK_TOLERANCE;
}

void PendingConsumer::update_priority()
{	
	priority.mean_value = ( is_zero(resources_count) ? MK_INFINITY : resources_sum / resources_count );
	priority.id = get_base_id();
}

priority_pair PendingConsumer::get_priority()
{
	return priority;
}

PendingConsumer::PendingConsumer(ResourceConsumer* base_consumer, const priority_pair& priority_lower_bound, 
	std::unordered_set<ResourceConsumer*> known_higher_priority_consumers, bool is_approximate)
{
	this->base_consumer = base_consumer;
	this->is_approximate = is_approximate;
	resources_count = 0;
	resources_sum = 0;
	if (is_approximate)
		BOOST_FOREACH(Resource* resource, base_consumer->owned_resources)
		{
			this->resources_count += resource->resources_count;
			this->resources_sum += resource->resources_value;
		}
	else
		BOOST_FOREACH(Resource* resource, base_consumer->asked_resources)
		{
			ResourceConsumer* resource_owner = resource->get_owner();
			if ( resource->is_free() || ( known_higher_priority_consumers.find(resource_owner) == known_higher_priority_consumers.end() ) && 
				!(priority_lower_bound < resource_owner->get_first_priority() ) )
			{
				this->resources_count += resource->resources_count;
				this->resources_sum += resource->resources_value;
				available_resources_owners[resource->get_owner()].insert(resource);
			}
		}

	update_priority();
}

void set_diff(std::set<Resource*>& resources1, std::set<Resource*>& resources2, std::vector<Resource*>& copy_to)
{
	std::set_difference(resources1.begin(), resources1.end(), resources2.begin(), resources2.end(), std::inserter(copy_to, copy_to.end()) );
}

void PendingConsumer::ToRegularConsumer(ResourceConsumer*& out_consumer, const priority_pair& priority_bound,
	std::vector<Resource*>& lost_cells, std::vector<Resource*>& acquired_cells)
{
	out_consumer = base_consumer;
	if (is_approximate)
	{
		out_consumer->priority.update( get_priority(), get_priority() );
	}
	else
	{
		std::set<Resource*> new_owned_resources;
		for (std::hash_map<ResourceConsumer*,std::unordered_set<Resource*> >::iterator available_resources_owners_iter = available_resources_owners.begin();
			available_resources_owners_iter != available_resources_owners.end();
			available_resources_owners_iter++)
			BOOST_FOREACH (Resource* resource, available_resources_owners_iter->second)
				new_owned_resources.insert(resource);
	
		if ( get_priority() < priority_bound )
			out_consumer->priority.update( get_priority(), get_priority() );
		else
			out_consumer->priority.update( priority_bound, get_priority() );

		set_diff(base_consumer->owned_resources, new_owned_resources, lost_cells);
		set_diff(new_owned_resources, base_consumer->owned_resources, acquired_cells);
		out_consumer->owned_resources = new_owned_resources;
		delete this;
	}
}

bool PendingConsumer::remove_resource( Resource* resource )
{
	std::hash_map<ResourceConsumer*,std::unordered_set<Resource*> >::iterator resource_owner_iter = available_resources_owners.find(resource->get_owner());
	if ( resource_owner_iter != available_resources_owners.end() )
	{
		std::unordered_set<Resource*>* consumer_resources = &resource_owner_iter->second;
		if ( consumer_resources->find(resource) != consumer_resources->end() )
		{
			consumer_resources->erase(resource);
			if (consumer_resources->empty())
				available_resources_owners.erase(resource->get_owner());
			resources_count -= resource->resources_count;
			resources_sum -= resource->resources_value;
			update_priority();
			return true;
		}
		else
			return false;
	}
	else
		return false;
}

bool PendingConsumer::add_resource( Resource* resource )
{
	resources_count += resource->resources_count;
	resources_sum += resource->resources_value;
	available_resources_owners[resource->get_owner()].insert(resource);
	update_priority();
	return true;
}

bool PendingConsumer::free_owner_resources( ResourceConsumer* consumer )
{
	std::hash_map<ResourceConsumer*,std::unordered_set<Resource*> >::iterator resource_owner_iter = available_resources_owners.find(consumer);
	if ( resource_owner_iter != available_resources_owners.end() )
	{
		std::unordered_set<Resource*>& free_resources = available_resources_owners[MK_FREE_RESOURCE_OWNER];
		BOOST_FOREACH (Resource* resource, resource_owner_iter->second)
			free_resources.insert(resource);
		available_resources_owners.erase(resource_owner_iter);
		return true;
	}
	else
		return false;
}

bool PendingConsumer::remove_consumer_resources( ResourceConsumer* consumer )
{
	std::hash_map<ResourceConsumer*,std::unordered_set<Resource*> >::iterator resource_owner_iter = available_resources_owners.find(consumer);
	if ( resource_owner_iter != available_resources_owners.end() )
	{
		BOOST_FOREACH (Resource* resource, resource_owner_iter->second)
		{
			resources_count -= resource->resources_count;
			resources_sum -= resource->resources_value;
		}
		available_resources_owners.erase(resource_owner_iter);
		update_priority();
		return true;
	}
	else
		return false;
}
	
double PendingConsumer::get_base_id()
{
	return base_consumer->id;
}

bool PendingConsumer::empty()
{
	return available_resources_owners.empty();
}

std::unordered_set<ResourceConsumer*> PendingConsumer::get_owners()
{
	std::unordered_set<ResourceConsumer*> owners;
	for (std::hash_map<ResourceConsumer*,std::unordered_set<Resource*> >::iterator available_resources_owners_iter = available_resources_owners.begin();
		available_resources_owners_iter != available_resources_owners.end();
		available_resources_owners_iter++)
		owners.insert(available_resources_owners_iter->first);
	owners.erase(0);
	return owners;
}

bool PendingConsumer::has_owner( ResourceConsumer* consumer )
{
	std::hash_map<ResourceConsumer*,std::unordered_set<Resource*> >::iterator resource_owner_iter = available_resources_owners.find(consumer);
	if ( resource_owner_iter == available_resources_owners.end() )
		return false;
	else
		return true;
}