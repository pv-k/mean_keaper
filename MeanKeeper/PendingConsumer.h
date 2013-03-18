#pragma once
#ifndef PENDING_CONSUMER_H
#define PENDING_CONSUMER_H

#include "Resource.h"
#include <unordered_set>
#include <vector>
#include <hash_map>
#include "consts.h"
#include "ResourceConsumer.h"
#include "priority_pair.h"

class PendingConsumer
{
private:
	//priority_pair get_candidate_priority();
	void update_priority();
	priority_pair priority;
	double resources_count;
	double resources_sum;
	std::hash_map<ResourceConsumer*,std::unordered_set<Resource*> > available_resources_owners;
public:
	bool is_approximate;
	ResourceConsumer* base_consumer;
	PendingConsumer(ResourceConsumer* base_consumer, const priority_pair& priority_lower_bound, 
		std::unordered_set<ResourceConsumer*> known_higher_priority_consumers, bool is_approximate);
	priority_pair get_priority();
	std::unordered_set<ResourceConsumer*> get_owners();
	void ToRegularConsumer(ResourceConsumer*& out_consumer, const priority_pair& priority_bound,
		std::vector<Resource*>& lost_cells, std::vector<Resource*>& acquired_cells);
	double get_base_id();
	bool remove_resource( Resource* resource );
	bool has_owner( ResourceConsumer* consumer );
	bool free_owner_resources( ResourceConsumer* consumer );
	bool remove_consumer_resources( ResourceConsumer* resource );
	bool add_resource(Resource* resource);
	bool empty();
};

#endif