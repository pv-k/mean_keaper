#pragma once
#ifndef MY_LINKED_LIST_H
#define MY_LINKED_LIST_H

template<class TValue>
class my_linked_list_node
{
public:
	my_linked_list_node()
	{
		next_node = 0;
		prev_node = 0;
	}

	TValue value;
	my_linked_list_node* next_node;
	my_linked_list_node* prev_node;
};

template<class TValue>
class my_linked_list
{
public:
	my_linked_list_node<TValue>* HEAD;
	my_linked_list_node<TValue>* TAIL;

	my_linked_list_node<TValue>* insert_after(my_linked_list_node<TValue>* node, TValue inserted_value)
	{
		my_linked_list_node<TValue>* new_node = new my_linked_list_node<TValue>();
		new_node->value = inserted_value;
		new_node->prev_node = node;
		new_node->next_node = node->next_node;
		node->next_node->prev_node = new_node;
		node->next_node = new_node;
		return new_node;
	}

	my_linked_list_node<TValue>* insert_before(my_linked_list_node<TValue>* node, TValue inserted_value)
	{
		my_linked_list_node<TValue>* new_node = new my_linked_list_node<TValue>();
		new_node->value = inserted_value;
		new_node->prev_node = node->prev_node;
		node->prev_node->next_node = new_node;
		new_node->next_node = node;
		node->prev_node = new_node;
		return new_node;
	}

	void remove(my_linked_list_node<TValue>* removed_node)
	{
		removed_node->prev_node->next_node = removed_node->next_node;
		removed_node->next_node->prev_node = removed_node->prev_node;
		delete removed_node;
	}

	void clear()
	{
		my_linked_list_node<TValue>* removed_node = HEAD->next_node;
		while ( removed_node != TAIL )
		{
			my_linked_list_node<TValue>* next_node = removed_node->next_node;
			delete removed_node;
			removed_node = next_node;
		}
		HEAD->next_node = TAIL;
		TAIL->prev_node = HEAD;
	}

	my_linked_list()
	{
		HEAD = new my_linked_list_node<TValue>();
		TAIL = new my_linked_list_node<TValue>();
		HEAD->next_node = TAIL;
		TAIL->prev_node = HEAD;
	}

	my_linked_list_node<TValue>* end()
	{
		return TAIL;
	}

	my_linked_list_node<TValue>* begin()
	{
		return HEAD->next_node();
	}
};

#endif