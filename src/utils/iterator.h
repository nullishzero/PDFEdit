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
	LinkedListIterator (Item oper) : _cur (oper) {};

	/** Next. */
	LinkedListIterator<Item>& next ()
	{
		if (!_cur.expired())
		{
			assert (_cur.lock());
			_cur = _cur.lock()->_next ();
					
		}else
			throw IteratorInvalidObject ();

		return *this;
	};
	
	/** Previous. */
	LinkedListIterator<Item>& prev ()
	{
		if (!_cur.expired())
		{
			assert (_cur.lock());
			_cur = _cur.lock()->_prev ();
			
		}else
			throw IteratorInvalidObject ();
		
		return *this;
	};
	
	/** Get current. */
	Item getCurrent () const {return _cur;};

	/** Are we at the end. Doesn't matter whether in the front or at the back */
	bool isEnd () const { return (_cur.expired()) ? true : false; };

};

//==========================================================
}
//==========================================================

#endif // _ITERATOR_H_
