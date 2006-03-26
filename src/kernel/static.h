/*
 * =====================================================================================
 *        Filename:  xpdf.h
 *     Description:  Header file containing all includes to xpdf
 *         Created:  03/07/2006 18:42:41 PM CET
 *          Author:  jmisutka ()
 * =====================================================================================
 */

#ifndef _STATIC_H_
#define _STATIC_H_

//
// stl
//
#include <ostream>
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

//
// boost
//
#include <boost/smart_ptr.hpp>
// Typelists THIS IS NOT AN STL VECTOR not any other similar RUN-TIME container
#include <boost/mpl/vector_c.hpp>
#include <boost/mpl/vector/vector0_c.hpp>
#include <boost/mpl/vector/vector10_c.hpp>
#include <boost/mpl/size.hpp>
#include <boost/mpl/reverse.hpp>
#include <boost/mpl/at.hpp>


//
// our stuff
//
#include "utils/debug.h"
#include "utils/objectstorage.h"

#include "observer.h"
#include "exceptions.h"
#include "filters.h"
#include "modecontroller.h"

#include "xpdf.h"


//============================================
// 
// Types
//

//
// Null and empty types.
// 
class NullType {};
struct EmptyType {};


//
// from CUJ 5/22, Herb Sutter ...
// 
const class 
{
public:
	template<typename T> operator T*() const { return 0; }
	template<typename C,typename T> operator T C::*() const { return 0; }
private:
	void operator&() const {}; // not defined  

} nullPtr = {};



#endif // _STATIC_H_

