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

// static
#include "kernel/static.h"
 
#include "kernel/cpagedisplay.h"

#include "kernel/cpage.h"
#include "kernel/cpdf.h"
#include "kernel/cpageattributes.h"

// =====================================================================================
namespace pdfobjects {
// =====================================================================================

using namespace boost;
using namespace utils;

//
//
//
void 
CPageDisplay::setDisplayParams (const DisplayParams& dp)
{ 
		if (_params == dp)
			return;

	// TODO ROTATION !!!!

	bool need_reparse = false;
	if (_params.hDpi != dp.hDpi || _params.vDpi != dp.vDpi)
		need_reparse = true;

	_params = dp; 
	// set rotate to positive integer
	_params.rotate -= ((int)(_params.rotate / 360) -1) * 360;
	// set rotate to range [ 0, 360 )
	_params.rotate -= ((int)_params.rotate / 360) * 360;
	// Use mediabox
	if (_params.useMediaBox)
		_params.pageRect = _page->getMediabox();

	// Update bboxes etc... 
	if (need_reparse)
		_page->contents()->reparse ();
}


//
//
//
void 
CPageDisplay::displayPage (::OutputDev& out, int x, int y, int w, int h) 
{
	displayPage (out,_page->getDictionary(),x,y,w,h);
}

//
// Display a page
//
void
CPageDisplay::displayPage (::OutputDev& out, 
						   shared_ptr<CDict> pagedict, 
						   int x, int y, int w, int h)
{
	// Get xref
	shared_ptr<CPdf> pdf = pagedict->getPdf().lock();
	XRef* xref = (pdf)?pdf->getCXref ():NULL;
	assert (NULL != xref);
	if (!(pagedict))
		throw XpdfInvalidObject ();

	//
	// Create xpdf object representing CPage
	//
	shared_ptr<Object> xpdfPage (pagedict->_makeXpdfObject(), xpdf::object_deleter());
		// Check page dictionary
		assert (objDict == xpdfPage->getType());
		if (objDict != xpdfPage->getType ())
			throw XpdfInvalidObject ();
	
	// Get page dictionary
	const Dict* xpdfPageDict = xpdfPage->getDict ();
		assert (NULL != xpdfPageDict);

	
	//
	// We need to handle special case
	//
	SplashOutputDev* sout = dynamic_cast<SplashOutputDev*> (&out);
	if (sout)
		sout->startDoc (xref);

	//
	// Create default page attributes and make page
	// ATTRIBUTES are deleted in Page destructor
	// 
	Page page (xref, 0, xpdfPageDict, new PageAttrs (NULL, xpdfPageDict));
	
	// Create catalog
	scoped_ptr<Catalog> xpdfCatalog (new Catalog (xref));
	
	//
	// Page object display (..., useMediaBox, crop, links, catalog)
	//
	// TODO ROTATION !! int rotation = _params.rotate - pagedict->getRotation ();
	page.displaySlice (&out, _params.hDpi, _params.vDpi,
			0, _params.useMediaBox, _params.crop,
			x, y, w, h, 
			false, xpdfCatalog.get());

}



//
//
//
void 
CPageDisplay::createXpdfDisplayParams (shared_ptr<GfxResources>& res, shared_ptr<GfxState>& state)
{
	//
	// Init Gfx resources
	//

	// Get resource dictionary
	CPageAttributes::InheritedAttributes atr;
	CPageAttributes::fillInherited (_page->getDictionary(),atr);
	
	// Start the resource stack
	shared_ptr<CPdf> pdf = _page->getDictionary()->getPdf().lock();
	XRef* xref = (pdf)?pdf->getCXref():NULL;
	assert (xref);
	Object* obj = atr._resources->_makeXpdfObject ();
	assert (obj); 
	assert (objDict == obj->getType());
	res = shared_ptr<GfxResources> (new GfxResources(xref, obj->getDict(), NULL));
	xpdf::freeXpdfObject (obj);
	
	//
	// Init Gfx state
	//
	
	// Create Media (Bounding) box
	shared_ptr<PDFRectangle> rc (new PDFRectangle (_params.pageRect.xleft,  _params.pageRect.yleft,
												  _params.pageRect.xright, _params.pageRect.yright));
	state = shared_ptr<GfxState> (new GfxState (_params.hDpi, _params.vDpi, 
												rc.get(), _params.rotate, _params.upsideDown));
}


//
//
//
int
CPageDisplay::getRotation () const
{
	CPageAttributes::InheritedAttributes atr;
	CPageAttributes::fillInherited (_page->getDictionary(),atr);
	return atr._rotate->getValue();
}


//
//
//
void
CPageDisplay::setRotation (int rot)
{
	CInt r (rot);
	_page->getDictionary()->setProperty (Specification::Page::ROTATE, r);
}


//
//
//
libs::Rectangle
CPageDisplay::getMediabox () const
{
		kernelPrintDbg (debug::DBG_DBG, "");
	
	CPageAttributes::InheritedAttributes atr;
	CPageAttributes::fillInherited (_page->getDictionary(),atr);
	libs::Rectangle rc;
	
  	rc.xleft  =	getDoubleFromArray (atr._mediaBox, 0);
	rc.yleft  =	getDoubleFromArray (atr._mediaBox, 1);
	rc.xright = getDoubleFromArray (atr._mediaBox, 2);
	rc.yright = getDoubleFromArray (atr._mediaBox, 3);

	return rc;
}

libs::Rectangle
CPageDisplay::getPageRect () const
{
	return _params.pageRect;
}

//
//
//
void
CPageDisplay::setMediabox (const libs::Rectangle& rc)
{
		kernelPrintDbg (debug::DBG_DBG, " [" << rc << "]");
	
	CArray mb;
	CReal r (rc.xleft);
	mb.addProperty (r);
	r.setValue (rc.yleft);
	mb.addProperty (r);
	r.setValue (rc.xright);
	mb.addProperty (r);
	r.setValue (rc.yright);
	mb.addProperty (r);
		
	boost::shared_ptr<CDict> dict = _page->getDictionary();
	dict->setProperty (Specification::Page::MEDIABOX,mb);

	// We should sync all generally used Boxes (CropBox and TrimBox) 
	// to the default one (MediaBox)
	// This is not perfect because someone could set it to different 
	// value intentionaly, but this is smaller problem than bad printing
	// size when MediaBox is enlarged [see bt#290]
	if (dict->containsProperty (Specification::Page::CROPBOX))
		dict->setProperty (Specification::Page::CROPBOX, mb);
	if (dict->containsProperty (Specification::Page::TRIMBOX))
		dict->setProperty (Specification::Page::TRIMBOX, mb);
}

//
// Transform matrix
// 

//
//
//
void
CPageDisplay::setTransformMatrix (double tm[6])
{
	//
	// Create new cm operator
	//
	PdfOperator::Operands operands;
	operands.push_back (shared_ptr<IProperty> (new CReal (tm[0])));
	operands.push_back (shared_ptr<IProperty> (new CReal (tm[1])));
	operands.push_back (shared_ptr<IProperty> (new CReal (tm[2])));
	operands.push_back (shared_ptr<IProperty> (new CReal (tm[3])));
	operands.push_back (shared_ptr<IProperty> (new CReal (tm[4])));
	operands.push_back (shared_ptr<IProperty> (new CReal (tm[5])));
	shared_ptr<PdfOperator> cmop = createOperator("cm", operands);

	// Insert at the beginning
	_page->contents()->getContentStream((size_t)0)->frontInsertOperator (cmop);
}


// =====================================================================================
} // namespace pdfobjects
// =====================================================================================
