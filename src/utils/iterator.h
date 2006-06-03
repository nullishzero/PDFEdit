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

/** Invalid iterator exception. */
struct IteratorInvalidObjectException : public std::exception
{
	char const* what() const throw() {return "Iterator function over invalid object.";}
};


/**
 * Iterator. The interface is similar to the Iterator designe pattern from Gof book. It is not supposed to 
 * be like stl iterators, because we do NOT need all the functionality stl provides.
 *
 * It is a general iterator over items that form a double linked list.
 * The item type over which to iterate has to implement 2 methods:
 * <ul>
 * 	<li> _next () </li>
 * 	<li> _prev () </li>
 * </ul>
 * 
 * The main diffrence between stl iterators and this iterator is that the information about previous 
 * and next items is stored in the item itself, not in a special container.
 * 
 * We use something like weak smart pointers so we instead of references. So we have to use
 * lock method to be able to access an item. We can not simply use pointers because we are relying on the 
 * fact that we can produce smart pointer from an iterator.
 *
 * This class has two template methods. It can be easily extended to specific
 * iterators overloading isValidItem method. *
 * This is very powerful feature that gives us enormous flexibilty over a list queue. We easily iterate 
 * only over specific items.
 */
template<typename ITEM>
class SharedDoubleLinkedListIterator
{

		//
		// Position
		//
private:
		/** Position state of the iteratorm. */
		enum _Position { _end, _begin, _valid, _invalid };
		static const _Position pend 		= _end; 	/**< The end. Not a valid item. */
		static const _Position pbegin		= _begin;	/**< The beggining. Not a valid item. */
		static const _Position pvalid		= _valid;	/**< Valid position in the list. */
		static const _Position pinvalid 	= _invalid;	/**< Invalid position. E.g. no item specified. */
		
		
public:
	typedef boost::weak_ptr<ITEM> ListItem;

protected:
	ListItem _cur;		/**< Current item. */
	_Position _curpos;	/**< Current position state. */

	//
	// Constructor
	// 
public:

	/** Constructors. */
	SharedDoubleLinkedListIterator (ListItem oper) : _cur (oper), _curpos (pvalid)
		{ if (_invalidItem (oper)) _curpos = pinvalid; };
	
	/** 
	 * Constructors. 
	 * Be carefull when using this one. It does point nowhere.
	 */
	SharedDoubleLinkedListIterator () : _cur (ListItem()), _curpos (pinvalid) {};

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
	 * Go to the next item. The same as 
	 * \code
	 * ++iterator;
	 * \endcode
	 * in the stl.
	 * We can iterate only over specific items that are selected by the validItem function
	 *
	 * \see validItem
	 *
	 * @return Iterator pointing at the next item.
	 */
	SharedDoubleLinkedListIterator<ITEM>& next ()
	{
		assert (pinvalid != _curpos);
		assert (pend != _curpos);

		if (pinvalid == _curpos || pend == _curpos)
				throw IteratorInvalidObjectException ();
		
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
	 * Go to the previous item. The same as 
	 * \code
	 * ++reverse_iterator;
	 * \endcode
	 * in the stl.
	 * We can iterate only over specific items that are selected by the validItem function
	 *
	 * \see validItem

	 * @return Iterator pointing at the previous item.
	 */
	SharedDoubleLinkedListIterator<ITEM>& prev ()
	{
		assert (pinvalid != _curpos);
		assert (pbegin != _curpos);

		if (pinvalid == _curpos || pbegin == _curpos)
				throw IteratorInvalidObjectException ();
		
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
	bool operator== (const SharedDoubleLinkedListIterator<ITEM>& it) const
		{ return (it.getCurrent() == getCurrent());	}
	
	
	//
	// Helper methods
	//
public:
	/** 
	 * Get item pointed at. It returns locked current pointer (e.g. boost::shared_ptr) 
	 */
	boost::shared_ptr<ITEM> getCurrent () const 
		{ assert (pvalid == _curpos); return _cur.lock(); }

	/** 
	 * Are we at a valid item. 
	 *
	 * @return True if the position is valid, false otherwise.
	 */
	bool valid () const { return (pvalid == _curpos); }

	/**
	 * Are we before the first valid item.
	 *
	 * @return True if we are at the beginning. (We are at an invalid position) 
	 */
	bool isBegin () const { return (pbegin == _curpos); }
	
	/**
	 * Are we after the last valid.
	 * 
	 * @return True if we are at the end. (We are at an invalid position) 
	 */
	bool isEnd () const { return (pend == _curpos); }

	//
	// Helper
	//
protected:
	/** Is this item ivalid. */
	bool _invalidItem (ListItem it) { return it.expired() ? true: false; }
	
	//
	// Template method interface
	//
protected:
	/**
	 * In this base iterator, loop through all objects.
	 *
	 * This is an implementation of a method which is used by the template method next and prev
	 * \see next, prev
	 * We can overload this function to be able to iterate only over specific items.
	 *
	 * @return True if the item is supported by this type of iterator, false otherwise.
	 */
	virtual bool validItem () const {return true;}
};

//==========================================================
} // namespace iterator
//==========================================================

#endif // _ITERATOR_H_
