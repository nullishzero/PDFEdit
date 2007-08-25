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
/*
 * $RCSfile$
 *
 * $Log$
 * Revision 1.23  2007/08/25 14:43:47  mstsxfx
 * formating
 *
 * Revision 1.22  2007/04/25 18:24:35  bilboq
 *
 * applied patch from Martin Michlmayr to fix compilation with gcc-4.3
 *
 * fixed some typos in comments
 *
 * Revision 1.21  2007/02/04 20:17:03  mstsxfx
 * Common Licence comment for all cc and h files available in doc/licence_header
 * file and its content to all cc and h files in src/{gui,kernel,utils}
 * directories.
 * Xpdf code, QSA and QOutputDevice code are not touched
 *
 * Revision 1.20  2006/07/02 14:10:57  misuj1am
 *
 * -- docs updated
 *
 * Revision 1.19  2006/06/29 22:23:41  misuj1am
 *
 * -- code cleanup + doc updated
 *
 * Revision 1.18  2006/06/17 15:40:07  misuj1am
 *
 * -- minor changes
 *
 * Revision 1.17  2006/06/10 16:10:46  misuj1am
 *
 * -- typo fixed
 *
 * Revision 1.16  2006/06/10 14:14:10  misuj1am
 *
 * -- ADD: general memor checker
 *
 * Revision 1.15  2006/05/13 22:22:11  hockm0bm
 * * getStringType removed (to iproperty.h)
 *
 * Revision 1.14  2006/05/06 08:48:50  hockm0bm
 * *PrintDbg macros really uses printDbg rather than _printDbg to dump information
 *   to default stream
 *
 * Revision 1.13  2006/05/05 12:02:04  petrm1am
 *
 * Committing changes from Michal:
 *
 * * printDbg changed to final state (prefix parameter added)
 * * changeDebugLevel function added
 * * *PrintDbg functions uses printDbg instead of _printDbg macro to keep
 * default output stream
 * * default debug level set to DBG_CRIT if macro DEFAULT_DEBUG_LEVEL not
 * specified
 *
 * Revision 1.12  2006/05/01 13:16:11  hockm0bm
 * minor changes
 *         - debugLevel is not marked as extern in cc file
 *         - *PrintDbg doesn't use braces for msg parameter
 *
 * Revision 1.11  2006/05/01 12:31:23  hockm0bm
 * Implementation of printDbg changed
 *         - debugLevel changed from macro to external variable defined in debug.cc
 *         - printDbg has new prefix parameter - for easy transition, it is not
 *           used now
 *         - _prinDbg helper has new prefix parameter - it is used and all printDbg
 *           macros should use this macro
 *         - new macros (aliases) kernelPrintDbg, guiPrintDbg, utilsPrintDbg
 *
 * TODO - when all old style printDbg are replaced by new one, printDbg should have
 * prefix parameter
 *
 *
 */

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
#define STATIC_CHECK(expr, msg)									\
	{															\
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

