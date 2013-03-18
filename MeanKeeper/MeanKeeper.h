#pragma once
#include <map>
#include <hash_map>
#include <unordered_set>
#include <set>
#include <vector>
#include "Resource.h"
#include "consts.h"
#include <functional>
#include <boost\foreach.hpp>
#include "priority_pair.h"
#include "complex_priority.h"
#include "ResourceConsumer.h"
#include "PendingConsumer.h"
#include "my_linked_list.h"
#include <iostream>

#define MK_NOT_REMOVED_RESOURCE 0

#define MK_REGULAR_CONSUMER 0
#define MK_PENDING_CONSUMER 1

template<class TKey1, class TKey2>
class bimap
{
public:
	std::hash_map<TKey1, TKey2> forward_map;
	std::hash_map<TKey2, TKey1> backward_map;

	void insert(TKey1 key1, TKey2 key2)
	{
		forward_map[key1] = key2;
		backward_map[key2] = key1;
	}

	void erase_forward(TKey1 key1)
	{
		TKey2 key2 = forward_map[key1];
		forward_map.erase(key1);
		backward_map.erase(key2);
	}
	
	void erase_backward(TKey2 key2)
	{
		TKey1 key1 = backward_map[key2];
		forward_map.erase(key1);
		backward_map.erase(key2);
	}

	void clear()
	{
		backward_map.clear();
		forward_map.clear();
	}
};

struct PendingConsumerMoreImportant {
    bool operator()(PendingConsumer* left, PendingConsumer* right)
	{
		return left->get_priority() > right->get_priority();
    }
};

struct ConsumerMoreImportant {
    bool operator()(ResourceConsumer* left, ResourceConsumer* right) 
	{
		return left->has_higher_priority( right );
    }
};

struct PriorityPairMoreImportant {
    bool operator()(const priority_pair& left, const priority_pair& right) 
	{
		return left > right;
    }
};

class ResourcesOwnersData
{
public:
	std::map<priority_pair, std::unordered_set<ResourceConsumer*>, PriorityPairMoreImportant> resources_owners;
	std::hash_map<ResourceConsumer*, std::unordered_set<PendingConsumer*> > owner_consumers_map;
	std::hash_map<PendingConsumer*, std::unordered_set<ResourceConsumer*> > consumer_owners_map;

	void AddConsumer(PendingConsumer* consumer)
	{
		std::unordered_set<ResourceConsumer*> owners = consumer->get_owners();
		BOOST_FOREACH(ResourceConsumer* owner, owners)
		{
			resources_owners[owner->get_first_priority()].insert(owner);
			owner_consumers_map[owner].insert(consumer);
		}
		consumer_owners_map[consumer] = owners;
	}

	void RemoveConsumer(PendingConsumer* consumer)
	{
		std::unordered_set<ResourceConsumer*> owners = consumer_owners_map[consumer];
		BOOST_FOREACH(ResourceConsumer* owner, owners)
			owner_consumers_map[owner].erase(consumer);
		consumer_owners_map.erase(consumer);
	}

	bool ContainsOwner(ResourceConsumer* owner)
	{
		std::map<priority_pair, std::unordered_set<ResourceConsumer*>, PriorityPairMoreImportant >::iterator owner_priority_iter = resources_owners.find(owner->get_first_priority());
		if ( owner_priority_iter == resources_owners.end() )
			return false;

		std::unordered_set<ResourceConsumer*>::iterator owner_iter = owner_priority_iter->second.find(owner);
		if (owner_iter == owner_priority_iter->second.end())
			return false;

		return true;
	}

	ResourceConsumer* GetFirstPriorityOwner()
	{
		while ( !resources_owners.empty() )
		{
			ResourceConsumer* first_priority_owner =*resources_owners.begin()->second.begin();
			if ( owner_consumers_map[first_priority_owner].empty() )
				RemoveOwner(first_priority_owner);
			else
				return first_priority_owner;
		}
		return 0;
	}

	void RemoveOwner(ResourceConsumer* owner)
	{
		std::map<priority_pair, std::unordered_set<ResourceConsumer*>, PriorityPairMoreImportant>::iterator owner_priority_iter = 
			resources_owners.find(owner->get_first_priority());
		if ( owner_priority_iter == resources_owners.end() )
			return;

		std::unordered_set<ResourceConsumer*>::iterator owner_iter = owner_priority_iter->second.find(owner);
		if (owner_iter == owner_priority_iter->second.end())
			return;

		owner_priority_iter->second.erase(owner_iter);
		if ( owner_priority_iter->second.empty() )
			resources_owners.erase(owner_priority_iter);

		std::unordered_set<PendingConsumer*> consumers = owner_consumers_map[owner];
		BOOST_FOREACH(PendingConsumer* consumer, consumers)
			consumer_owners_map[consumer].erase(owner);
		owner_consumers_map.erase(owner);
	}
};

class PriorityUpperBound
{
public:
	priority_pair priority;
	std::unordered_set<ResourceConsumer*> higher_priority_consumers;

	void update(ResourceConsumer* consumer)
	{
		if ( !consumer->get_first_priority().has_equal_priority(priority) )
		{
			higher_priority_consumers.clear();
			priority = consumer->get_first_priority();
		}
		higher_priority_consumers.insert(consumer);
	}

	bool has_higher_priority(ResourceConsumer* consumer) const
	{
		if ( consumer->get_first_priority().has_equal_priority(priority) )
			if ( higher_priority_consumers.find(consumer) == higher_priority_consumers.end() )
				return true;
			else
				return false;
		else
			return consumer->get_first_priority().has_lower_priority(priority);
	}

	bool has_lower_priority(ResourceConsumer* consumer)
	{
		if ( consumer->get_first_priority().has_equal_priority(priority) )
			if ( higher_priority_consumers.find(consumer) == higher_priority_consumers.end() )
				return false;
			else
				return true;
		else
			return consumer->get_first_priority().has_higher_priority(priority);
	}
};

template <class ResourceIDType, class ConsumerIDType>
class MeanKeeper
{
private:
	PendingConsumer* RegularToPendingConsumer(ResourceConsumer* consumer, 
		const PriorityUpperBound& priority_upper_bound, std::unordered_set<PendingConsumer*>& updated_consumers, ResourcesOwnersData& resources_owners_data );						
	void AddPendingConsumer(ResourceConsumer* consumer,	const PriorityUpperBound& priority_upper_bound, std::unordered_set<PendingConsumer*>& updated_consumers, 
		ResourcesOwnersData& resources_owners_data);
	void FreeConsumerResources(ResourceConsumer* consumer, PendingConsumer* pending_consumer,
		const PriorityUpperBound& priority_upper_bound, ResourcesOwnersData& resources_owners_data);
	void AddRegularConsumer(ConsumerIDType consumer_id, ResourceConsumer* consumer);
	void ToRegularConsumer(PendingConsumer* consumer, PriorityUpperBound& priority_upper_bound, 
		my_linked_list_node<ResourceConsumer*>* insert_after, ResourcesOwnersData& resources_owners_data, std::vector<ConsumerIDType>& update_changed_consumers);
	void SetResourceState(Resource* resource, ResourceConsumer* owner, std::unordered_set<PendingConsumer*>& updated_consumers);
	PendingConsumer* GetPendingConsumer(ResourceConsumer* consumer);
	void UpdateQueue(std::unordered_set<PendingConsumer*>& new_consumers);
	void AddConsumer(ConsumerIDType consumer_id, std::set<ResourceIDType>& asked_resources, ResourcesOwnersData& resources_owners_data);
	void RemoveConsumers(std::set<ConsumerIDType> removed_consumers_ids, ResourcesOwnersData& resources_owners_data);

	int total_num_consumers;
	int total_num_resources;

	bimap<ResourceIDType, Resource*> resources;
	bimap<ConsumerIDType, ResourceConsumer*> consumers;

	my_linked_list<ResourceConsumer*> ordered_consumers_list;
	std::map<priority_pair, ResourceConsumer*, std::greater<priority_pair> > ordered_consumers_map;
	std::set<PendingConsumer*, PendingConsumerMoreImportant> pending_consumers_queue;

	bool approximate_state;
public:
	MeanKeeper(void);
	~MeanKeeper(void);

	void MakeApproximate();
	void ClearConsumers();
	double GetMeanValue(ConsumerIDType consumer);
	bool OwnsResource(ConsumerIDType consumer, ResourceIDType resource);
	void AddResource(ResourceIDType resource_cell, double resources_count, double resources_value);
	std::vector<ConsumerIDType> UpdateState(std::set<ConsumerIDType>& removed_consumers, std::vector< std::pair<ConsumerIDType, std::set<ConsumerIDType> > >& new_consumers);
	void RemoveResource(ResourceIDType resource_cell);
};

template <class ResourceIDType, class ConsumerIDType>
void MeanKeeper<ResourceIDType, ConsumerIDType>::MakeApproximate()
{
	approximate_state = true;
}

template <class ResourceIDType, class ConsumerIDType>
MeanKeeper<ResourceIDType, ConsumerIDType>::MeanKeeper(void)
{
	approximate_state = false;
	total_num_consumers = 0;
	total_num_resources = 0;
}

template <class ResourceIDType, class ConsumerIDType>
MeanKeeper<ResourceIDType, ConsumerIDType>::~MeanKeeper(void)
{
	ClearConsumers();

	std::set<ResourceIDType> removed_resources;
	for (std::hash_map<ResourceIDType, Resource*>::iterator resources_ids_iter = resources.forward_map.begin();
		resources_ids_iter != resources.forward_map.end();
		resources_ids_iter++)
		removed_resources.insert(resources_ids_iter->first);

	BOOST_FOREACH ( ResourceIDType resource_id, removed_resources)
		RemoveResource(resource_id);
}

template <class ResourceIDType, class ConsumerIDType>
void MeanKeeper<ResourceIDType, ConsumerIDType>::ClearConsumers()
{
	std::set<ConsumerIDType> removed_consumers;
	for (std::hash_map<ConsumerIDType, ResourceConsumer*>::iterator consumers_ids_iter = consumers.forward_map.begin();
		consumers_ids_iter != consumers.forward_map.end();
		consumers_ids_iter++)
		removed_consumers.insert(consumers_ids_iter->first);

	BOOST_FOREACH ( ConsumerIDType consumer_id, removed_consumers)
	{
		ResourceConsumer* removed_consumer = consumers.forward_map[consumer_id];
		consumers.erase_forward(consumer_id);
		delete removed_consumer;
	}

	ordered_consumers_list.clear();
	ordered_consumers_map.clear();

	for ( std::hash_map<ResourceIDType,Resource*>::iterator resources_iter = resources.forward_map.begin();
							resources_iter != resources.forward_map.end();
							resources_iter++)
	{
		Resource* resource = resources_iter->second;
		resource->set_owner(0);
		resource->consumers.clear();
	}
	total_num_consumers = 0;
}

template <class ResourceIDType, class ConsumerIDType>
double MeanKeeper<ResourceIDType, ConsumerIDType>::GetMeanValue(ConsumerIDType consumerID)
{
	return consumers.forward_map[consumerID]->priority.first_priority.mean_value;
}

template <class ResourceIDType, class ConsumerIDType>
bool MeanKeeper<ResourceIDType, ConsumerIDType>::OwnsResource(ConsumerIDType consumerID, ResourceIDType resourceID)
{
	ResourceConsumer* consumer = consumers.forward_map[consumerID];
	return consumer->owned_resources.find(resources.forward_map[resourceID]) != consumer->owned_resources.end();
	return true;
}

template <class ResourceIDType, class ConsumerIDType>
void MeanKeeper<ResourceIDType, ConsumerIDType>::AddResource(ResourceIDType resource_cell_ID, double resources_count, double resources_value)
{
	Resource* cell = new Resource();
	cell->resources_count = resources_count;
	cell->resources_value = resources_value;
	total_num_resources++;
	cell->id = total_num_resources;
	resources.insert( resource_cell_ID, cell);
}

template <class ResourceIDType, class ConsumerIDType>
void MeanKeeper<ResourceIDType, ConsumerIDType>::SetResourceState(Resource* resource, ResourceConsumer* owner, std::unordered_set<PendingConsumer*>& updated_consumers)
{
	if ( resource->is_free() && owner == 0 || resource->get_owner()== owner)
		return;

	BOOST_FOREACH(ResourceConsumer* consumer, resource->consumers)
	{
		PendingConsumer* pending_consumer = consumer->pending_consumer;

		if ( pending_consumer != 0)
		{
			if (updated_consumers.find(pending_consumer) == updated_consumers.end() )
			{
				pending_consumers_queue.erase(pending_consumer);
				updated_consumers.insert(pending_consumer);
			}
			pending_consumer->remove_resource(resource);
		}
	}

	resource->set_owner(owner);

	if ( resource->is_free() )
		BOOST_FOREACH(ResourceConsumer* consumer, resource->consumers)
		{
			PendingConsumer* pending_consumer = consumer->pending_consumer;
			if (pending_consumer!=0)
				pending_consumer->add_resource(resource);
		}
}

template <class ResourceIDType, class ConsumerIDType>
void MeanKeeper<ResourceIDType, ConsumerIDType>::UpdateQueue(std::unordered_set<PendingConsumer*>& new_consumers)
{
	BOOST_FOREACH(PendingConsumer* consumer, new_consumers)
		pending_consumers_queue.insert(consumer);
}

template <class ResourceIDType, class ConsumerIDType>
PendingConsumer* MeanKeeper<ResourceIDType, ConsumerIDType>::RegularToPendingConsumer(ResourceConsumer* consumer, 
	const PriorityUpperBound& priority_upper_bound, std::unordered_set<PendingConsumer*>& updated_consumers, ResourcesOwnersData& resources_owners_data )
{
	resources_owners_data.RemoveOwner(consumer);
	if (consumer->main_list_node != 0)
	{
		ordered_consumers_list.remove(consumer->main_list_node);
		if (consumer->get_first_priority() == consumer->get_second_priority())
			ordered_consumers_map.erase(consumer->get_first_priority());
		consumer->main_list_node = 0;
	}

	std::vector<Resource*> updated_resources;
	BOOST_FOREACH(Resource* resource, consumer->owned_resources)
	{
		updated_resources.push_back(resource);
		SetResourceState(resource, 0, updated_consumers);
	}

	PendingConsumer* pending_consumer = new PendingConsumer(consumer, priority_upper_bound.priority, 
		priority_upper_bound.higher_priority_consumers, false );
	consumer->pending_consumer = pending_consumer;
	resources_owners_data.AddConsumer(pending_consumer);
	pending_consumers_queue.insert(pending_consumer);
	
	BOOST_FOREACH(Resource* resource, updated_resources)
		BOOST_FOREACH( ResourceConsumer* contender, resource->consumers )
			AddPendingConsumer(contender, priority_upper_bound, updated_consumers, resources_owners_data);

	return pending_consumer;
}

template <class ResourceIDType, class ConsumerIDType>
void MeanKeeper<ResourceIDType, ConsumerIDType>::AddPendingConsumer(ResourceConsumer* consumer,
	const PriorityUpperBound& priority_upper_bound, std::unordered_set<PendingConsumer*>& updated_consumers, ResourcesOwnersData& resources_owners_data)
{
	if ( consumer->pending_consumer == 0 )
		RegularToPendingConsumer( consumer, priority_upper_bound, updated_consumers, resources_owners_data );
}

template <class ResourceIDType, class ConsumerIDType>
void MeanKeeper<ResourceIDType, ConsumerIDType>::ToRegularConsumer(PendingConsumer* consumer, 
	PriorityUpperBound& priority_upper_bound, my_linked_list_node<ResourceConsumer*>* insert_after, 
	ResourcesOwnersData& resources_owners_data, std::vector<ConsumerIDType>& update_changed_consumers)
{	
	bool is_approximate = consumer->is_approximate;
	if ( !is_approximate )
		resources_owners_data.RemoveConsumer(consumer);

	priority_pair before_update_priority = consumer->base_consumer->get_first_priority();
	ResourceConsumer* regular_consumer;
	std::vector<Resource*> lost_cells;
	std::vector<Resource*> acquired_cells;
	consumer->ToRegularConsumer(regular_consumer, priority_upper_bound.priority, lost_cells, acquired_cells);
	regular_consumer->pending_consumer = 0;
	if ( regular_consumer->get_first_priority() != before_update_priority)
		update_changed_consumers.push_back(consumers.backward_map[regular_consumer]);

	std::unordered_set<PendingConsumer*> updated_consumers;
	std::unordered_set<ResourceConsumer*> updated_owners;
	if (!is_approximate)
		BOOST_FOREACH(Resource* resource, regular_consumer->owned_resources)
		{
			if (resource->get_owner() != 0)
			{
				if (approximate_state)
					updated_owners.insert(resource->get_owner());
				resource->get_owner()->owned_resources.erase(resource);
			}
			SetResourceState(resource, regular_consumer, updated_consumers);
		}

	if (approximate_state)
		BOOST_FOREACH(ResourceConsumer* owner, updated_owners)
		{
			priority_pair current_owner_priority = owner->calculate_priority();
			if ( owner->get_first_priority() > current_owner_priority)
			{
				bool remove_from_map = owner->get_first_priority() == owner->get_second_priority();
				if (remove_from_map && owner->main_list_node->next_node != ordered_consumers_list.TAIL)
				{
					ResourceConsumer* next_consumer = owner->main_list_node->next_node->value;
					if ( next_consumer->get_first_priority() == owner->get_first_priority() )
					{
						next_consumer->priority.second_priority = owner->get_first_priority();
						ordered_consumers_map[owner->get_first_priority()] = next_consumer;
						remove_from_map = false;
					}
				}

				if (remove_from_map)
					ordered_consumers_map.erase(owner->get_first_priority());

				ordered_consumers_list.remove(owner->main_list_node);
				owner->main_list_node = 0;

				PendingConsumer* pending_consumer = new PendingConsumer(owner, priority_upper_bound.priority, 
					priority_upper_bound.higher_priority_consumers, true );
				owner->pending_consumer = pending_consumer;
				pending_consumers_queue.insert(pending_consumer);
			}
		}

	regular_consumer->main_list_node = ordered_consumers_list.insert_after(insert_after, regular_consumer);
	if (regular_consumer->get_first_priority().has_equal_priority(regular_consumer->get_second_priority()))
		ordered_consumers_map[regular_consumer->get_first_priority()] = regular_consumer;
	
	priority_upper_bound.update(regular_consumer);

	if (!approximate_state)
		BOOST_FOREACH( Resource* resource, regular_consumer->owned_resources)
			BOOST_FOREACH(ResourceConsumer* contender, resource->consumers)
				if ( contender != regular_consumer )
					AddPendingConsumer(contender, priority_upper_bound, updated_consumers, resources_owners_data);
	
	UpdateQueue(updated_consumers);
}

template <class ResourceIDType, class ConsumerIDType>
void MeanKeeper<ResourceIDType, ConsumerIDType>::RemoveConsumers(std::set<ConsumerIDType> removed_consumers_ids, ResourcesOwnersData& resources_owners_data)
{
	std::unordered_set<PendingConsumer*> updated_consumers;
	std::vector<ResourceConsumer*> removed_consumers;
	BOOST_FOREACH(ConsumerIDType removed_consumer_id, removed_consumers_ids)
	{
		ResourceConsumer* removed_consumer = consumers.forward_map[removed_consumer_id];
		removed_consumers.push_back(removed_consumer);
		BOOST_FOREACH(Resource* resource, removed_consumer->owned_resources)
			SetResourceState(resource, 0, updated_consumers);
		BOOST_FOREACH(Resource* resource, removed_consumer->asked_resources)
			resource->consumers.erase(removed_consumer);
	}
	
	PriorityUpperBound priority_upper_bound;
	BOOST_FOREACH(ResourceConsumer* removed_consumer, removed_consumers)
	{
		if (!approximate_state)
			BOOST_FOREACH(Resource* resource, removed_consumer->owned_resources)
				BOOST_FOREACH(ResourceConsumer* contender, resource->consumers)
					AddPendingConsumer(contender, priority_upper_bound, updated_consumers, resources_owners_data);

		ordered_consumers_list.remove(removed_consumer->main_list_node);
		if ( removed_consumer->get_first_priority() == removed_consumer->get_second_priority() )
			ordered_consumers_map.erase(removed_consumer->get_first_priority());
		consumers.erase_backward(removed_consumer);
		delete removed_consumer;
	}
	UpdateQueue(updated_consumers);
}

template <class ResourceIDType, class ConsumerIDType>
void MeanKeeper<ResourceIDType, ConsumerIDType>::RemoveResource(ResourceIDType resource_id)
{
	Resource* resource = resources.forward_map[resource_id];
	if (resource->consumers.empty())
	{
		delete resource;
		resources.erase_forward(resource_id);
	}
	else
		throw 1;
}

template <class ResourceIDType, class ConsumerIDType>
void MeanKeeper<ResourceIDType, ConsumerIDType>::AddConsumer(ConsumerIDType consumer_id, std::set<ResourceIDType>& asked_resources, 
	ResourcesOwnersData& resources_owners_data)
{
	total_num_consumers++;
	ResourceConsumer* consumer = new ResourceConsumer();
	consumer->priority.first_priority.id = -1;
	consumer->priority.first_priority.mean_value = -1;
	consumer->id = total_num_consumers;
	consumers.insert(consumer_id, consumer);
	for (std::set<ResourceIDType>::iterator asked_resources_iter = asked_resources.begin();
		asked_resources_iter != asked_resources.end();
		asked_resources_iter++)
	{
		ResourceIDType resource_id = *asked_resources_iter;
		Resource* resource = resources.forward_map[resource_id];
		consumer->asked_resources.insert(resource);
		resource->consumers.insert(consumer);
	}

	std::unordered_set<PendingConsumer*> updated_consumers;
	AddPendingConsumer(consumer, PriorityUpperBound(), updated_consumers, resources_owners_data);
	UpdateQueue(updated_consumers);
}

template <class ResourceIDType, class ConsumerIDType>
std::vector<ConsumerIDType> MeanKeeper<ResourceIDType, ConsumerIDType>::UpdateState(std::set<ConsumerIDType>& removed_consumers, 
	std::vector< std::pair<ConsumerIDType, std::set<ConsumerIDType> > >& new_consumers)
{
	std::vector<ConsumerIDType> update_changed_consumers;
	ResourcesOwnersData resources_owners_data;
	RemoveConsumers(removed_consumers, resources_owners_data);
	for( std::vector<std::pair<ConsumerIDType, std::set<ConsumerIDType> > >::iterator new_consumers_iter = new_consumers.begin();
						new_consumers_iter != new_consumers.end();
						new_consumers_iter++ )
	{
		std::pair<ConsumerIDType, std::set<ConsumerIDType> >& consumer_info = *new_consumers_iter;
		AddConsumer(consumer_info.first, consumer_info.second, resources_owners_data);
	}
	
	my_linked_list_node<ResourceConsumer*>* prev_node = ordered_consumers_list.HEAD;
	PriorityUpperBound priority_upper_bound;
	while (!pending_consumers_queue.empty())
	{
		PendingConsumer* first_priority_pending_consumer = *pending_consumers_queue.begin();
		ResourceConsumer* first_priority_owner = resources_owners_data.GetFirstPriorityOwner();
		priority_pair consumer_priority = first_priority_pending_consumer->get_priority();
		
		bool success = false;
		while (!success)
		{
			bool resource_owner_more_important = first_priority_owner!=0 && first_priority_owner->get_first_priority().has_higher_priority(consumer_priority);
			if ( resource_owner_more_important )
			{
				if ( first_priority_owner->get_first_priority() != first_priority_owner->get_second_priority() )
				{
					bool not_moving_from_start = prev_node==ordered_consumers_list.HEAD || 
						prev_node->value->get_first_priority() != first_priority_owner->get_first_priority();
					if ( not_moving_from_start )
					{
						first_priority_owner = ordered_consumers_map[first_priority_owner->get_first_priority()];
						prev_node = first_priority_owner->main_list_node->prev_node;
					}
					else if (prev_node->next_node->value != first_priority_owner)
						first_priority_owner = prev_node->next_node->value;
					else if ( !resources_owners_data.ContainsOwner(first_priority_owner) )
					{
						priority_upper_bound.update(first_priority_owner);
						prev_node = prev_node->next_node;
						first_priority_owner = prev_node->next_node->value;
					}
					else
						success = true;
				}
				else
					success = true;

				if (success)
				{
					if (resources_owners_data.ContainsOwner(first_priority_owner))
					{
						std::unordered_set<PendingConsumer*> owner_consumers = resources_owners_data.owner_consumers_map[first_priority_owner];
						BOOST_FOREACH(PendingConsumer* pending_consumer, owner_consumers)
							if (pending_consumer->has_owner(first_priority_owner))
							{
								pending_consumers_queue.erase(pending_consumer);
								pending_consumer->remove_consumer_resources(first_priority_owner);
								pending_consumers_queue.insert(owner_consumers.begin(), owner_consumers.end());
							}
						resources_owners_data.RemoveOwner(first_priority_owner);
					}
					priority_upper_bound.update(first_priority_owner);
					prev_node = first_priority_owner->main_list_node;
				}
			}
			else
			{
				if ( prev_node != ordered_consumers_list.HEAD && prev_node->value->get_first_priority().has_lower_priority(consumer_priority) )
				{
					if ( prev_node->next_node != ordered_consumers_list.end() )
					{
						first_priority_owner = prev_node->next_node->value;
						if ( first_priority_owner->get_second_priority().has_higher_priority(consumer_priority) )
						{
							if (resources_owners_data.ContainsOwner(first_priority_owner))
							{
								std::unordered_set<PendingConsumer*> owner_consumers = resources_owners_data.owner_consumers_map[first_priority_owner];
								BOOST_FOREACH(PendingConsumer* pending_consumer, owner_consumers)
									if (pending_consumer->has_owner(first_priority_owner))
									{
										pending_consumers_queue.erase(pending_consumer);
										pending_consumer->remove_consumer_resources(first_priority_owner);
										pending_consumers_queue.insert(owner_consumers.begin(), owner_consumers.end());
									}
								resources_owners_data.RemoveOwner(first_priority_owner);
							}
							priority_upper_bound.update(first_priority_owner);
							prev_node = first_priority_owner->main_list_node;
							success = true;
							continue;
						}
					}
				}
				else
				{
					if (!ordered_consumers_map.empty())
					{
						std::map<priority_pair, ResourceConsumer*, std::greater<priority_pair> >::iterator prev_node_iter = ordered_consumers_map.upper_bound(consumer_priority);
						if (prev_node_iter == ordered_consumers_map.end())
							prev_node = ordered_consumers_list.TAIL->prev_node;
						else
							prev_node = prev_node_iter->second->main_list_node->prev_node;
					}
					else
						prev_node = ordered_consumers_list.HEAD;
				}
				
				pending_consumers_queue.erase(pending_consumers_queue.begin());
				ResourceConsumer* consumer = first_priority_pending_consumer->base_consumer;
				ToRegularConsumer(first_priority_pending_consumer, priority_upper_bound, prev_node, resources_owners_data, update_changed_consumers);
				prev_node = consumer->main_list_node;
				success = true;
			}
		}
	}

	return update_changed_consumers;
}