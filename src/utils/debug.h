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
#ifndef DEBUG_H
#define DEBUG_H

#include <iostream>
#include <string>
#include <iomanip>


// =============================================================================
namespace debug {
// =============================================================================

	
/** Helper class for STATIC_CHECK. */
template<bool> struct CompileTimeChecker;
template<> struct CompileTimeChecker<true> { CompileTimeChecker(...) {} };


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
#define STATIC_CHECK(expr, msg)						\
	{								\
		debug::CompileTimeChecker<(expr) != 0> (ERROR_##msg);	\
	}



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
const unsigned int  DBG_ERR	= 2;

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

/** Filter for message logging.
 *
 * Only messages with priority higher (lower number) than this filter are
 * printed when printDbg (TODO link) macro is used.
 * <br>
 * DEBUG_<PRIORITY> macro should be used for value. Value can be changed also
 * in runtime by changeDebugLevel function (or directly changing variable value
 * - first way is prefered one).
 */
extern unsigned int debugLevel;

/** Changes value of debugLevel.
 * @param level New value for debugLevel.
 *
 * Use DBG_* constants for parameter value.
 * <br>
 * Logs information about change.
 *
 * @return Old value of debugLevel.
 */
unsigned int changeDebugLevel(unsigned int level);

/** Prints message with given priority.
 * @param prefix Prefix for message.
 * @param dbgLevel Priority of message.
 * @param msg Message to dump.
 *
 * Wrapper to _printDbg macro with default stream where to put data. Don't use
 * _printDbg directly unless you want to print different than default stream.
 * <br>
 * <b>REMARKs</b><br>
 * Don't use variables for dbgLevel.<br>
 * Don't use direct priority numbers and use DBG_* macros instead.
 * msg has to have operator &gt&gt implemented (or must be convertable to one
 * that does).
 * <br>
 * Can be used in many ways.
 * \code
 * printDbg("", DBG_DBG,"getExistingProperty();");
 *
 * printDbg("", DBG_INFO, "Page moved to this location");
 *
 * printDbg("", DBG_WARN, "This should be done this way");
 *
 * printDbg("", DBG_ERR, "Value of indirect object can't be reference");
 *
 * printDbg("KERNEL", DBG_CRIT, "Internal structures problem - program is about to exit");
 *
 * printDbg("UTILS", DBG_PANIC, "Memmory allocation problem");
 * \endcode
 *
 * REMARK: This is a macro, because we want to output line number and file name.
 * We can't force GCC to do inlining, we can just give a hint.
 */
#define printDbg(prefix, dbgLevel,msg)	_printDbg((prefix),(dbgLevel),std::cerr,msg);

/** Alias to printDbg for kernel messages.
 * @param dbgLevel Priority of message.
 * @param msg Message to dump.
 *
 * Use this macro for all message important for kernel.
 */
#define kernelPrintDbg(dbgLevel, msg) printDbg("KERNEL", (dbgLevel), msg)

/** Alias to printDbg for gui messages.
 * @param dbgLevel Priority of message.
 * @param msg Message to dump.
 *
 * Use this macro for all message important for gui.
 */
#define guiPrintDbg(dbgLevel, msg) printDbg("GUI", (dbgLevel), msg)

/** Alias to printDbg for util messages.
 * @param dbgLevel Priority of message.
 * @param msg Message to dump.
 *
 * Use this macro for all message important for utils.
 */
#define utilsPrintDbg(dbgLevel, msg) printDbg("UTILS", (dbgLevel), msg)

/** Low level macro for debugging.
 * @param prefix Prefix string for message.
 * @param level Priority of message.
 * @param stream Stream where to dump message.
 * @param msg Message to dump.
 *
 * If given priority is enough (number is smaller than __DEBUG_LEVEL macro),
 * massage is printed out to the standard error output with following format:
 * @code
 * priority:prefix:fileName:functionName:line: message
 * @endcode
 */
#define _printDbg(prefix, level, stream, msg)					\
{										\
	if ( debug::debugLevel >= level) 					\
	{									\
		(stream) << level <<":"<<prefix<<":"				\
		    << __FILE__ << ":" << __FUNCTION__ <<":"<< __LINE__ 	\
			<< ": "							\
			<<  msg 						\
			<< std::endl;						\
	}									\
}

// =============================================================================
} // namespace debug
// =============================================================================



#endif	// DEBUG_H

