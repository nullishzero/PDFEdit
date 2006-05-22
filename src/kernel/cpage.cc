// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80
/*
 * =====================================================================================
 *        Filename:  cpage.cc
 *     Description:  Pdf page object implemetation.
 *         Created:  03/20/2006 11:41:43 AM CET
 *          Author:  jmisutka (), 
 * =====================================================================================
 */

// static
#include "static.h"

// Page
#include "cpage.h"
// CContenteStream
#include "ccontentstream.h"
// Helper functions
#include "cobjecthelpers.h"

// =====================================================================================
namespace pdfobjects{
// =====================================================================================

using namespace std;
using namespace boost;
using namespace utils;

		
//
// Constructor
//
CPage::CPage (boost::shared_ptr<CDict>& pageDict) : dictionary(pageDict)
{
	kernelPrintDbg (debug::DBG_DBG, "");
}

//
// Get methods
//

//
//
//
Rectangle
CPage::getMediabox () const
{
	kernelPrintDbg (debug::DBG_DBG, "");
	
	// Get the array representing media box
	shared_ptr<IProperty> mbox = utils::getReferencedObject (dictionary->getProperty ("MediaBox"));
	assert (isArray (mbox));
	if (!isArray (mbox))
		throw MalformedFormatExeption ("Page::MediaBox is not array.");

	Rectangle rc;

  	rc.xleft  =	getDoubleFromArray (mbox, 0);
	rc.yleft  =	getDoubleFromArray (mbox, 1);
	rc.xright = getDoubleFromArray (mbox, 2);
	rc.yright = getDoubleFromArray (mbox, 3);

	return rc;
}

//
// \TODO magic constants 0,0, 1000, 1000
//
void
CPage::getText (std::string& text) const
{
	kernelPrintDbg (debug::DBG_DBG, "");

	// Create text output device
	boost::scoped_ptr<TextOutputDev> textDev (new ::TextOutputDev (NULL, gFalse, gFalse, gFalse));
	assert (textDev->isOk());
	if (!textDev->isOk())
		throw CObjInvalidOperation ();

	// Display page
	displayPage (*textDev);	

	// Init xpdf mess
	xpdf::openXpdfMess ();

	// Get the text
	boost::scoped_ptr<GString> gtxt (textDev->getText(0, 0, 1000, 1000));
	text = gtxt->getCString();
	
	// Uninit xpdf mess
	xpdf::closeXpdfMess ();
}


//
// Set methods
//

//
//
//
void
CPage::setMediabox (const Rectangle& rc)
{
	kernelPrintDbg (debug::DBG_DBG, " [" << rc << "]");

	// Get the array representing media box
	shared_ptr<IProperty> mbox = dictionary->getProperty ("MediaBox");
	assert (isArray(mbox));
	if (!isArray(mbox))
		throw MalformedFormatExeption ("Page::MediaBox is not array.");

  	setDoubleInArray (*mbox, 0, rc.xleft);
	setDoubleInArray (*mbox, 1, rc.yleft);
	setDoubleInArray (*mbox, 2, rc.xright);
	setDoubleInArray (*mbox, 3, rc.yright);
}

//
// Display a page
//
void
CPage::displayPage (::OutputDev& out, const DisplayParams params) const
{

	// Are we in valid pdf
	assert (hasValidPdf (dictionary));
	assert (hasValidRef (dictionary));
	if (!hasValidPdf (dictionary) || !hasValidRef (dictionary))
		throw XpdfInvalidObject ();

	// Get xref
	XRef* xref = dictionary->getPdf()->getCXref ();
	assert (NULL != xref);
	
	// Create xpdf object representing CPage
	//
	boost::scoped_ptr<Object> xpdfPage (dictionary->_makeXpdfObject());
	// Check page dictionary
	assert (objDict == xpdfPage->getType ());
	if (objDict != xpdfPage->getType ())
		throw XpdfInvalidObject ();
	
	// Get page dictionary
	Dict* xpdfPageDict = xpdfPage->getDict ();
	assert (NULL != xpdfPageDict);

	
	// Init xpdf
	xpdf::openXpdfMess ();

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
	page.display   (&out, params.hDpi, params.vDpi, 
					params.rotate, params.useMediaBox, 
					params.crop, NULL, xpdfCatalog.get());
	
	//
	// Cleanup
	// 

	// Clean xpdf mess
	xpdf::closeXpdfMess ();
	// Free object
	xpdfPage->free ();

}

//
// Helper methods
//

//
//
//
bool CPage::parseContentStream ()
{
	assert (hasValidRef(dictionary));
	assert (hasValidPdf (dictionary));
	if (!hasValidPdf(dictionary) || !hasValidRef(dictionary))
		throw CObjInvalidObject ();
	

	//
	// Init Gfx resources
	//

	// Init mess
	xpdf::openXpdfMess ();
	
	// Get resource dictionary
	boost::shared_ptr<IProperty> resources = 
		utils::getReferencedObject (dictionary->getProperty("Resources"));
	assert (isDict(resources));
	if (!isDict(resources))
		throw CObjInvalidObject ();
	
	// Start the resource stack
	XRef* xref = dictionary->getPdf()->getCXref();
	assert (xref);
	::Object* obj = resources->_makeXpdfObject ();
	assert (obj); assert (objDict == obj->getType());
	boost::shared_ptr<GfxResources> res (new GfxResources(xref, obj->getDict(), NULL));

	//
	// Init Gfx state
	//
	
	// Default values
	DisplayParams params;
	// Create Media (Bounding) box
	boost::shared_ptr<PDFRectangle> rc (new PDFRectangle (params.pageRect.xleft,  params.pageRect.yleft,
														  params.pageRect.xright, params.pageRect.yright));
	boost::shared_ptr<GfxState> state (new GfxState (params.hDpi, params.vDpi, rc.get(), params.rotate, params.upsideDown));

	// Close the mess
	xpdf::closeXpdfMess ();

	//
	// Get the stream representing content stream (if any), make an xpdf object
	// and finally instantiate CContentStream
	//
	shared_ptr<IProperty> contents;
	try 
	{
		contents = dictionary->getProperty ("Contents");

	}catch (ElementNotFoundException&)
	{
		kernelPrintDbg (debug::DBG_DBG, "No content stream found.");
		return false;
	}
	
	// If indirect, get the real object
	contents = utils::getReferencedObject (contents);
	
	if (contents)
	{
		//
		// Contents can be either stream or an array of streams
		//
		if (isStream (contents))	
		{
			shared_ptr<CStream> stream = IProperty::getSmartCObjectPtr<CStream> (contents); 
			// Create contentstream from a stream
			contentstreams.push_back (shared_ptr<CContentStream> (new CContentStream (stream,state,res)));
		
		}else if (isArray (contents))
		{
			// We can be sure that streams are indirect objects (pdf spec)
			shared_ptr<CArray> array = IProperty::getSmartCObjectPtr<CArray> (contents); 
			for (size_t i = 0; i < array->getPropertyCount(); ++i)
				contentstreams.push_back 
					(shared_ptr<CContentStream> (new CContentStream(getCStreamFromArray(array,i),state,res)) );
			
		}else // Neither stream nor array
		{
			kernelPrintDbg (debug::DBG_DBG, "Content stream type: " << contents->getType());
			throw ElementBadTypeException ("Bad content stream type.");
		}
	
	}else
		throw ElementBadTypeException ("Bad pointer to content stream.");

	// Everything went ok
	return true;
}


// =====================================================================================
} /* namespace pdfobjects */
// =====================================================================================
