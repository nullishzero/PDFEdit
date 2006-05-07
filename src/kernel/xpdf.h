// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80
/*
 * =====================================================================================
 *        Filename:  xpdf.h
 *     Description:  Header file containing all includes to xpdf
 *         Created:  03/07/2006 18:41:44 PM CET
 *          Author:  jmisutka ()
 * =====================================================================================
 */

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
#include <xpdf/GlobalParams.h>
#include <xpdf/Catalog.h>



//=====================================================================================

//=====================================================================================
namespace xpdf {
//=====================================================================================

	
/** \TODO
 * Wrapper around really really fucked up class with free method (e.g. Xpdf Object).
 */
template<typename T>
class MassiveIdiocyWrapper // : noncopyable 
{
private:
	T obj;
	
	// Disallow copy ctor
	MassiveIdiocyWrapper (const MassiveIdiocyWrapper&);
	const MassiveIdiocyWrapper& operator= (const MassiveIdiocyWrapper&);
	
public:	
	typedef T element_type;

	explicit MassiveIdiocyWrapper () {};

	//
	// Explicit delete
	//
	void reset () { obj.free (); };
	
	//
	// Dereference
	//
	T& operator*() /*const*/ { return obj; };
	T* operator->() /*const*/ { return &obj; };

	//
	// Raw pointer
	//
	T* get () /*const*/ { return &obj; };
	
	//
	// Delete
	//
	~MassiveIdiocyWrapper () { obj.free (); };
};

/** Xpdf object wrapper. */
typedef MassiveIdiocyWrapper<Object> XpdfObject;


//=====================================================================================
} // namespace xpdf
//=====================================================================================

#endif // _XPDF_H_

