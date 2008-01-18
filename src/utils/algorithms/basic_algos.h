/*                                                                              
 * PDFedit - free program for PDF document manipulation.                        
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko, 
 *                                              Miroslav Jahoda,       
 *                                              Jozef Misutka, 
 *                                              Martin Petricek                                             
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit                                                                      
 */ 
/*
 * =====================================================================================
 *        Filename:  basic_algos.h
 *     Description: 
 *         Created:  26/11/2006 17:42:41 PM CET
 *          Author:  jmisutka ()
 * =====================================================================================
 */

#ifndef _BASIC_ALGOS_H_
#define _BASIC_ALGOS_H_ 


//============================================
namespace _JM_NAMESPACE {
//============================================


//=====================================================================================
// Standard min / max functions
//=====================================================================================

/** Min of four items. */
template<typename T>
inline T min (T a, T b, T c, T d)
	{ return std::min (std::min(a,b), std::min(c,d)); }

/** Max of four items. */
template<typename T>
inline T max (T a, T b, T c, T d)
	{ return std::max (std::max(a,b), std::max(c,d)); }

//============================================
} // namespace _JM_NAMESPACE
//============================================


#endif // _BASIC_ALGOS_H_ 
