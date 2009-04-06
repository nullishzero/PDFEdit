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
// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80

#ifndef _LISTITEM_H_
#define _LISTITEM_H_

#include <boost/smart_ptr.hpp>

//==========================================================
//namespace  {
//==========================================================

/**
 * Class representing an item in a list.
 */
template<typename Item>
class ListItem
{	
	// Typedefs
public:
	typedef boost::weak_ptr<Item> Type;
	
	// Variables
private:
	Type _p;	/**< Reference to previous item. */
	Type _n;	/**< Reference to next item. */

	//
	// Methods
	//
public:
	/**
	 * Set next item.
	 * @param nxt Next item in iterator list.
	 */
	void setNext (boost::shared_ptr<Item> nxt) 
		{ setNext (Type (nxt));	}
	/** \copydoc setNext */
	void setNext (Type nxt) 
		{ _n = nxt; }

	/**
	 * Set previous item.
	 * @param prv Previous item in iterator list.
	 */
	void setPrev (boost::shared_ptr<Item> prv) 
		{ setPrev (Type (prv)); }
	/** \copydoc setPrev */
	void setPrev (Type prv) 
		{ _p = prv; }

protected:
	/** Returns previous item in the iterator list that is implemented by PdfOperator. */
	Type _next () const 
		{ return _n; }

	/** Returns next item in the iterator list that is implemented by PdfOperator. */
	Type _prev () const 
		{ return _p; }

}; // class ListItem

//==========================================================
//} // namespace 
//==========================================================


#endif // _LISTITEM_H_

