#pragma once
#ifndef RESOURCE_H
#define RESOURCE_H

#include <unordered_set>
#include <forward_list>
#include <set>
#include "priority_pair.h"
#include "consts.h"

class ResourceConsumer;
class PendingConsumer;

class Resource
{
public:
	Resource(void);
	~Resource(void);

	bool can_be_taken(double bounded_mean_value, double bounded_id, priority_pair& priority);
	bool can_be_taken(const priority_pair& bounded_priority, priority_pair& priority);
	
	priority_pair get_first_owner_priority();

	void set_owner(ResourceConsumer* owner );

	ResourceConsumer* get_owner()
	{
		return owner;
	}

	bool is_free()
	{
		return owner==0;
	}
	
	double id;
	double resources_count;
	double resources_value;
	std::unordered_set<ResourceConsumer*> consumers;
private:
	ResourceConsumer* owner;
};

bool Resource_owner_is_less_important(Resource* left, Resource* right);
bool Resource_owner_is_more_important(Resource* left, Resource* right);

struct Resource_owner_less_important {
    bool operator()(Resource* left, Resource* right)
	{
		return Resource_owner_is_less_important(left, right);
    }
};

struct Resource_owner_more_important {
    bool operator()(Resource* left, Resource* right)
	{
		return Resource_owner_is_more_important(left, right);
    }
};

#endif