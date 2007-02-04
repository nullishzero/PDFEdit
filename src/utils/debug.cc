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
 * Revision 1.8  2007/02/04 20:17:03  mstsxfx
 * Common Licence comment for all cc and h files available in doc/licence_header
 * file and its content to all cc and h files in src/{gui,kernel,utils}
 * directories.
 * Xpdf code, QSA and QOutputDevice code are not touched
 *
 * Revision 1.7  2006/09/07 00:29:13  petrm1am
 *
 * modified default debug level
 *
 * Revision 1.6  2006/05/06 08:48:50  hockm0bm
 * *PrintDbg macros really uses printDbg rather than _printDbg to dump information
 *   to default stream
 *
 *
 */
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
