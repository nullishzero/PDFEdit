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
 * prev and nex it is stored in the Item itself, not in a special container.
 *
 * The item over which to iterate has to implement 2 methods:
 * <ul>
 * 	<li> _next () </li>
 * 	<li> _prev () </li>
 * </ul>
 * 
 * 
 * It could be confusing.
 */
template<typename Item>
class LinkedListIterator
{
private:
	Item _cur;
		
public:

	/** Constructors. */
	LinkedListIterator () {};
	LinkedListIterator (Item oper) : _cur (oper) {};

	/** Next. */
	LinkedListIterator<Item>& next ()
	{
		if (_cur)
		{
			_cur = _cur->_next ();
					
		}else
			throw IteratorInvalidObject ();

		return *this;
	};
	
	/** Previous. */
	LinkedListIterator<Item>& prev ()
	{
		if (_cur)
		{
			_cur = _cur->_prev ();
			
		}else
			throw IteratorInvalidObject ();
		
		return *this;
	};
	
	/** Operator =. */
	LinkedListIterator<Item>& operator= (LinkedListIterator<Item>& iter)
	{
		_cur = iter._cur;
		return *this;
	};
	
	/** Get current. */
	Item getCurrent () {return _cur;};
};

//==========================================================
}
//==========================================================

#endif // _ITERATOR_H_
