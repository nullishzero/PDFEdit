/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006-2009  PDFedit team: Michal Hocko,
 *                                        Jozef Misutka,
 *                                        Martin Petricek
 *                   Former team members: Miroslav Jahoda
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (in doc/LICENSE.GPL); if not, write to the 
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 * MA  02111-1307  USA
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit
 */
#ifndef _ITERATOR_H_
#define _ITERATOR_H_

#include <exception>
#include <boost/smart_ptr.hpp>

//==========================================================
namespace iterator {
//==========================================================

/** Iterator exception. */
struct IteratorInvalidObjectException : public std::exception
{
	char const* what() const throw() 
		{return "Iterating over invalid object.";}
};

/**
 * Iterator implementation. 
 * Interface is similar to the Iterator design pattern (Gof book). It is not supposed to 
 * be like stl iterators, because we do NOT need all the functionality stl provides.
 *
 * It is a generic iterator over items that form a doubly linked list.
 * The item type over which to iterate has to implement 2 methods:
 * <ul>
 * 	<li> _next () </li>
 * 	<li> _prev () </li>
 * </ul>
 * 
 * The main difference between stl iterators and this iterator is where information about previous 
 * and next items is stored. In this iterator it is in the item itself while in stl it is in a 
 * special container.
 * 
 * This list is intended to be used with smart pointers. Which means we can not build the linked list
 * using smart pointers because we would get a dependency cycle and the objects would be never freed.
 * So instead smart pointers we use a weak pointers from which you can 
 * can get the smart pointer. 
 * We can not simply use raw pointers because we are using smart pointers all over the code so we have 
 * to be able to get smart pointer from an iterator.
 *
 * This class has two design pattern template methods. It can be easily extended to specific
 * iterators overloading isValidItem() method.
 * This is a very powerful feature which gives us enormous flexibilty over a list queue. We can easily iterate 
 * only over specific items.
 */
template<typename Item>
class DoubleListIterator
{

	// Typedefs
public:
	/** ListItem type. */
	typedef IteratorInvalidObjectException Exception;
	typedef boost::weak_ptr<Item> ListItem;


	// Position
private:
	enum Position { _end, _begin, _valid, _invalid };	/**< Actual position state. */
	static const Position pend 		= _end; 	/**< The end. Not a valid item. */
	static const Position pbegin	= _begin;	/**< The beggining. Not a valid item. */
	static const Position pvalid	= _valid;	/**< Valid position in the list. */
	static const Position pinvalid 	= _invalid;	/**< Invalid position. E.g. no item specified. */
		

	// Variables
protected:
	ListItem _cur;		/**< Current item. */
	Position _curpos;	/**< Current position state. */


	//
	// Ctor & Dtor
	// 
public:

	/** Constructor. */
	DoubleListIterator (ListItem oper) : _cur (oper), _curpos (pvalid)
		{ if (_invalidItem (oper)) _curpos = pinvalid; }
	/**  Created iterator is not in valid state and can not be used to iterate. */
	DoubleListIterator () : _cur (ListItem()), _curpos (pinvalid) {}

	/** Dtor. */
	virtual ~DoubleListIterator () {}
	
	//
	// Iterator interface
	//
public:
	/** 
	 * Go to the next item. The same as <code>++iterator;</code> in the stl.
	 * We can iterate only over specific items that are selected by the validItem() function
	 *
	 * \see validItem
	 *
	 * @return Iterator pointing at the next item.
	 */
	DoubleListIterator<Item>& next ()
	{
			if (pinvalid == _curpos || pend == _curpos)
				throw Exception ();
		
		// Are we at the beginning
		if (pbegin == _curpos)
		{
			_curpos = pvalid;
			return *this;
		}
		
		// Loop until an item is valid or end is reached
		ListItem tmp;
		do {
				
			tmp = _cur.lock()->_next ();
			// Are we at the end
			if (_invalidItem(tmp))
			{
				_curpos = pend;
				return *this;
			}
			_cur = tmp;
			
		} while (!validItem());
		
		return *this;
	}
	
	/** 
	 * Go to the previous item. The same as <code>++reverse_iterator</code> in the stl.
	 * We can iterate only over specific items that are selected by the validItem() function.
	 *
	 * \see validItem

	 * @return Iterator pointing at the previous item.
	 */
	DoubleListIterator<Item>& prev ()
	{
			if (pinvalid == _curpos || pbegin == _curpos)
				throw Exception ();
		
		// Are we at the end
		if (pend == _curpos)
		{
			_curpos = pvalid;
			return *this;
		}
		
		// Loop until an item is valid or beginning is reached
		ListItem tmp;
		do {
				
			tmp = _cur.lock()->_prev ();
			// Are we at the end
			if (_invalidItem(tmp))
			{
				_curpos = pbegin;
				return *this;
			}
			_cur = tmp;
			
		} while (!validItem());
		
		return *this;
	}
	
	
	//
	// Comparable interface
	//
public:	
	/** Equality operator. */
	bool operator== (const DoubleListIterator<Item>& it) const
		{ return (it.getCurrent() == getCurrent());	}
	
	
	//
	// Helper methods
	//
public:
	/** 
	 * Get item pointed at. 
	 * @return Smart pointer which points to the actual item.
	 */
	boost::shared_ptr<Item> getCurrent () const 
		{ assert (pvalid == _curpos); return _cur.lock(); }

	/** 
	 * Are we at a valid item. 
	 *
	 * @return True if the position is valid, false otherwise.
	 */
	bool valid () const 
		{ return (pvalid == _curpos); }

	/**
	 * Are we before the first valid item.
	 *
	 * @return True if we are at the beginning. (We are at an invalid item) 
	 */
	bool isBegin () const 
		{ return (pbegin == _curpos); }
	
	/**
	 * Are we after the last valid item.
	 * 
	 * @return True if we are at the end. (We are at an invalid item)
	 */
	bool isEnd () const 
		{ return (pend == _curpos); }

	//
	// Helper
	//
protected:
	/** Is this item ivalid. */
	bool _invalidItem (ListItem it) 
		{ return it.expired() ? true: false; }
	
	//
	// Template method interface
	//
protected:
	/**
	 * In the base iterator, loop through all objects.
	 * We can overload this function to be able to iterate only over specific items.
	 *
	 * @return True if the item is supported by this type of iterator, false otherwise.
	 */
	virtual bool validItem () const 
		{ return true; }

}; // class DoubleListIterator


//==========================================================
} // namespace iterator
//==========================================================

#endif // _ITERATOR_H_
