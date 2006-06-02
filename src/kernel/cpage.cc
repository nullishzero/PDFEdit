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
#include "factories.h"

// =====================================================================================
namespace pdfobjects {
// =====================================================================================

using namespace std;
using namespace boost;
using namespace utils;

//
//
//
namespace {
/** 
 * Page attributes structure of dictionary properties which can be inherited from a parent 
 * in the page tree.
 *
 * If an inheritable property is not present in a page it is defined in one of
 * its parents in the page tree.
 */
struct InheritedPageAttr
{
	boost::shared_ptr<CDict> resources;
	boost::shared_ptr<CArray> mediaBox;
	boost::shared_ptr<CArray> cropBox;
	boost::shared_ptr<CInt> rotate;
};

/** 
 * Fills InheritedPageAttr structure for a given page dictionary.
 *
 * Recursive function which checks given pageDict whether it contains
 * uninitialized (NULL values) from a given attribute structure. If true, sets
 * the value from the dictionary. If at least one property is still not 
 * initialized, repeats the process for the parent dictionary (dereferenced 
 * "Parent" property). End if the "Parent" property is not present (in root 
 * of a page tree).
 * <br>
 * Use default value when a property was not initialized.
 *
 * @param pageDict Page dictionary.
 * @param attrs Output attribute structure where correct values are put.
 *
 * @throw NotImplementedException at this moment.
 */
void fillInheritedPageAttr(const boost::shared_ptr<CDict> pageDict, InheritedPageAttr & attrs)
{
	int initialized=0;
	
	// resource field
	shared_ptr<CDict> resources=attrs.resources;
	if(!resources.get())
	{
		// resources field is not specified yet, so tries this dictionary
		try
		{
			shared_ptr<IProperty> prop=pageDict->getProperty("Resources");
			if(isRef(prop))
			{
				resources=getDictFromRef(prop);
				initialized++;
			}
			else
				if(isDict(prop))
				{
					resources=IProperty::getSmartCObjectPtr<CDict>(prop);
					initialized++;
				}
		}catch(CObjectException & e)
		{
			// not found
		}
	}else
		initialized++;

	// mediabox field
	shared_ptr<CArray> mediaBox=attrs.mediaBox;
	if(!mediaBox.get())
	{
		// mediaBox field is not specified yet, so tries this array
		try
		{
			shared_ptr<IProperty> prop=pageDict->getProperty("MediaBox");
			if(isRef(prop))
			{
				mediaBox=getCObjectFromRef<CArray, pArray>(prop);
				initialized++;
			}else
				if(isArray(prop))
				{
					mediaBox=IProperty::getSmartCObjectPtr<CArray>(prop);
					initialized++;
				}
		}catch(CObjectException & e)
		{
			// not found or bad type
		}
	}else
		initialized++;

	// cropbox field
	shared_ptr<CArray> cropBox=attrs.cropBox;
	if(!cropBox.get())
	{
		// cropBox field is not specified yet, so tries this array
		try
		{
			shared_ptr<IProperty> prop=pageDict->getProperty("CropBox");
			if(isRef(prop))
			{
				cropBox=getCObjectFromRef<CArray, pArray>(prop);
				initialized++;
			}else
				if(isArray(prop))
				{
					cropBox=IProperty::getSmartCObjectPtr<CArray>(prop);
					initialized++;
				}
		}catch(CObjectException & e)
		{
			// not found or bad type
		}
	}else
		initialized++;

	// rotate field
	shared_ptr<CInt> rotate=attrs.rotate;
	if(!rotate.get())
	{
		// rotate field is not specified yet, so tries this array
		try
		{
			shared_ptr<IProperty> prop=pageDict->getProperty("Rotate");
			if(isRef(prop))
			{
				rotate=getCObjectFromRef<CInt, pInt>(prop);
				initialized++;
			}else
				if(isInt(prop))
				{
					rotate=IProperty::getSmartCObjectPtr<CInt>(prop);
					initialized++;
				}
		}catch(CObjectException & e)
		{
			// not found or bad type
		}
	}else
		initialized++;

	// all values available from this dictionary are set now
	if(initialized<4)
	{
		// not everything from InheritedPageAttr is initialized now
		// tries to initialize from parent.
		// If parent is not present, uses dafault value
		try
		{
			shared_ptr<IProperty> parentRef=pageDict->getProperty("Parent");
			if(!isRef(parentRef))
				// this should not happen - malformed page tree structure
				return;

			shared_ptr<CDict> parentDict=getDictFromRef(parentRef);
				
		}catch(ElementNotFoundException & e)
		{
			// parent not found - uses default values
			
			// Resources is required and at least empty dictionary should be
			// specified 
			if(!attrs.resources.get())
				attrs.resources=shared_ptr<CDict>(CDictFactory::getInstance());

			// default A4 sized box
			// FIXME change to constants
			Rectangle defaultRect(0, 0, 612, 792);
			//Rectangle defaultRect(DEFAULT_PAGE_LX, DEFAULT_PAGE_LY, DEFAULT_PAGE_RX, DEFAULT_PAGE_RY);

			// MediaBox is required and specification doesn't say anything about
			// default value - we are using standard A4 format
			if(!attrs.mediaBox.get())
				attrs.mediaBox=IProperty::getSmartCObjectPtr<CArray>(getIPropertyFromRectangle(defaultRect));

			// CropBox is optional and specification doesn't say anything about
			// default value - we are using standard A4 format
			if(!attrs.cropBox.get())
				attrs.cropBox=IProperty::getSmartCObjectPtr<CArray>(getIPropertyFromRectangle(defaultRect));
			
			// Rotate is optional and specification defines default value to 0
			if(!attrs.rotate.get())
				// FIXME
				attrs.rotate=shared_ptr<CInt>(CIntFactory::getInstance(0));
				//attrs.rotate=shared_ptr<CInt>(CIntFactory::getInstance(DEFAULT_ROTATE));
		}
	}
}

} // anonnymous namespace for inherited attributes

//=====================================================================================
// CPage
//=====================================================================================

//
// Constructor
//
CPage::CPage (boost::shared_ptr<CDict>& pageDict) : dictionary(pageDict)
{
	kernelPrintDbg (debug::DBG_DBG, "");
	assert (pageDict);

// Better not throw in a constructor
//  if (!isPage (pageDict))
//		throw CObjInvalidObject ();		
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
//
//
void
CPage::addAnnotation(boost::shared_ptr<CAnnotation> annot)
{
using namespace boost;
using namespace debug;

	kernelPrintDbg(DBG_DBG, "");
	
	// checks whether this page is valid (has pdf and valid reference.
	if(!hasValidPdf(dictionary) || !hasValidPdf(dictionary))
	{
		kernelPrintDbg(DBG_ERR, "This page doesn't belong to pdf or doesn't "
				<< "have correct indiRef. It is not suitable for annotation.");
		throw CObjInvalidObject();
	}

	// gets pdf of this annotation - it is valid
	CPdf * pdf=dictionary->getPdf();
		
	// gets Annots array from page dictionary. If no found, creates it. If bad
	// typed, throws an exception
	shared_ptr<CArray> annotsArray;
	try
	{
		shared_ptr<IProperty> arrayProp=dictionary->getProperty("Annots");
		if(isRef(arrayProp))
			// this will throw if target is not an array
			annotsArray=getCObjectFromRef<CArray, pArray>(arrayProp);
	}catch(ElementBadTypeException & e)
	{
		// TODO provide also bad type information
		kernelPrintDbg(DBG_ERR, "Page's Annots field is malformed. Array property expected.");
		throw;
	}
	catch(ElementNotFoundException & e)
	{
		kernelPrintDbg(DBG_INFO, "Page's Annots field missing. Creating one.");
		
		// Annots array doesn't exist - so creates one and registers observer to
		// it.
		// annotsArray must be set from getProperty method because addProperty
		// creates deep copy of given
		scoped_ptr<IProperty> tmpArray(CArrayFactory::getInstance());
		dictionary->addProperty("Annots", *tmpArray);
		annotsArray=IProperty::getSmartCObjectPtr<CArray>(
				dictionary->getProperty("Annots")
				);
		
		// TODO registers observer
	}

	kernelPrintDbg(DBG_DBG, "Creating new indirect dictionary for annotation.");
	
	// addes annotation dictionary to the pdf - this will add deep copy if given
	// dictionary and also solves problems with annotation from different pdf
	IndiRef annotRef=pdf->addIndirectProperty(annot->getDictionary());
	
	// gets added annotation dictionary
	shared_ptr<CDict> annotDict=IProperty::getSmartCObjectPtr<CDict>(
			pdf->getIndirectProperty(annotRef)
			);

	kernelPrintDbg(DBG_DBG, "Setting annotation dictionary field P="<<dictionary->getIndiRef());
	// updates P field with reference to this page
	// This is not explictly required by specification for all annotation types,
	// but is not an error to supply this information
	shared_ptr<CRef> pageRef(CRefFactory::getInstance(dictionary->getIndiRef()));
	utils::checkAndReplace(annotDict, "P", *pageRef);

	kernelPrintDbg(DBG_INFO, "Adding reference "<<annotRef<<" to annotation dictionary to Annots array");
	// annotation dictionary is prepared and so its reference can be stored	
	// to Annots array from page dictionary - this will start observer which
	// will update annotStorage
	scoped_ptr<CRef> annotCRef(CRefFactory::getInstance(annotRef));
	annotsArray->addProperty(*annotCRef);
}
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
		(*it)->reparse (true, state, res);
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


//
// Font interface
//

//
//
//
void
CPage::addSystemType1Font (const std::string& fontname)
{
	// << 
	//    /Type /Font
	//    /SubType /Type1
	//    /BaseFont / ...
	// >>
	boost::shared_ptr<CDict> font (new CDict ());
	
	boost::shared_ptr<CName> name (new CName ("Font"));
	font->addProperty (string("Type"), *name);
	
	name->writeValue ("Type1");
	font->addProperty (string ("SubType"), *name);
	
	name->writeValue (fontname);
	font->addProperty (string ("BaseFont"), *name);
	
	// Resources is an inheritable property, must be present
	boost::shared_ptr<CDict> res = utils::getCDictFromDict (dictionary, "Resources");
	boost::shared_ptr<CDict> fonts;
	
	try {
		
		fonts = utils::getCDictFromDict (res, "Font");
	
	}catch (ElementNotFoundException&)
	{
		boost::shared_ptr<CDict> fontdict (new CDict ());
		res->addProperty ("Font", *fontdict);
		
		fonts = utils::getCDictFromDict (res, "Font");
	}

	typedef vector<pair<string,string> > Fonts;
	Fonts fs;
	getFontIdsAndNames (fs);

	size_t len = 0;
	bool our = false;
	static const string ourfontname ("PdfEditor");
	for (Fonts::iterator it = fs.begin(); it != fs.end(); ++it)
	{
		// Compare basenames and look for longest string and for PdfEdit string
		if (ourfontname == (*it).first)
			our = true;
		len = std::max ((*it).first.length(), len);
	}

	if (!our)
		fonts->addProperty (ourfontname, *font);
	else
	{/**\todo make sane */
		len -= ourfontname.length();
		len++;
		string tmpname = ourfontname;
		for (size_t i = 0; i < len; i++)
			tmpname.push_back ('r');
		fonts->addProperty (tmpname, *font);
	}
		
}


//
// Helper functions
//
bool 
isPage (boost::shared_ptr<IProperty> ip)
{
	assert (ip);
	assert (isDict (ip));

	if (!isDict(ip))
		throw CObjInvalidObject ();

	boost::shared_ptr<CDict> dict = IProperty::getSmartCObjectPtr<CDict> (ip);

	if ("Page" != utils::getStringFromDict (dict, "Type"))
		throw CObjInvalidObject ();

	return true;
}



// =====================================================================================
} // namespace pdfobjects
// =====================================================================================
