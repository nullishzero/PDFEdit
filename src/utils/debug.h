// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80
/*
 * =====================================================================================
 *	  Filename:  debug.h
 *     Description:  debugging macros & stuff
 *	   Created:  01/19/2006 11:00:08 PM CET
 *	    Author:  jmisutka ()
 * =====================================================================================
 */

#ifndef DEBUG_H
#define DEBUG_H

#include <iostream>
#include <string>
#include <iomanip>


/**
 * Helper class for STATIC_CHECK.
 */

template<bool> struct CompileTimeChecker;
template<> struct CompileTimeChecker<true> { CompileTimeChecker(...) {}; };


/**
 * Compile time check.
 * Static (compile-time) assertions with a solid debug message.
 * 
 * borrowed AND MODIFIED from Andrei Alexandrescu's book Modern C++ design
 * almost the same as in BOOST library (the example in the book does not work)
 * 
 * @param expr	Expression to be evaluated.
 * @param msg	Message that will be printed if the expression evaluates
 *				to false. This has to be a valid variable name, e.g. 
 *				BAD_EXPRESSION, COMPILE_TIME_CHECK_FAILED...
 */
#define STATIC_CHECK(expr, msg)							\
	{													\
		CompileTimeChecker<(expr) != 0> (ERROR_##msg);	\
	}



/** Debug priority macros.
 *
 * TODO make available for group of debug priority.
 */

/** Panic situation priority.
 * After this kind of message, program usually ends without any resonable 
 * rescue routines. It should contain the cause of this state.
 */
const unsigned int DBG_PANIC 	= 0;

/** Critical error priority.
 * After this kind of message, program also ends but with coordinate way.
 */
const unsigned int  DBG_CRIT 	= 1;

/** Error message priority.
 *
 * Priority for casual errors.
 */
const unsigned int  DBG_ERR		= 2;

/** Warning massage priority.
 * Messages which contains comments to some non casual behaviour.
 */
const unsigned int  DBG_WARN 	= 3;

/** Information message priority.
 * It is designed for messages which informs about some important parts of
 * internals important also if we are not in debuging mode.
 */
const unsigned int  DBG_INFO 	= 4;

/** Debug message priority.
 * All debuging information should use this priority.
 */
const unsigned int  DBG_DBG 	= 5;


// may be defined from compilator 
#ifndef __DEBUG_LEVEL

/** Filter for message logging.
 *
 * Only messages with priority higher (lower number) then this filter are 
 * printed when printDbg (TODO link) macro is used.
 * <br>
 * DEBUG_<PRIORITY> macro should be used for value
 */
#define __DEBUG_LEVEL	0
#endif

/** Prints message with given priority.
 * @param dbgLevel Priority of message.
 * @param msg Message to dump.
 *
 * If given priority is enough (number is smaller than __DEBUG_LEVEL macro),
 * massage is printed out to the standard error output with following format:
 * @code 
 * priority:fileName:line: message
 * @endcode
 * <br>
 * <b>REMARKs</b><br>
 * Don't use variables for dbgLevel.<br>
 * Don't use direct priority numbers and use DBG_* macros instead.
 * msg has to have operator &gt&gt implemented (or must be convertable to one
 * that does).
 * <br>
 * Can be used in many ways.
 * \code
 * printDbg(DBG_DBG,"getExistingProperty();");
 *
 * printDbg(DBG_INFO, "Page moved to this location");
 * 
 * printDbg(DBG_WARN, "This should be done this way");
 *
 * printDbg(DBG_ERR, "Value of indirect object can't be reference");
 *
 * printDbg(DBG_CRIT, "Internal structures problem - program is about to exit");
 * 
 * printDbg(DBG_PANIC, "Memmory allocation problem");
 * \endcode
 *
 * REMARK: This is a macro, because we want to output line number and file name.
 * We can't force GCC to do inlining, we can just give a hint.
 */
#define printDbg(dbgLevel,msg)	_printDbg((dbgLevel),std::cerr,msg);

/** Helper macro used by printDbg.
 * @param dbgLevel Priority of message.
 * @param a Stream where to dump message.
 * @param b Message to dump.
 *
 * This macro should be used if different stream is about to be used than one
 * used in printDbg macro.
 */
#define _printDbg(dbgLevel,a,b)										\
{																	\
	if ( __DEBUG_LEVEL >= dbgLevel) 								\
	{																\
		(a) << dbgLevel <<":"<< __FILE__ << ":" << __LINE__ << ": "	\
			<<  b 													\
			<< std::endl;											\
	}																\
}


/**
 * If we want to print something more complicated just
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


namespace pdfobjects {
namespace debug {
		
//
// Returns name of objects type
//
template<int i> inline
std::string getStringType () {return "Unknown";}
template<> inline
std::string getStringType<0> () {return "pBool";}
template<> inline
std::string getStringType<1> () {return "pInt";}
template<> inline
std::string getStringType<2> () {return "pReal";}
template<> inline
std::string getStringType<3> () {return "pString";}
template<> inline
std::string getStringType<4> () {return "pName";}
template<> inline
std::string getStringType<5> () {return "pNull";}
template<> inline
std::string getStringType<9> () {return "pRef";}
template<> inline
std::string getStringType<6> () {return "pArray";}
template<> inline
std::string getStringType<7> () {return "pDict";}
template<> inline
std::string getStringType<8> () {return "pStream";}


} // namespace debug
} // namespace pdfobjects

#endif	// DEBUG_H
