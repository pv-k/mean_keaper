#include "Resource.h"
#include "ResourceConsumer.h"
#include "PendingConsumer.h"

Resource::Resource(void)
{
	this->id = -1;
	this->resources_count = -1;
	this->resources_value = -1;
	this->set_owner(0);
}

Resource::~Resource(void)
{
}

void Resource::set_owner(ResourceConsumer* owner )
{
	this->owner = owner;
}

priority_pair Resource::get_first_owner_priority()
{
	if (owner==0)
		return priority_pair(MK_INFINITY, MK_INFINITY);
	else
		return owner->get_first_priority();
}

bool Resource_owner_is_less_important(Resource* left, Resource* right)
{
	return left->get_owner()->has_lower_priority(right->get_owner());
}

bool Resource_owner_is_more_important(Resource* left, Resource* right)
{
	return left->get_owner()->has_higher_priority(right->get_owner());
}