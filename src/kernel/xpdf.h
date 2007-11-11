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

#ifndef _XPDF_H_
#define _XPDF_H_


#include <xpdf/Object.h>
#include <xpdf/Parser.h>
#include <xpdf/Lexer.h>
#include <xpdf/Stream.h>
#include <xpdf/XRef.h>
#include <xpdf/Gfx.h>
#include <xpdf/GfxState.h>
#include <xpdf/GfxFont.h>
#include <xpdf/ErrorCodes.h>
#include <xpdf/Page.h>
#include <xpdf/TextOutputDev.h>
#include <xpdf/SplashOutputDev.h>
// Note that GlobalParams::initGlobalParams has to be called before
// we can use globalParams
#include <xpdf/GlobalParams.h>
#include <xpdf/Catalog.h>

#include <assert.h>

//=====================================================================================

//=====================================================================================
namespace xpdf {
//=====================================================================================

/**
 * Xpdf object deleter.
 */
struct object_deleter
{
	void operator() (::Object* o)
		{ assert (o); o->free(); ::gfree(o); }
};
					
	
/** 
 * Wrapper around a class which uses free method.
 *
 * This class uses a special free method to deallocate objects. 
 * In xpdf Object class the free method is necessary because Object implements reference 
 * counting.
 */
template<typename T>
class MassiveIdiocyWrapper // : noncopyable 
{
private:
	T obj;
	
	/** Disallow copy ctor. */
	MassiveIdiocyWrapper (const MassiveIdiocyWrapper&);
	/** Disallow copy ctor. */
	const MassiveIdiocyWrapper& operator= (const MassiveIdiocyWrapper&);
	
public:	
	typedef T element_type;

	/** Constructor. */
	MassiveIdiocyWrapper () {}

	/** Explicit delete.	*/
	void reset () { obj.free (); }
	
	/** Dereference. */
	T& operator*() /*const*/ { return obj; }
	/** Dereference. */
	T* operator->() /*const*/ { return &obj; }

	/** Get raw pointer. */
	T* get () /*const*/ { return &obj; }
	
	/** Destructor. */
	~MassiveIdiocyWrapper () { obj.free (); }
};

/** Xpdf object wrapper. */
typedef MassiveIdiocyWrapper<Object> XpdfObject;


//=====================================================================================
} // namespace xpdf
//=====================================================================================

#endif // _XPDF_H_
