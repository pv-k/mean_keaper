#include <stdio.h>
#include <set>
#include <math.h>
#include "MeanKeeper.h"
#include <boost/foreach.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_real_distribution.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <iostream>
#include <ctime>

//#include <vld.h>

bool test_equal(MeanKeeper<int,int>* keeper1, MeanKeeper<int,int>* keeper2, std::set<int> resources_ids, std::set<int> consumers_ids)
{
	int cnt = 0;
	BOOST_FOREACH (int resource_id, resources_ids)
		BOOST_FOREACH (int consumer_id, consumers_ids)
		{
			cnt++;
			if (keeper1->OwnsResource(consumer_id, resource_id) != keeper2->OwnsResource(consumer_id, resource_id))
				return false;
		}

	BOOST_FOREACH (int consumer_id, consumers_ids)
		if (abs(keeper1->GetMeanValue(consumer_id) - keeper2->GetMeanValue(consumer_id))>1e-9)
			return false;

	return true;
}

void test_addition()
{
	for (int num_resources = 15; num_resources<100001; num_resources++)
	{
		for (int i=1; i<300000; i++)
		{
			boost::random::mt19937 rand_generator;
			rand_generator.seed(i);
			boost::random::uniform_real_distribution<> probability_distribution(0, 1);
			boost::random::uniform_int_distribution<> value_distribution(1, 100);
			
			int num_consumers = 15;
			double resources_fraction = 0.25;
			MeanKeeper<int,int>* keeper2 = new MeanKeeper<int,int>();
			MeanKeeper<int,int>* keeper1 = new MeanKeeper<int,int>();
			std::set<int> resources_inds;
			for (int resource_ind=0; resource_ind<num_resources; resource_ind++)
			{
				double count = value_distribution(rand_generator);
				double value = value_distribution(rand_generator);
				keeper1->AddResource(resource_ind, count, value);
				keeper2->AddResource(resource_ind, count, value);
				resources_inds.insert(resource_ind);
			}
	
			std::vector<std::pair<int, std::set<int> > > keeper1_new_consumers;
			std::vector<std::pair<int, std::set<int> > > keeper2_new_consumers;
			std::pair<int, std::set<int> > keeper2_new_consumer;
			std::set<int> removed_consumers;
			std::set<int> consumers_inds;
			for (int consumer_ind=0; consumer_ind<num_consumers; consumer_ind++)
			{
				consumers_inds.insert(consumer_ind);
				keeper2_new_consumers.clear();
				std::set<int> resources;
				for (int resource_ind=0; resource_ind<num_resources; resource_ind++)
				{
					if ( probability_distribution(rand_generator) < resources_fraction )
						resources.insert(resource_ind);
				}

				keeper1_new_consumers.push_back(make_pair(consumer_ind, resources));
				keeper2_new_consumers.push_back(make_pair(consumer_ind, resources));
				/*if (consumer_ind==10000)
				{
					clock_t begin = clock();
					keeper1->UpdateState(removed_consumers, keeper1_new_consumers);
					clock_t end = clock();
					double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
					std::cout<<elapsed_secs;
					std::cin>>consumer_ind;
				}*/
				keeper1->UpdateState(removed_consumers, keeper1_new_consumers);
				keeper2->UpdateState(removed_consumers, keeper2_new_consumers);

				if (!test_equal(keeper1, keeper2, resources_inds, consumers_inds))
					throw 1;
				
				keeper1->ClearConsumers();

				std::cout<<consumer_ind<<std::endl;
			}
			keeper2->ClearConsumers();
			delete keeper1;
			delete keeper2;
			std::cout<<i<<std::endl;
			//_CrtDumpMemoryLeaks();
		}
	}
}

void test_removal()
{
	for (int num_resources = 15; num_resources<100001; num_resources++)
	{
		for (int i=382; i<300000; i++)
		{
			boost::random::mt19937 rand_generator;
			rand_generator.seed(i);
			boost::random::uniform_real_distribution<> probability_distribution(0, 1);
			boost::random::uniform_int_distribution<> value_distribution(1, 100);
			
			int num_consumers = 25;
			double resources_fraction = 0.25;
			MeanKeeper<int,int>* keeper2 = new MeanKeeper<int,int>();
			MeanKeeper<int,int>* keeper1 = new MeanKeeper<int,int>();
			std::set<int> resources_inds;
			for (int resource_ind=0; resource_ind<num_resources; resource_ind++)
			{
				double count = value_distribution(rand_generator);
				double value = value_distribution(rand_generator);
				keeper1->AddResource(resource_ind, count, value);
				keeper2->AddResource(resource_ind, count, value);
				resources_inds.insert(resource_ind);
			}
	
			std::vector<std::pair<int, std::set<int> > > keeper1_new_consumers;
			std::vector<std::pair<int, std::set<int> > > keeper2_new_consumers;
			std::pair<int, std::set<int> > keeper2_new_consumer;
			std::set<int> removed_consumers;
			std::set<int> consumers_inds;
			for (int consumer_ind=0; consumer_ind<num_consumers; consumer_ind++)
			{
				consumers_inds.insert(consumer_ind);
				keeper2_new_consumers.clear();
				std::set<int> resources;
				for (int resource_ind=0; resource_ind<num_resources; resource_ind++)
				{
					if ( probability_distribution(rand_generator) < resources_fraction )
						resources.insert(resource_ind);
				}
				
				if (consumer_ind==15)
				{
					keeper1_new_consumers.erase(keeper1_new_consumers.begin()+8,keeper1_new_consumers.begin()+12);
					removed_consumers.insert(8);
					removed_consumers.insert(9);
					removed_consumers.insert(10);
					removed_consumers.insert(11);
					consumers_inds.erase(8);
					consumers_inds.erase(9);
					consumers_inds.erase(10);
					consumers_inds.erase(11);
					keeper2->UpdateState(removed_consumers, keeper2_new_consumers);
					removed_consumers.clear();
				}

				keeper1_new_consumers.push_back(make_pair(consumer_ind, resources));
				keeper2_new_consumers.push_back(make_pair(consumer_ind, resources));

				keeper1->UpdateState(removed_consumers, keeper1_new_consumers);
				keeper2->UpdateState(removed_consumers, keeper2_new_consumers);

				if (!test_equal(keeper1, keeper2, resources_inds, consumers_inds))
					throw 1;
				
				keeper1->ClearConsumers();

				std::cout<<consumer_ind<<std::endl;
			}
			keeper2->ClearConsumers();
			delete keeper1;
			delete keeper2;
			std::cout<<i<<std::endl;
			//_CrtDumpMemoryLeaks();
		}
	}
}

void test_approximate()
{
	for (int num_resources = 15; num_resources<100001; num_resources++)
	{
		for (int i=3; i<300000; i++)
		{
			boost::random::mt19937 rand_generator;
			rand_generator.seed(i);
			boost::random::uniform_real_distribution<> probability_distribution(0, 1);
			boost::random::uniform_int_distribution<> value_distribution(1, 100);
			
			int num_consumers = 250;
			double resources_fraction = 0.25;
			MeanKeeper<int,int>* keeper2 = new MeanKeeper<int,int>();
			MeanKeeper<int,int>* keeper1 = new MeanKeeper<int,int>();
			std::set<int> resources_inds;
			for (int resource_ind=0; resource_ind<num_resources; resource_ind++)
			{
				double count = value_distribution(rand_generator);
				double value = value_distribution(rand_generator);
				keeper2->AddResource(resource_ind, count, value);
				resources_inds.insert(resource_ind);
			}
	
			std::vector<std::pair<int, std::set<int> > > keeper2_new_consumers;
			std::pair<int, std::set<int> > keeper2_new_consumer;
			std::set<int> removed_consumers;
			std::set<int> consumers_inds;
			for (int consumer_ind=0; consumer_ind<num_consumers; consumer_ind++)
			{
				consumers_inds.insert(consumer_ind);
				keeper2_new_consumers.clear();
				std::set<int> resources;
				for (int resource_ind=0; resource_ind<num_resources; resource_ind++)
				{
					if ( probability_distribution(rand_generator) < resources_fraction )
						resources.insert(resource_ind);
				}

				keeper2_new_consumers.push_back(make_pair(consumer_ind, resources));
				if (consumer_ind==15)
					keeper2->MakeApproximate();
				keeper2->UpdateState(removed_consumers, keeper2_new_consumers);

				std::cout<<consumer_ind<<std::endl;
			}
			keeper2->ClearConsumers();
			delete keeper2;
			std::cout<<i<<std::endl;
		}
	}
}

int main( int argc, const char* argv[] )
{
	//test_addition();
	//test_removal();
	test_approximate();
}