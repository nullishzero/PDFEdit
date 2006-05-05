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
