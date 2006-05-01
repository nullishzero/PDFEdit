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
	printDbg (debug::DBG_DBG, "");

	// Parse the content stream if found
	//parseContentStream ();	
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
	printDbg (debug::DBG_DBG, "");
	
	// Get the array representing media box
	shared_ptr<IProperty> mbox = dictionary->getProperty ("MediaBox");
	assert (isArray (mbox));
	if (!isArray (mbox))
		throw MalformedFormatExeption ("Page::MediaBox is not array.");

	Rectangle rc;

  	rc.xleft  =	getDoubleFromArray (mbox, 0);
	rc.xright = getDoubleFromArray (mbox, 1);
	rc.yleft  =	getDoubleFromArray (mbox, 2);
	rc.yright = getDoubleFromArray (mbox, 3);

	return rc;
}

//
// Get contents stream.
//
boost::shared_ptr<CContentStream> 
CPage::getContentStream ()
{ 
	// If contentstrea exists && is not empty and not changed return it
	if (contentstream && !contentstream->invalid())
	{
		return contentstream; 
	
	}else
	{
		contentstream.reset ();
		parseContentStream ();
		return contentstream; 
	}
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
	printDbg (debug::DBG_DBG, " [" << rc << "]");

	// Get the array representing media box
	shared_ptr<IProperty> mbox = dictionary->getProperty ("MediaBox");
	assert (isArray(mbox));
	if (!isArray(mbox))
		throw MalformedFormatExeption ("Page::MediaBox is not array.");

  	setDoubleInArray (*mbox, 0, rc.xleft);
	setDoubleInArray (*mbox, 1, rc.xright);
	setDoubleInArray (*mbox, 2, rc.yleft);
	setDoubleInArray (*mbox, 3, rc.yright);
}

//
// Display a page
//
void
CPage::displayPage (::OutputDev& out, const DisplayParams params) const
{
	//
	// Xpdf Global variable TFUJ!!!
	// REMARK: FUCKING xpdf uses global variable globalParams that uses another global
	// variable builtinFonts which causes that globalParams can NOT be nested
	// 
	assert (NULL == globalParams);
	boost::scoped_ptr<GlobalParams> aGlobPar (new GlobalParams (NULL));
	GlobalParams* oldGlobPar = globalParams;
	globalParams = aGlobPar.get();
	globalParams->setupBaseFonts (NULL);
	
	// Are we in valid pdf
	assert (isInValidPdf (dictionary));
	assert (hasValidRef (dictionary));
	if (!isInValidPdf (dictionary) || !hasValidRef (dictionary))
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
	// Set global variable back to null
	globalParams = oldGlobPar;
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
		printDbg (debug::DBG_DBG, "No content stream found.");
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
			contentstream = shared_ptr<CContentStream> (new CContentStream (stream, stream->_makeXpdfObject()));
		
		}else if (isArray (contents))
		{
			// Save all streams from array to a vector
			CContentStream::ContentStreams streams;
			shared_ptr<CArray> array = IProperty::getSmartCObjectPtr<CArray> (contents); 
			for (size_t i = 0; i < array->getPropertyCount(); ++i)
				streams.push_back (getCStreamFromArray (array, i));
			
			// Create contentstream from array of streams
			contentstream = shared_ptr<CContentStream> 
				(new CContentStream (streams, array->_makeXpdfObject()));
			
		}else // Neither stream nor array
		{
			printDbg (debug::DBG_DBG, "Content stream type: " << contents->getType());
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
