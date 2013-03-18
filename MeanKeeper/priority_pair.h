#pragma once
#ifndef priority_pair_h
#define priority_pair_h

class priority_pair
{
public:
	double mean_value;
	double id;

	priority_pair()
	{
		this->mean_value = -1;
		this->id = -1;
	}

	priority_pair(double mean_value, double id)
	{
		this->mean_value = mean_value;
		this->id = id;
	}

	bool has_higher_priority(priority_pair* rival) const
	{
		return rival->mean_value>mean_value || rival->mean_value==mean_value && 
			rival->id>id;
	}

	bool has_higher_priority(const priority_pair& rival) const
	{
		return rival.mean_value>mean_value || rival.mean_value==mean_value && 
			rival.id>id;
	}

	bool has_equal_priority(const priority_pair& rival) const
	{
		return rival.mean_value==mean_value && rival.id==id;
	}

	bool has_equal_priority(priority_pair* rival) const
	{
		return rival->mean_value==mean_value && rival->id==id;
	}

	bool has_lower_priority(priority_pair* rival) const
	{
		return rival->mean_value<mean_value || rival->mean_value==mean_value && 
			rival->id<id;
	}
	
	bool has_lower_priority(const priority_pair& rival) const
	{
		return rival.mean_value<mean_value || rival.mean_value==mean_value && 
			rival.id<id;
	}

	bool has_higher_priority(double rival_mean_value, double rival_id) const
	{
		return rival_mean_value>mean_value || rival_mean_value==mean_value && 
			rival_id>id;
	}

	bool has_equal_priority(double rival_mean_value, double rival_id) const
	{
		return rival_mean_value==mean_value && rival_id==id;
	}

	bool has_lower_priority(double rival_mean_value, double rival_id) const
	{
		return rival_mean_value<mean_value || rival_mean_value==mean_value && 
			rival_id<id;
	}

	void update(double new_mean_value, double new_id)
	{
		if ( has_higher_priority(new_mean_value, new_id) )
		{
			mean_value = new_mean_value;
			id = new_id;
		}
	}

	void update(const priority_pair& new_priority)
	{
		if ( has_higher_priority(new_priority) )
		{
			mean_value = new_priority.mean_value;
			id = new_priority.id;
		}
	}

	void update(priority_pair* new_priority)
	{
		if ( has_higher_priority(new_priority) )
		{
			mean_value = new_priority->mean_value;
			id = new_priority->id;
		}
	}
	
	void set_value(double new_mean_value, double new_id)
	{
		mean_value = new_mean_value;
		id = new_id;
	}

	void set_value(const priority_pair& new_priority)
	{
		mean_value = new_priority.mean_value;
		id = new_priority.id;
	}

	void set_value(priority_pair* new_priority)
	{
		mean_value = new_priority->mean_value;
		id = new_priority->id;
	}
	
	friend bool operator== (const priority_pair& p1, const priority_pair& p2)
	{
		return p1.has_equal_priority(p2);
	}

    friend bool operator!= (const priority_pair& p1, const priority_pair& p2)
	{
		return !p1.has_equal_priority(p2);
	}

	friend bool operator> (const priority_pair& p1, const priority_pair& p2)
	{
		return p1.has_higher_priority(p2);
	}

    friend bool operator< (const priority_pair& p1, const priority_pair& p2)
	{
		return p1.has_lower_priority(p2);
	}

};

#endif