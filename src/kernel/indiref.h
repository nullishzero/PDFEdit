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
#ifndef _INDIREF_H_
#define _INDIREF_H_

//=====================================================================================
namespace pdfobjects {
//=====================================================================================


/** 
 * Two numbers specifying indirect reference of a pdf object. 
 */
typedef 
struct IndiRef
{
	/** Object identification number. */
	typedef unsigned int ObjNum;
	/** Object generation number. */
	typedef unsigned int GenNum;

	ObjNum	num; /**< Objects pdf identification number. */
	GenNum	gen; /**< Objects pdf generation number. */

	static const ObjNum invalidnum = 0;	/**< Invalid object identification number. */
	static const GenNum invalidgen = 0; /**< Invalid object generation number. */

	/** 
	 * Empty constructor.
	 * Initialize identification and generation number to invalid reference.
	 */
	IndiRef() : num(invalidnum), gen(invalidgen) {}
	
	/** 
	 * Constructor.
	 * Stores supplied identification and generation numbers.
	 * 
	 * @param ref Indirect Reference.
	 */
	IndiRef(const IndiRef& ref) : num(ref.num), gen(ref.gen)	{}

	/** 
	 * Constructor.
	 * Stores supplied identification and generation numbers.
	 * 
	 * @param ref Xpdf reference.
	 */
	IndiRef(const ::Ref& ref) : num(ref.num), gen(ref.gen) {}

	/** 
	 * Constructor.
	 * Stores supplied identification and generation numbers.
	 * 
	 * @param _num Object number.
	 * @param _gen Generation number.
	 */
	IndiRef(int _num, int _gen) : num(_num), gen(_gen) {}

	/** Copy constructor. */
	IndiRef& operator= (const IndiRef& _r) { num = _r.num; gen = _r.gen; return *this;}
	/** Equality operator. */
	bool operator== (const IndiRef& _r) const { return (num == _r.num && gen == _r.gen) ? true : false;}
			
} IndiRef;



/** 
 * Prints reference.
 * Prints given reference according to pdf specification.
 * 
 * @param out String where to print.
 * @param ref Reference to print.
 * @return reference to given string.
 */
inline std::ostream& 
operator<< (std::ostream& out, const IndiRef& ref)
{
	out << ref.num << " " << ref.gen;
	return out;
}


/** 
 * Prints reference.
 * Prints given xpdf reference in ref[num, gen] format.
 * 
 * @param out String where to print.
 * @param ref Reference to print.
 * @return reference to given string.
 */
inline std::ostream& 
operator<< (std::ostream& out, const ::Ref& ref)
{
	return out << IndiRef (ref);
}



//=====================================================================================
} // pdfobjects 
//=====================================================================================


#endif  //_INDIREF_H_
