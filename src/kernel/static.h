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

//
// boost
//
#include <boost/smart_ptr.hpp>
// Typelists THIS IS NOT AN STL VECTOR not any other similar RUN-TIME container
//#include <boost/mpl/vector.hpp>
//#include <boost/mpl/size.hpp>
//#include <boost/mpl/reverse.hpp>
//#include <boost/mpl/insert.hpp>
//#include <boost/mpl/at.hpp>


//
// our stuff
//
#include "utils/debug.h"
#include "utils/objectstorage.h"
#include "utils/observer.h"

#include "exceptions.h"

//#include "utils/iterator.h"
//#include "observer.h"
//#include "filters.h"
//#include "modecontroller.h"

#include "xpdf.h"

//#include "iproperty.h"

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

/** Coordinate. */
typedef double Coordinate;
/** Invalid coordinate. */
const double COORDINATE_INVALID = std::numeric_limits<Coordinate>::max();

/**
 * Point. 
 */
typedef struct Point
{
	Coordinate x;
	Coordinate y;
	// Constructor
	Point () {x = y = COORDINATE_INVALID;}
	Point (Coordinate _x, Coordinate _y) : x(_x), y(_y) {}

} Point;


/**
 * Rectangle structure. Defined as in pdf specification v1.5 (p. 133)
 */
typedef struct Rect
{
	Coordinate xleft;
	Coordinate yleft;
	Coordinate xright;
	Coordinate yright;

	// Constructor
	Rect ()	{xleft = yleft = xright = yright = COORDINATE_INVALID;}
	Rect (double x1, double y1, double x2, double y2) : xleft(x1), yleft(y1), xright(x2), yright(y2) {}

} Rectangle;

/**
 * Output rectangle.
 */
inline std::ostream& 
operator << (std::ostream& os, const Rectangle& rc)
{
	os 	<< "xleft: " << rc.xleft << " yleft: " << rc.yleft
		<< " xright: " << rc.xright << " yright: " << rc.yright ;
	
	return os;
}

/**
 * Output point.
 */
inline std::ostream& 
operator << (std::ostream& os, const Point& pt)
{
	os 	<< "x: " << pt.x << " y: " << pt.y;
	return os;
}





#endif // _STATIC_H_

