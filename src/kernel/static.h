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

#ifndef _STATIC_H_
#define _STATIC_H_

//============================================
//  WINDOWS
//============================================

#include <os/compiler.h>

#ifdef WIN32
#include <os/win.h>
#endif

#ifdef _POSIX_SOURCE
#include <os/posix.h>
#endif

//============================================
//  POSIX
//============================================
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
#include <errno.h>
#include <cstring>
#include <stdio.h>

//============================================
// stl
//============================================
#include <ostream>
#include <fstream>
#include <iostream>
#include <sstream>

#include <string>
#include <cstring>
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
#include <boost/noncopyable.hpp>
#include <boost/algorithm/string/replace.hpp>

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
#include "utils/confparser.h"
#include "utils/rulesmanager.h"


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

