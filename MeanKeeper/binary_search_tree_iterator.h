/************************************************************************/
/*	binary_search_tree_iterator.h										*/
/*	-----------------------------										*/
/*	an implementation of an STL iterator for binary search trees.		*/
/*											author: Nir Idisis			*/
/************************************************************************/

#pragma once
#ifndef __BINARY_SEARCH_TREE_ITERATOR
#define __BINARY_SEARCH_TREE_ITERATOR

#include <iterator>
using std::iterator ;
using std::bidirectional_iterator_tag ;

template< typename T >
class binary_search_tree_iterator:
	public std::iterator<	bidirectional_iterator_tag,
							typename T::value_type,
							typename T::size_type	>
{
protected:
		T * Iterator ;	// a pointer to a binary_node.
public:
	typedef typename T::value_type *			pointer ;			/* pointer typedef (usually T*). */  
	typedef typename T::value_type const *		const_pointer ;		/* pointer typedef (usually const T*). */  
	typedef typename T::value_type &			reference ;			/* reference typedef (usually T&). */  
	typedef typename T::value_type const &		const_reference ;	/* reference typedef (usually const T&). */

// Constructors:
// -------------
	/* create a new empty instance of binary_search_tree_iterator. */
	explicit binary_search_tree_iterator( void ): iterator(), Iterator( T::NIL ) { } ;
//	/* create a new instance of binary_search_tree_iterator. */
//	binary_search_tree_iterator( const_reference that ): Iterator( that ) { }
	/* create a new instance of binary_search_tree_iterator. */
	binary_search_tree_iterator( T * ptr ): Iterator( ptr ) { }
	/* copy constructor. */
	binary_search_tree_iterator( const binary_search_tree_iterator & that ): Iterator( that.Iterator ) { }

// Increment / Decrement Operators:
// --------------------------------
	/* go to the next node in the tree. */
	binary_search_tree_iterator operator++( void ) {
		Iterator = Iterator->successor() ;
		return *this ;
	}
	/* go to the next node in the tree. */
	binary_search_tree_iterator operator++( int ) {
		T * temp = Iterator ;
		operator++() ;
		return binary_search_tree_iterator( temp ) ;
	}
	/* go to the previous node in the tree. */
	binary_search_tree_iterator operator--( void ) {
		Iterator = Iterator->predecessor() ;
		return ( *this ) ;
	}
	/* go to the previous node in the tree. */
	binary_search_tree_iterator operator--( int ) {
		T * temp = Iterator ;
		operator--() ;
		return binary_search_tree_iterator( temp ) ;
	}
	/* assignment operator.	*/
	binary_search_tree_iterator & operator=( const_reference that ) {
		Iterator->value = that ;
		return ( *this );
	}
	/* compare two general_iterators. */
	bool operator==( const binary_search_tree_iterator & that ) const	{	return ( Iterator == that.Iterator ) ;	}
	/* compare two general_iterators (not equal). */
	bool operator!=( const binary_search_tree_iterator & that ) const	{	return ( Iterator != that.Iterator ) ;	}
	/* iterator operator. */
	operator T&()				{	return *Iterator ;	}
	/* const iterator operator. */
	operator const T&() const	{	return *Iterator ;	}
	/* indirect access. */
	reference operator* ( void )	{	return Iterator->value ;		}
	/* return a pointer to the object. */
	T * operator->( void )		{	return Iterator ;	}
	/* convertion to void *. */
	operator void *( void ) {
		return static_cast<void *>( Iterator ) ;
	}
	/* delete this node and free-up memory. */
	static void operator delete( void * ptr ) {
		delete *ptr ;
	}
} ;

#endif