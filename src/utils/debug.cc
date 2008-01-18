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
#include "kernel/static.h" // WIN32 port - precompiled headers - REMOVE IN FUTURE!
#include "debug.h"

// if default debug level doesn't come from gcc command line, we use
// ERR
#ifndef DEFAULT_DEBUG_LEVEL
#define DEFAULT_DEBUG_LEVEL debug::DBG_ERR
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
