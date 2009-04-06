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
/*
 * =====================================================================================
 *        Filename:  basic_types.h
 *     Description:  
 *         Created:  25/11/2006 05:42:41 PM CET
 *          Author:  jmisutka ()
 * =====================================================================================
 */

#ifndef _BASIC_TYPES_H_
#define _BASIC_TYPES_H_


//============================================
namespace _JM_NAMESPACE {
//============================================


//============================================
// Basic types
//============================================

/** Null type. */
class NullType {};
/** Null type. */
struct EmptyType {};


/**
 * Correct null pointer (NULL) implementation (from CUJ 5/22 by Herb Sutter).
 */
const class 
{
public:
	template<typename T> operator T*() const { return 0; }
	template<typename C,typename T> operator T C::*() const { return 0; }
private:
	void operator&() const {} // not defined  

} NULLPTR = {};


//============================================
// Singleton idiom
//============================================

/**
 * Singleton idiom.
 *
 * Derived class must declere this class a friend.
 * Usage:
 *	Derived : public Singleton<Derived> {
 *		friend class Singleton<Derived>;
 */
template <class Derived>
class Singleton
{
   public:
     static Derived& instance ();

   protected:
     Singleton () {}

   private:
     Singleton (const Singleton&);
	 Singleton& operator=(const Singleton&);
};

template <class Derived>
Derived& Singleton<Derived>::instance()
{
  static Derived _instance;
  return _instance;
}


//============================================
} // namespace _JM_NAMESPACE
//============================================


#endif // _BASIC_TYPES_H_

