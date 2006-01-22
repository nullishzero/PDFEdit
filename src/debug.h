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
				(void)sizeof(CompileTimeChecker<(expr) != 0> ((ERROR_##msg())));\
		}
#endif  // DEBUG_H
