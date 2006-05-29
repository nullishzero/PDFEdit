// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80
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
// Filters
#include <boost/iostreams/categories.hpp> // output_filter_tag
#include <boost/iostreams/operations.hpp> // put
//#include <boost/iostreams/concepts.hpp> // multichar_output_filter
//#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/filtering_stream.hpp>
//#include <boost/iostreams/filtering_streambuf.hpp>

//
// our stuff
//
#include "utils/debug.h"
#include "utils/objectstorage.h"
#include "utils/observer.h"

#include "exceptions.h"
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



//=====================================================================================

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
template <typename Coord>
struct GenRect
{
	Coord xleft;
	Coord yleft;
	Coord xright;
	Coord yright;

	// Constructor
	GenRect ()	{xleft = yleft = xright = yright = COORDINATE_INVALID;}
	GenRect (Coord x1, Coord y1, Coord x2, Coord y2) : xleft(x1), yleft(y1), xright(x2), yright(y2) {}

	/** 
	 * True if the rectangle contains the point.
	 * 
	 * @param x x-coordinate of point.
	 * @param y y-coordinate of point.
	 *
	 * @return True if the specified point is in the rectangle, false otherwise.
	 */
	bool contains (Coord x, Coord y) const
	{
		//kernelPrintDbg (debug::DBG_DBG, "x: " << x << " y: " << y << " Rect:" << *this);
		return ( std::min(xleft,xright) <= x && x <= std::max(xleft,xright) 
				&& std::min (yleft,yright) <= y && y <= std::max(yleft,yright) );
	}

	/** Equality operator. */
	bool operator== (const GenRect<Coord>& rc) const
	{
		return (xleft == rc.xleft && xright == rc.xright &&
				yleft == rc.yleft && yright == rc.yright);
	}

	
};

/** Our specialization of rect. */
typedef struct GenRect<Coordinate> Rectangle;

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


//=====================================================================================
//	Char buffer that can contain null characters
//=====================================================================================

typedef boost::shared_ptr<char> CharBuffer;
struct char_buffer_delete
	{void operator() (char* p) {assert (p); delete [] p;};};
inline char* char_buffer_new (size_t l) {return new char [l];}



//=====================================================================================

//
// From boost
//
class noncopyable
{
protected:
	noncopyable() {}
	~noncopyable() {}
private:  // emphasize the following members are private
	noncopyable( const noncopyable& );
	const noncopyable& operator=( const noncopyable& );
};



#endif // _STATIC_H_

