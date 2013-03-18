#pragma once
#ifndef COMPEX_PRIORITY_H
#define COMPEX_PRIORITY_H

#include "priority_pair.h"

class complex_priority
{
public:
	priority_pair first_priority;
	priority_pair second_priority;

	complex_priority()
	{
		first_priority.mean_value = -1;
		first_priority.id = -1;
		second_priority.mean_value = -1;
		second_priority.id = -1;
	}

	complex_priority(const priority_pair& first_priority, const priority_pair& second_priority)
	{
		this->first_priority = first_priority;
		this->second_priority = second_priority;
	}

	void update(const priority_pair& new_first_priority, const priority_pair& new_second_priority)
	{
		first_priority = new_first_priority;
		second_priority = new_second_priority;
	}

	void bounded_update(const priority_pair& new_first_priority, const priority_pair& new_second_priority)
	{
		if ( new_first_priority < first_priority )
		{
			first_priority = new_first_priority;
			second_priority = new_second_priority;
		}
		else if ( new_first_priority == first_priority && new_second_priority < second_priority)
			second_priority = new_second_priority;
	}

	void bounded_update(const complex_priority& new_priority)
	{
		if ( new_priority.has_lower_priority( *this ) )
		{
			this->first_priority = new_priority.first_priority;
			this->second_priority = new_priority.second_priority;
		}
	}

	bool has_higher_priority(const complex_priority& priority) const
	{
		if ( first_priority == priority.first_priority )
			return second_priority > priority.second_priority;
		else
			return first_priority > priority.first_priority;
	}

	bool has_higher_priority( const priority_pair& first_priority, const priority_pair& second_priority ) const
	{
		if ( this->first_priority == first_priority )
			return this->second_priority > second_priority;
		else
			return this->first_priority > first_priority;
	}

	bool has_equal_priority(const complex_priority& priority) const
	{
		return first_priority == priority.first_priority && second_priority == priority.second_priority;
	}

	bool has_equal_priority( const priority_pair& first_priority, const priority_pair& second_priority ) const
	{
		return this->first_priority == first_priority && this->second_priority == second_priority;
	}

	bool has_lower_priority(const complex_priority& priority) const
	{
		if ( first_priority == priority.first_priority )
			return second_priority < priority.second_priority;
		else
			return first_priority < priority.first_priority;
	}

	bool has_lower_priority( const priority_pair& first_priority, const priority_pair& second_priority ) const
	{
		if ( this->first_priority == first_priority )
			return this->second_priority < second_priority;
		else
			return this->first_priority < first_priority;
	}

	friend bool operator== (const complex_priority& cP1, const complex_priority& cP2)
	{
		return cP1.has_equal_priority(cP2);
	}

    friend bool operator!= (const complex_priority& cP1, const complex_priority& cP2)
	{
		return !cP1.has_equal_priority(cP2);
	}

	friend bool operator> (const complex_priority& cP1, const complex_priority& cP2)
	{
		return cP1.has_higher_priority(cP2);
	}

    friend bool operator< (const complex_priority& cP1, const complex_priority& cP2)
	{
		return cP1.has_lower_priority(cP2);
	}

};

#endif