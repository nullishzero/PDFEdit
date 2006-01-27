/*
 * =====================================================================================
 * 
 *        Filename:  debug.h
 * 
 *     Description:  debugging macros & stuff
 * 
 *         Version:  1.0
 *         Created:  01/19/2006 11:00:08 PM CET
 *        Revision:  none
 *        Compiler:  gcc
 * 
 *          Author:  jmisutka ()
 * 
 * =====================================================================================
 */

#ifndef DEBUG_H
#define DEBUG_H

#include <iostream>
#include <string>

/**
 * Static (compile-time) assertions with a solid debug message
 *
 * borrowed from Andrei Alexandrescu's book Modern C++ design
 * almost the same as in BOOST library
 */
template<bool>
struct CompileTimeChecker {CompileTimeChecker (...);};
template<> struct CompileTimeChecker<false> {};

#define STATIC_CHECK(expr, msg) 		\
		{								\
				class ERROR_##msg {};	\
				CompileTimeChecker<(expr) != 0> ERROR_##msg();\
				(void) ERROR_##msg;  \
		}


/** 
 * 0 - everything 
 */
#define __DEBUG_LEVEL		0
/**
 * Print debug output
 */
#define printDbg(dbgLevel,msg)	_printDbg((dbgLevel),std::cout,(msg));
#define _printDbg(dbgLevel,a,b)	\
{\
		if ( __DEBUG_LEVEL <= dbgLevel)\
			(a) << __FILE__ << ":" << __LINE__ << ": " << (b) << std::endl;\
}

//
// TODO: find out how to force gcc to make a function inline
// gcc -- can't be done?
//
/*__inline__ void
printDbg (std::ostream& out, const std::string& msg)
{
		out << __FILE__ << ":" << __LINE__ << ": " << msg << std::endl;
}
*/

#endif  // DEBUG_H
