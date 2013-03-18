#pragma once
#ifndef RESOURCE_CONSUMER_H
#define RESOURCE_CONSUMER_H

#include "Resource.h"
#include <set>
#include "priority_pair.h"
#include "complex_priority.h"
#include "my_linked_list.h"
#include "consts.h"

class PendingConsumer;

class ResourceConsumer
{
public:
	ResourceConsumer(void);
	~ResourceConsumer(void);

	std::set<Resource*> asked_resources;
	std::set<Resource*> owned_resources;
	PendingConsumer* pending_consumer;
	complex_priority priority;
	my_linked_list_node<ResourceConsumer*>* main_list_node;
	double id;

	priority_pair calculate_priority();

	bool has_higher_priority(ResourceConsumer* rival)
	{
		return priority.has_higher_priority(rival->priority);
	}

	bool has_lower_priority(ResourceConsumer* rival)
	{
		return priority.has_lower_priority(rival->priority);
	}

	bool has_equal_priority(ResourceConsumer* rival)
	{
		return priority.has_equal_priority(rival->priority);
	}

	priority_pair get_first_priority()
	{
		return priority.first_priority;
	}
	
	priority_pair get_second_priority()
	{
		return priority.second_priority;
	}
};

#endif