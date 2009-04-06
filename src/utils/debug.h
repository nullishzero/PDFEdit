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


// if default debug level doesn't come from gcc command line, we use
// ERR
#ifndef DEFAULT_DEBUG_LEVEL
#define DEFAULT_DEBUG_LEVEL debug::DBG_ERR
#endif

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

/** API massage priority.
 */
const unsigned int  DBG_API 	= 3;

/** Information message priority.
 * It is designed for messages which informs about some important parts of
 * internals important also if we are not in debuging mode.
 */
const unsigned int  DBG_INFO 	= 4;

/** Debug message priority.
 * All debuging information should use this priority.
 */
const unsigned int  DBG_DBG 	= 5;

/** Target for debugging.
 * This simple structure contains filter debug level and stream for data.
 */
struct DebugTarget 
{
	/** Filter for message logging.
	 *
	 * Only messages with priority higher (lower number) than this filter are
	 * printed when printDbg (TODO link) macro is used.
	 * <br>
	 * DEBUG_<PRIORITY> macro should be used for value. Value can be changed also
	 * in runtime by changeDebugLevel function (or directly changing variable value
	 * - first way is prefered one).
	 */
	unsigned int debugLevel;

	/** Stream for data. */
	std::ostream &stream;

	/** Defaul constructor.
	 * Initializes debugLevel to DEFAULT_DEBUG_LEVEL and stream to 
	 * the standard error.
	 */
	DebugTarget():debugLevel(DEFAULT_DEBUG_LEVEL), stream(std::cerr) {}

	/** Constructor for full initialization.
	 * @param level Debug level to be used.
	 * @param s Stream to be used.
	 */
	DebugTarget(unsigned int level, std::ostream & s): debugLevel(level), stream(s) {}
};

/** Debug target for kernel. */
extern DebugTarget kernelDebugTarget;
/** Debug target for gui. */
extern DebugTarget guiDebugTarget;
/** Debug target for utils. */
extern DebugTarget utilsDebugTarget;

/** Changes value of debugLevel for given debug target.
 * @param debugTarget Debug target to update.
 * @param level New value for debugLevel.
 *
 * Use DBG_* constants for parameter value.
 * <br>
 * Logs information about change.
 *
 * @return Old value of debugLevel.
 */
unsigned int changeDebugLevel(DebugTarget & debugTarget, unsigned int level);

/** Changes value of debugLevel for all standard debug targets.
 * @param level New value for debugLevel.
 */
void changeDebugLevel(unsigned int level);

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
#define printDbg(prefix, dbgLevel,target, msg)	_printDbg((prefix), (dbgLevel), (target), msg)

/** Alias to printDbg for kernel messages.
 * @param dbgLevel Priority of message.
 * @param msg Message to dump.
 *
 * Use this macro for all message important for kernel.
 */
#define kernelPrintDbg(dbgLevel, msg) printDbg("KERNEL", (dbgLevel), debug::kernelDebugTarget, msg)

/** Alias to printDbg for gui messages.
 * @param dbgLevel Priority of message.
 * @param msg Message to dump.
 *
 * Use this macro for all message important for gui.
 */
#define guiPrintDbg(dbgLevel, msg) printDbg("GUI", (dbgLevel), debug::guiDebugTarget, msg)

/** Alias to printDbg for util messages.
 * @param dbgLevel Priority of message.
 * @param msg Message to dump.
 *
 * Use this macro for all message important for utils.
 */
#define utilsPrintDbg(dbgLevel, msg) printDbg("UTILS", (dbgLevel), debug::utilsDebugTarget, msg)

/** Low level macro for debugging.
 * @param prefix Prefix string for message.
 * @param level Priority of message.
 * @param target Target for debug messages (reference).
 * @param msg Message to dump.
 *
 * If given priority is enough (number is smaller than target::debugLevel),
 * massage is printed out to the target stream with following format:
 * @code
 * priority:prefix:fileName:functionName:line: message
 * @endcode
 */
#define _printDbg(prefix, level, target, msg)					\
	do {									\
	if (target.debugLevel >= level) { 					\
		target.stream << level <<":"<<prefix<<":"			\
		    << __FILE__ << ":" << __FUNCTION__ <<":"<< __LINE__ 	\
			<< ": "							\
			<<  msg 						\
			<< std::endl;						\
	}									\
	}while(0)


#undef DEFAULT_DEBUG_LEVEL

// =============================================================================
} // namespace debug
// =============================================================================



#endif	// DEBUG_H

