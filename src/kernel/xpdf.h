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

#include <assert.h>

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


//
// Xpdf global variables
//

/** Init xpdf. */
inline void 
openXpdfMess ()
{
	//
	// Xpdf Global variable TFUJ!!!
	// REMARK: FUCKING xpdf uses global variable globalParams that uses another global
	// variable builtinFonts which causes that globalParams can NOT be nested
	// 
	assert (NULL == globalParams);
	globalParams = new ::GlobalParams (NULL);
	globalParams->setupBaseFonts (NULL);	
}

/** Uninit xpdf. */
inline void
closeXpdfMess ()
{
	// Clean-up
	assert (NULL != globalParams);
	delete globalParams;
	globalParams = NULL;
}

/** Use xpdf functions sensitive to global variables.. */
struct GlobalUseXpdf
{
	GlobalUseXpdf () {openXpdfMess ();};
	~GlobalUseXpdf () {closeXpdfMess ();};
};




//=====================================================================================
} // namespace xpdf
//=====================================================================================

#endif // _XPDF_H_

