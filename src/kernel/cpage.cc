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
namespace pdfobjects {
// =====================================================================================

using namespace std;
using namespace boost;
using namespace utils;


//=====================================================================================
// Display parameters (loose xpdf paramters put into a simple structure)
//=====================================================================================

namespace {
	//
	// Default values
	//
	static const double DEFAULT_HDPI 	= 72;		/**< Default horizontal dpi. */
	static const double DEFAULT_VDPI 	= 72;		/**< Default vertical dpi. */
	static const int DEFAULT_ROTATE 	= 0;		/**< No rotatation. */

	static const double DEFAULT_PAGE_LX = 0;
	static const double DEFAULT_PAGE_LY = 0;
	static const double DEFAULT_PAGE_RX = 612;		/**< Default A4 width on a device with 72 horizontal dpi. */
	static const double DEFAULT_PAGE_RY = 792;		/**< Default A4 height on a device with 72 vertical dpi. */
}

//
// Constructor
//
DisplayParams::DisplayParams () : 
	hDpi (DEFAULT_HDPI), vDpi (DEFAULT_VDPI),
	pageRect (Rectangle (DEFAULT_PAGE_LX, DEFAULT_PAGE_LY, DEFAULT_PAGE_RX, DEFAULT_PAGE_RY)),
	rotate (DEFAULT_ROTATE), useMediaBox (gTrue), crop (gFalse), upsideDown (gTrue) 
{}


//=====================================================================================
// Text searching parameters
//=====================================================================================

namespace {
	//
	// Default values
	// 
	static const GBool DEFAULT_START_AT_TOP 	= gTrue;	/**< Default start at top. */
	
	/** Default start positino when start at top is false. */
	static const double DEFAULT_X_START = 0;				
	static const double DEFAULT_Y_START = 0;
	static const double DEFAULT_X_END = 0;
	static const double DEFAULT_Y_END = 0;
}

//
// Constructor
//
TextSearchParams::TextSearchParams () : 
	startAtTop (DEFAULT_START_AT_TOP),
	xStart (DEFAULT_X_START), yStart (DEFAULT_Y_START), xEnd (DEFAULT_X_END), yEnd (DEFAULT_Y_END)
{}



//=====================================================================================
// CPage
//=====================================================================================

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
CPage::displayPage (::OutputDev& out) const
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
	boost::shared_ptr<Object> xpdfPage (dictionary->_makeXpdfObject(), xpdf::object_deleter());
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
	page.display   (&out, lastParams.hDpi, lastParams.vDpi, 
					lastParams.rotate, lastParams.useMediaBox, 
					lastParams.crop, NULL, xpdfCatalog.get());

	//
	// Cleanup
	// 

	// Clean xpdf mess
	xpdf::closeXpdfMess ();
}

//
//
//
void 
CPage::displayPage (::OutputDev& out, const DisplayParams params) 
{

	
	// Reparse content streams if parameters changed
	if (!(lastParams == params))
	{
		lastParams = params;
		
		// Use mediabox
		if (lastParams.useMediaBox)
		{
			try {
				
				lastParams.pageRect = getMediabox ();
				
			}catch (ElementNotFoundException&)
			{
				kernelPrintDbg (debug::DBG_CRIT, "Mediabox not found.");
				lastParams.pageRect = DisplayParams().pageRect;
			}
		}
		
		reparseContentStream ();
	}

	// display page
	displayPage (out);
}


//
// Helper methods
//

//
//
//
void 
CPage::createXpdfDisplayParams (boost::shared_ptr<GfxResources>& res, boost::shared_ptr<GfxState>& state)
{
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
	res = boost::shared_ptr<GfxResources> (new GfxResources(xref, obj->getDict(), NULL));

	//
	// Init Gfx state
	//
	
	// Create Media (Bounding) box
	boost::shared_ptr<PDFRectangle> rc (new PDFRectangle (lastParams.pageRect.xleft,  lastParams.pageRect.yleft,
														  lastParams.pageRect.xright, lastParams.pageRect.yright));
	state = boost::shared_ptr<GfxState> (new GfxState (lastParams.hDpi, lastParams.vDpi, 
														rc.get(), lastParams.rotate, lastParams.upsideDown));

	// Close the mess
	xpdf::closeXpdfMess ();
}

//
//
//
bool CPage::parseContentStream ( )
{
	// Clear content streams
	contentstreams.clear();
	
	assert (hasValidRef(dictionary));
	assert (hasValidPdf (dictionary));
	if (!hasValidPdf(dictionary) || !hasValidRef(dictionary))
		throw CObjInvalidObject ();
	

	//
	// Create state and resources
	//
	boost::shared_ptr<GfxResources> res;
	boost::shared_ptr<GfxState> state;
	createXpdfDisplayParams (res, state);
	
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
		CContentStream::CStreams streams;
		
		//
		// Contents can be either stream or an array of streams
		//
		if (isStream (contents))	
		{
			shared_ptr<CStream> stream = IProperty::getSmartCObjectPtr<CStream> (contents); 
			streams.push_back (stream);
		
		}else if (isArray (contents))
		{
			// We can be sure that streams are indirect objects (pdf spec)
			shared_ptr<CArray> array = IProperty::getSmartCObjectPtr<CArray> (contents); 
			for (size_t i = 0; i < array->getPropertyCount(); ++i)
				streams.push_back (getCStreamFromArray(array,i));
			
		}else // Neither stream nor array
		{
			kernelPrintDbg (debug::DBG_CRIT, "Content stream type: " << contents->getType());
			throw ElementBadTypeException ("Bad content stream type.");
		}

		//
		// Create content streams, each cycle will take one/more content streams from streams variable
		//
		assert (contentstreams.empty());
		assert (!streams.empty());
		while (!streams.empty())
			contentstreams.push_back (shared_ptr<CContentStream> (new CContentStream(streams,state,res)));
	
	}else
		throw ElementBadTypeException ("Bad pointer to content stream.");

	// Everything went ok
	return true;
}


//
//
//
void CPage::reparseContentStream ( )
{
	// Clear content streams
	contentstreams.clear();
	
	assert (hasValidRef(dictionary));
	assert (hasValidPdf (dictionary));
	if (!hasValidPdf(dictionary) || !hasValidRef(dictionary))
		throw CObjInvalidObject ();
	

	//
	// Create state and resources
	//
	boost::shared_ptr<GfxResources> res;
	boost::shared_ptr<GfxState> state;
	createXpdfDisplayParams (res, state);
	
	// Set only bboxes
	for (ContentStreams::iterator it = contentstreams.begin();
			it != contentstreams.end(); ++it)
		(*it)->reparse (state, res, true);
}


//
// Text search/find
//

// 
// Find all occcurences of a text on a page
//
template<typename RectangleContainer>
size_t CPage::findText (std::string text, 
					  RectangleContainer& recs, 
					  const TextSearchParams&) const
{
	// Create text output device
	boost::scoped_ptr<TextOutputDev> textDev (new ::TextOutputDev (NULL, gFalse, gFalse, gFalse));
	assert (textDev->isOk());
	if (!textDev->isOk())
		throw CObjInvalidOperation ();

	// Get the text
	displayPage (*textDev);	

	GBool startAtTop, stopAtBottom, startAtLast, stopAtLast, caseSensitive, backward;
	startAtTop = stopAtBottom = startAtLast = stopAtLast = gTrue;
	caseSensitive = backward = gFalse;
	
	double xMin = 0, yMin = 0, xMax = 0, yMax = 0;

	// Convert text to unicode (slightly modified from from PDFCore.cc)
	int length = text.length();
	::Unicode* utext = static_cast<Unicode*> (new Unicode [length]);
	for (int i = 0; i < length; ++i)
	    utext[i] = static_cast<Unicode> (text[i] & 0xff);
	
	if (textDev->findText  (utext, length, 
							startAtTop, stopAtBottom, 
							startAtLast,stopAtLast, 
							caseSensitive, backward,
							&xMin, &yMin, &xMax, &yMax))
	{
		startAtTop = gFalse;
		
		recs.push_back (Rectangle (xMin, yMin, xMax, yMax));
		// Get all text objects
		while (textDev->findText (utext, length,
								  startAtTop, stopAtBottom, 
								  startAtLast, stopAtLast, 
								  caseSensitive, backward,
								  &xMin, &yMin, &xMax, &yMax))
		{
			recs.push_back (Rectangle (xMin, yMin, xMax, yMax));
		}
	}

	//
	// Find out the words...
	//

	delete[] utext;	
	return recs.size();
}

// Explicit instantiation
template size_t CPage::findText<std::vector<Rectangle> >
	(std::string text, 
	 std::vector<Rectangle>& recs, 
	 const TextSearchParams& params) const;


// =====================================================================================
} /* namespace pdfobjects */
// =====================================================================================
