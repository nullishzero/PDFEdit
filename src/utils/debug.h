/*
 * =====================================================================================
 *        Filename:  debug.h
 *     Description:  debugging macros & stuff
 *         Created:  01/19/2006 11:00:08 PM CET
 *          Author:  jmisutka ()
 * =====================================================================================
 */

#ifndef DEBUG_H
#define DEBUG_H

#include <iostream>
#include <string>

/**
 * Helper class for STATIC_CHECK.
 */
template<bool>
struct CompileTimeChecker {CompileTimeChecker (...);};
template<> struct CompileTimeChecker<false> {};

/**
 * Compile time check.
 * Static (compile-time) assertions with a solid debug message.
 * 
 * borrowed from Andrei Alexandrescu's book Modern C++ design
 * almost the same as in BOOST library
 * 
 * @param expr 	Expression to be evaluated.
 * @param msg	Message that will be printed if the expression evaluates
 * 				to false. This has to be a valid variable name, e.g. 
 * 				BAD_EXPRESSION, COMPILE_TIME_CHECK_FAILED...
 */
#define STATIC_CHECK(expr, msg) 		\
		{								\
				class ERROR_##msg {};	\
				CompileTimeChecker<(expr) != 0> ERROR_##msg();\
				(void) ERROR_##msg;  \
		}


/** 
 * 0 - everything 
 * 1 - less than everything
 * 2 - ...
 */
#define __DEBUG_LEVEL		0

/**
 * Print debug output.
 *
 * Can be used in many ways.
 * \code
 * printDbg (0,"getExistingProperty();");
 * printDbg (0,"setPropertyMapping(" << n << "," <<  g << ");");
 * \endcode
 *
 * @param dbgLevel 	Debug level that will be associated with this message
 * @param msg 		Message that will be printed.
 */
#define printDbg(dbgLevel,msg)				_printDbg((dbgLevel),std::cout,msg);
#define _printDbg(dbgLevel,a,b)	\
{\
		if ( __DEBUG_LEVEL <= dbgLevel) \
		{\
			(a) << __FILE__ << ":" << __LINE__ << ": ";\
			(a) <<  b ;\
			(a) << std::endl;\
		}\
}


/**
 * If we want to print sometheing more complicated just
 * overload this function
 */
inline void
__print (std::ostream& out,const std::string& str)
{
	out << str;
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
