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
// Class copy constructor abbreviations
//============================================

/**
 * This empty class ensures that the derived object can not use copy
 * constructor.
 *
 * Idea from from boost library.
 */
class noncopyable
{
protected:
	noncopyable() {}
	~noncopyable() {}
private:  // emphasize the following members are private
	noncopyable( const noncopyable& );
	const noncopyable& operator=( const noncopyable& );
};

//============================================
} // namespace _JM_NAMESPACE
//============================================


#endif // _BASIC_TYPES_H_

