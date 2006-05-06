// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80
/*
 * $RCSfile$
 *
 * $Log$
 * Revision 1.6  2006/05/06 08:48:50  hockm0bm
 * *PrintDbg macros really uses printDbg rather than _printDbg to dump information
 *   to default stream
 *
 *
 */
#include "debug.h"

// if default debug level doesn't come from gcc command line, we use
// CRIT
#ifndef DEFAULT_DEBUG_LEVEL
#define DEFAULT_DEBUG_LEVEL debug::DBG_CRIT
#endif

/** Prefix for debug messages. */
#define DEBUG_PREFIX "DEBUG"

namespace debug
{

unsigned int debugLevel = DEFAULT_DEBUG_LEVEL;

unsigned int changeDebugLevel(unsigned int level)
{
	// gets an old value and sets new
	unsigned int oldLevel=debugLevel;
	debugLevel=level;
	
	// logs change of level with DEBUG prefix to default output stream
	printDbg(DEBUG_PREFIX, DBG_INFO, "debugLevel has changed from "<<oldLevel<<" to "<<level);
	
	return oldLevel;    
}

}
