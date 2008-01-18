/*                                                                              
 * PDFedit - free program for PDF document manipulation.                        
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko, 
 *                                              Miroslav Jahoda,       
 *                                              Jozef Misutka, 
 *                                              Martin Petricek                                             
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit                                                                      
 */ 
// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80

#ifndef _UTILS_H_
#define _UTILS_H_

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

#endif // _UTILS_H_

