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

/** Iterator is in invalid state. */
struct IteratorInvalidObjectException : public std::exception
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

		//
		// Position
		//
private:
		enum _Position { _end, _begin, _valid, _invalid };
		static const _Position pend 		= _end;
		static const _Position pbegin		= _begin;
		static const _Position pvalid		= _valid;
		static const _Position pinvalid 	= _invalid;
		
		
public:
	typedef boost::weak_ptr<ITEM> ListItem;

protected:
	ListItem _cur;
	_Position _curpos;

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
	 * Next. 
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
	 * Previous. 
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
	bool operator== (const SharedDoubleLinkedListIterator<ITEM>& it) const
		{ return (it.getCurrent() == getCurrent());	}
	
	
	//
	// Helper methods
	//
public:
	/** 
	 * Get current. It returns locked current pointer. 
	 */
	boost::shared_ptr<ITEM> getCurrent () const 
		{ assert (pvalid == _curpos); return _cur.lock(); }

	/** 
	 * Are we at the end. Doesn't matter whether in the front or at the back 
	 */
	bool valid () const { return (pvalid == _curpos); }

	/**
	 * Are we before the first valid item.
	 */
	bool isBegin () const { return (pbegin == _curpos); }
	
	/**
	 * Are we after the last valid item.
	 */
	bool isEnd () const { return (pend == _curpos); }

	//
	// Helper
	//
protected:
	inline bool _invalidItem (ListItem it) { return it.expired() ? true: false; }
	
	//
	// Template method interface
	//
protected:
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
