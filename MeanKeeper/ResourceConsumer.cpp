#include "ResourceConsumer.h"
#include <boost\foreach.hpp>
#include "consts.h"

ResourceConsumer::ResourceConsumer(void)
{
	pending_consumer = 0;
	main_list_node = 0;
	id = -1;
}

priority_pair ResourceConsumer::calculate_priority()
{
	double sum = 0;
	double count = 0;
	BOOST_FOREACH(Resource* resource, owned_resources)
	{
		sum += resource->resources_value;
		count += resource->resources_count;
	}

	return priority_pair( count<MK_TOLERANCE ? MK_INFINITY : sum / count , id );
}

ResourceConsumer::~ResourceConsumer(void)
{
}