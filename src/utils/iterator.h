/*
 * =====================================================================================
 *        Filename:  iterator.h
 *         Created:  03/25/2006 02:43:42 AM CET
 *          Author:  jmisutka ()
 * =====================================================================================
 */

#ifndef _ITERATOR_H_
#define _ITERATOR_H_

#include <exception>
#include <boost/smart_ptr.hpp>

//==========================================================
namespace iterator {
//==========================================================


struct IteratorInvalidObject : public std::exception
{
	char const* what() const throw() {return "Iterator function over invalid object.";}
};


/**
 * Iterator interface from Gof book. We do NOT want it to 
 * be like stl iterators, because we do NOT need all the functionality and also
 * prev and next are stored in the Item itself, not in a special container.
 *
 * The item type over which to iterate has to implement 2 methods:
 * <ul>
 * 	<li> _next () </li>
 * 	<li> _prev () </li>
 * </ul>
 * 
 * Also an assumption is made, that we use something like weak smart pointers so we 
 * must use lock method.
 *
 * This class has two template methods (Design pattern). It can be easily extended to specific
 * iterators overloading isValidItem method.
 */
template<typename ITEM>
class SharedDoubleLinkedListIterator
{
public:
	typedef boost::weak_ptr<ITEM> ListItem;

protected:
	ListItem _cur;

	//
	// Constructor
	// 
public:

	/** Constructors. */
	SharedDoubleLinkedListIterator (ListItem oper) : _cur (oper) {};
	
	/** 
	 * Constructors. 
	 * Be carefull when using this one. It does not point anywhere.
	 */
	SharedDoubleLinkedListIterator () : _cur (ListItem()) {};

	//
	// Destructor
	//
public:
	virtual ~SharedDoubleLinkedListIterator () {};
	
	//
	// Iterator interface
	//
public:
	/** 
	 * Next. 
	 */
	SharedDoubleLinkedListIterator<ITEM>& next ()
	{
		// Loop until an item is valid or end is reached
		do {
		
			assert (_cur.lock());
			_cur = _cur.lock()->_next ();
			
		}while (!_cur.expired() && !validItem());

		return *this;
	}
	
	/** 
	 * Previous. 
	 */
	SharedDoubleLinkedListIterator<ITEM>& prev ()
	{
		// Loop until an item is valid or end is reached
		do {

			assert (_cur.lock());
			_cur = _cur.lock()->_prev ();

		}while (!_cur.expired() && !validItem());

		return *this;
	}
	
	
	//
	// Comparable interface
	//
public:	
	bool operator== (const SharedDoubleLinkedListIterator<ITEM>& it) const
		{ return (it.getCurrent() == getCurrent());	}
	
	
	//
	// Helper methods
	//
public:
	/** 
	 * Get current. It returns locked current pointer. 
	 */
	boost::shared_ptr<ITEM> getCurrent () const {return _cur.lock();}

	/** 
	 * Are we at the end. Doesn't matter whether in the front or at the back 
	 */
	bool isEnd () const { return (_cur.expired()) ? true : false; }

	
	//
	// Template method interface
	//
public:
	/**
	 * In this base iterator, loop through all objects.
	 *
	 * @return True if the item is supported by this type of iterator, false otherwise.
	 */
	virtual bool validItem () const {return true;}
};

//==========================================================
} // namespace iterator
//==========================================================

#endif // _ITERATOR_H_
