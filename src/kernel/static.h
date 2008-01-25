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

#ifndef _STATIC_H_
#define _STATIC_H_

//============================================
//  WINDOWS
//============================================

#ifdef WIN32
#include <os/win.h>
#endif

//============================================
//  POSIX
//============================================
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

//============================================
// stl
//============================================
#include <ostream>
#include <fstream>
#include <iostream>
#include <sstream>

#include <string>
#include <iomanip>

#include <list>
#include <vector>
#include <deque>
#include <map>
#include <set>

#include <limits>

//============================================
// boost
//============================================
#include <boost/smart_ptr.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/filtering_stream.hpp>


//============================================
// our stuff
//============================================

//this includes <windows.h> - BEWARE of max min
#include "kernel/pdfspecification.h"
#include "kernel/xpdf.h"

#ifdef max
	#undef max
#endif
#ifdef min
	#undef min
#endif
// basic algorithms
using std::max;
using std::min;


// utils
#include "utils/debug.h"
#include "utils/objectstorage.h"
#include "utils/observer.h"

// types
#define _JM_NAMESPACE libs	 
#include "utils/algorithms.h"
#include "utils/types.h"
using libs::Point;
//using libs::Rectangle;
using boost::noncopyable;
using libs::NullType;

// other includes
#include "kernel/exceptions.h"
#include "kernel/utils.h"


#endif // _STATIC_H_

