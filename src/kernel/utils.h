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

#ifndef _UTILS_H_
#define _UTILS_H_

#include "utils/types.h"

//=====================================================================================
// Deallocator
//=====================================================================================

/** 
 * Empty deallocator.
 * This functor can be used with smart pointers when instance cannot be deallocated.
 */
template<typename T> struct EmptyDeallocator
{
	/** Deallocation function operator. */
	void operator()(T * ){}
};


//=====================================================================================
//	Char buffer that can contain null characters
//=====================================================================================

/** 
 * Special char buffer that can contain null characters. 
 *
 * Standard string class can also contain null characters but consider the
 * following pitfall:
 *	 
 * \code
 * string nul = "\0";
 * char cnul = '\0';
 *
 * str = "123";
 * str += nul; // "123" size: 3 
 * str += cnul // "123\0" size: 4
 * \endcode
 */
typedef boost::shared_ptr<char> CharBuffer;

/** Char buffer deleter. */
struct char_buffer_delete
	{void operator() (char* p) {assert (p); delete [] p;}};
/** Char buffer alocator. */
inline char* char_buffer_new (size_t l) {return new char [l];}


//==========================================================
// Somewhat special bit functions.
//==========================================================

/**
 * Is n-th bit set.
 * 
 * @param value Value to be checked.
 * @param mask N-th bit.
 *
 * @return True if set, false otherwise.
 */
template<class T, class U>
inline bool isBitSet(T value, U mask)
	{ 	// compare number of bits of where we want to store bit with the position 
		assert ((int)(sizeof(short) * 8) > mask);
		return (value & ((unsigned short)0x1 << mask)) != 0;}

/** Reset all bits. */
inline unsigned short setNoneBitsShort()
	{ return 0x0;}

/**
 * Set n-th set.
 * 
 * @param mask N-th bit.
 * @return Value with n-th bit set.
 */
template<class U>
inline unsigned short setNthBitsShort(U mask)
	{ 	// compare number of bits of where we want to store bit with the position 
		assert ((int)(sizeof(short) * 8) > mask);
		return ((unsigned short) 0x1 << mask);}

template<class U>
inline unsigned short setNthBitsShort(U mask, U mask1)
	{ return setNthBitsShort (mask) | setNthBitsShort (mask1); }
template<class U>
inline unsigned short setNthBitsShort(U mask, U mask1, U mask2)
	{ return setNthBitsShort (mask,mask1) | setNthBitsShort (mask2); }
template<class U>
inline unsigned short setNthBitsShort(U mask, U mask1, U mask2, U mask3)
	{ return setNthBitsShort (mask,mask1,mask2) | setNthBitsShort (mask3); }
template<class U>
inline unsigned short setNthBitsShort(U mask, U mask1, U mask2, U mask3, U mask4)
	{ return setNthBitsShort (mask,mask1,mask2,mask3) | setNthBitsShort (mask4); }


//=====================================================================================
// Comparators Point/Rectangle
//=====================================================================================

/** 
 * Comparator that we can use to find out if another rectangle intersects
 * rectangle specified by this comparator.
 */
struct PdfOpCmpRc
{
	/** 
	 * Consructor. 
	 *
	 * @param rc Rectangle used when comparing.
	 */
	PdfOpCmpRc (const _JM_NAMESPACE::Rectangle& rc) : rc_(rc) {}
	
	/** 
	 * Is the intersection of supplied rectangle and rectangle specified in the
	 * constructor not empty. 
	 *
	 * Our rectangle does NOT contain another rectangle if
	 * <ul>
	 * 	<li>min (xleft-our, xright-our) >= min (xleft, xright)</li>
	 * 	<li>max (xleft-our, xright-our) <= max (xleft, xright)</li>
	 * 	<li>min (yleft-our, yright-our) >= min (yleft, yright)</li>
	 * 	<li>max (yleft-our, yright-our) <= max (yleft, yright)</li>
	 * </ul>
	 */
	bool operator() (const _JM_NAMESPACE::Rectangle& rc) const
		{ return _JM_NAMESPACE::Rectangle::isInitialized (_JM_NAMESPACE::rectangle_intersect (rc_, rc)); }

private:
	const _JM_NAMESPACE::Rectangle rc_;	/**< Rectangle to be compared. */
};


/** 
 * Comparator that we can use to find out if a rectange contains point specified
 * by this comparator.
 */
struct PdfOpCmpPt
{
	/** 
	 * Consructor. 
	 * 
	 * @param pt Point that we use when comparing.
	 */
	PdfOpCmpPt (const Point& pt) : pt_(pt) {}
	
	/** 
	 * Is point in a rectangle. 
	 * 
	 * @param rc Rectangle.
	 */
	bool operator() (const _JM_NAMESPACE::Rectangle& rc) const
	{
		return (rc.contains (pt_.x, pt_.y));
	}

private:
	const Point pt_;	/**< Point to be compared. */
};


//=====================================================================================
// Comparators Point/Rectangle
//=====================================================================================

/** Case insensitive comparator. */
inline bool 
nocase_compare (char c1, char c2)
{
	return toupper(c1) == toupper(c2);
}


#endif // _UTILS_H_

