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
	openXpdfMess ();

	// Get the text
	boost::scoped_ptr<GString> gtxt (textDev->getText(0, 0, 1000, 1000));
	text = gtxt->getCString();
	
	// Uninit xpdf mess
	closeXpdfMess ();
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
	// Init xpdf
	openXpdfMess ();

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
	
	// Free object
	xpdfPage->free ();
	// Clean xpdf mess
	closeXpdfMess ();

}

//
// Helper methods
//

//
//
//
bool CPage::parseContentStream ()
{
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
		// No content stream return
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
			contentstreams.push_back (shared_ptr<CContentStream> (new CContentStream (stream)));
		
		}else if (isArray (contents))
		{
			// We can be sure that streams are indirect objects (pdf spec)
			shared_ptr<CArray> array = IProperty::getSmartCObjectPtr<CArray> (contents); 
			for (size_t i = 0; i < array->getPropertyCount(); ++i)
				contentstreams.push_back 
					(shared_ptr<CContentStream> (new CContentStream(getCStreamFromArray(array,i))) );
			
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

//
// Xpdf mess methods
//

//
//
//
void
CPage::openXpdfMess () const
{
	//
	// Xpdf Global variable TFUJ!!!
	// REMARK: FUCKING xpdf uses global variable globalParams that uses another global
	// variable builtinFonts which causes that globalParams can NOT be nested
	// 
	assert (NULL == globalParams);
	globalParams = new GlobalParams (NULL);
	globalParams->setupBaseFonts (NULL);	
}

//
//
//
void
CPage::closeXpdfMess () const
{
	// Clean-up
	assert (NULL != globalParams);
	delete globalParams;
	globalParams = NULL;
}

// =====================================================================================
} /* namespace pdfobjects */
// =====================================================================================
