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
#include "observer.h"

// =====================================================================================
namespace pdfobjects {
// =====================================================================================

using namespace std;
using namespace boost;
using namespace utils;

// =====================================================================================
namespace {
// =====================================================================================

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
 * <br>
 * Note that attrs structure comes out allways initialized when recursion is
 * finished.
 *
 * @param pageDict Page dictionary.
 * @param attrs Output attribute structure where correct values are put.
 *
 * @throw NotImplementedException at this moment.
 */
void 
fillInheritedPageAttr(const boost::shared_ptr<CDict> pageDict, InheritedPageAttr & attrs)
{
	int initialized=0;

	// TODO consolidate code - get rid of copy & paste
	
	// resource field
	if(!attrs.resources.get())
	{
		// attrs.resources field is not specified yet, so tries this dictionary
		if(pageDict->containsProperty("Resources"))
		{
			shared_ptr<IProperty> prop=pageDict->getProperty("Resources");
			if(isRef(prop))
			{
				attrs.resources=getDictFromRef(prop);
				initialized++;
			}
			else
				if(isDict(prop))
				{
					attrs.resources=IProperty::getSmartCObjectPtr<CDict>(prop);
					initialized++;
				}
		}
	}else
		initialized++;

	// mediabox field
	if(!attrs.mediaBox.get())
	{
		// attrs.mediaBox field is not specified yet, so tries this array
		if(pageDict->containsProperty("MediaBox"))
		{
			shared_ptr<IProperty> prop=pageDict->getProperty("MediaBox");
			if(isRef(prop))
			{
				attrs.mediaBox=getCObjectFromRef<CArray, pArray>(prop);
				initialized++;
			}else
				if(isArray(prop))
				{
					attrs.mediaBox=IProperty::getSmartCObjectPtr<CArray>(prop);
					initialized++;
				}
		}
	}else
		initialized++;

	// cropbox field
	if(!attrs.cropBox.get())
	{
		// attrs.cropBox field is not specified yet, so tries this array
		if(pageDict->containsProperty("CropBox"))
		{
			shared_ptr<IProperty> prop=pageDict->getProperty("CropBox");
			if(isRef(prop))
			{
				attrs.cropBox=getCObjectFromRef<CArray, pArray>(prop);
				initialized++;
			}else
				if(isArray(prop))
				{
					attrs.cropBox=IProperty::getSmartCObjectPtr<CArray>(prop);
					initialized++;
				}
		}
	}else
		initialized++;

	// rotate field
	if(!attrs.rotate.get())
	{
		// attrs.rotate field is not specified yet, so tries this array
		if(pageDict->containsProperty("Rotate"))
		{
			shared_ptr<IProperty> prop=pageDict->getProperty("Rotate");
			if(isRef(prop))
			{
				attrs.rotate=getCObjectFromRef<CInt, pInt>(prop);
				initialized++;
			}else
				if(isInt(prop))
				{
					attrs.rotate=IProperty::getSmartCObjectPtr<CInt>(prop);
					initialized++;
				}
		}
	}else
		initialized++;

	// all values available from this dictionary are set now
	if(initialized<4)
	{
		// not everything from InheritedPageAttr is initialized now
		// tries to initialize from parent.
		// If parent is not present (root of page tree hierarchy is reached),
		// stops recursion and initializes values with default
		if(pageDict->containsProperty("Parent"))
		{
			shared_ptr<IProperty> parentRef=pageDict->getProperty("Parent");
			if(!isRef(parentRef))
				// this should not happen - malformed page tree structure
				return;

			shared_ptr<CDict> parentDict=getDictFromRef(parentRef);
			fillInheritedPageAttr(parentDict, attrs);
		}else
		{
			// Resources is required and at least empty dictionary should be
			// specified 
			if(!attrs.resources.get())
				attrs.resources=shared_ptr<CDict>(CDictFactory::getInstance());

			// default A4 sized box
			Rectangle defaultRect(
					DisplayParams::DEFAULT_PAGE_LX, 
					DisplayParams::DEFAULT_PAGE_LY, 
					DisplayParams::DEFAULT_PAGE_RX, 
					DisplayParams::DEFAULT_PAGE_RY
					);

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
			{
				// gcc workaround
				// direct usage of static DEFAULT_ROTATE value caused linkage
				// error
				int defRot=DisplayParams::DEFAULT_ROTATE;
				attrs.rotate=shared_ptr<CInt>(CIntFactory::getInstance(defRot));
			}
		}
	}
}


// =====================================================================================
} // namespace
// =====================================================================================

//
//
//
void 
setInheritablePageAttr(boost::shared_ptr<CDict> & pageDict)
{

	InheritedPageAttr attrs;
	fillInheritedPageAttr(pageDict, attrs);

	// checks Resources
	if(!pageDict->containsProperty("Resources"))
		pageDict->addProperty("Resources", *(attrs.resources));
	
	// checks MediaBox
	if(!pageDict->containsProperty("MediaBox"))
		pageDict->addProperty("MediaBox", *(attrs.mediaBox));
	
	// checks CropBox
	if(!pageDict->containsProperty("CropBox"))
		pageDict->addProperty("CropBox", *(attrs.cropBox));
	
	// checks Rotate
	if(!pageDict->containsProperty("Rotate"))
		pageDict->addProperty("Rotate", *(attrs.rotate));
}

// =====================================================================================
namespace {
// =====================================================================================
		
	/** Helper method to extract annotations array from page dictionary.
	 * @param pageDict Page dictionary.
	 *
	 * If Annots property is reference, dereferece it and checks whether target
	 * property is array.  
	 *
	 * @throw ElementNotFoundException if given page dictionary doesn't contain
	 * Annots property.
	 * @throw ElementBadTypeException if Annots property exists but it is not an
	 * array or reference to array.
	 *
	 * @throw ElementNotFoundException if Annots array is not present in given
	 * dictionary.
	 * @return Annotation array.
	 */
	shared_ptr<CArray> getAnnotsArray(shared_ptr<CDict> pageDict)
	{

		shared_ptr<CArray> annotsArray;

		shared_ptr<IProperty> arrayProp=pageDict->getProperty("Annots");
		if(isRef(arrayProp))
			// this will throw if target is not an array
			annotsArray=getCObjectFromRef<CArray, pArray>(arrayProp);
		else 
			annotsArray=IProperty::getSmartCObjectPtr<CArray>(arrayProp);

		// just to be sure that return value is initialized
		assert(annotsArray.get());
		return annotsArray;
	}

	/** Collects all annotations from given page dictionary.
	 * @param pageDict Page dictionary.
	 * @param container Container where to place annotations.
	 *
	 * Checks Annots property from given dictionary and if it is an array (or
	 * reference to array) then continue, otherwise immediatelly returns.
	 * All members which are referencies and points to dictionaries are used to
	 * create new CAnnotation instance which is placed to given container.
	 * <br>
	 * Note that given container is cleared in any case.
	 */
	template<typename Container>
	void
	collectAnnotations(boost::shared_ptr<CDict> pageDict, Container & container)
	{

		// clears given container
		container.clear();

		try
		{
			// gets annotation array from page dictionary
			shared_ptr<CArray> annotsArray=getAnnotsArray(pageDict);

			// gets all Annots elements - these has to be referencies to
			// dictionaries
			for(size_t i=0; i<annotsArray->getPropertyCount(); i++)
			{
				// gets elements and ignores those which are not referencies
				shared_ptr<IProperty> elem=annotsArray->getProperty(i);
				if(!isRef(elem))
				{
					kernelPrintDbg(debug::DBG_WARN, "Annots["<<i<<"] is not reference. Ignoring.");
					continue;
				}

				// gets target property which has to be dictionary - if not skips
				// element
				try
				{
					shared_ptr<CDict> annotDict=getCObjectFromRef<CDict, pDict>(elem);

					// creates CAnnotation instance and inserts it to the container
					shared_ptr<CAnnotation> annot(new CAnnotation(annotDict));
					container.push_back(annot);
				}catch(ElementBadTypeException & e)
				{
					kernelPrintDbg(debug::DBG_WARN, "Annots["<<i<<"] target object is not dictionary. Ignoring.");
				}
			}
			
		}catch(CObjectException & e)
		{
			kernelPrintDbg(debug::DBG_WARN, "Unable to get Annots array - message="<<e.what());
		}
	}

//=====================================================================================
} // annonymous namespace for page helper functions
//=====================================================================================



//=====================================================================================
// CPage
//=====================================================================================

//
//
//
void CPage::AnnotsWatchDog::notify(
		boost::shared_ptr<IProperty> newValue, 
		boost::shared_ptr<const IProperty::ObserverContext> context) const throw()
{

	kernelPrintDbg(debug::DBG_DBG, "");
	
	// if not basic context type, does notnig
//	if(context->getType() != IProperty::BasicObserverContextType)
//		return;
	
	// gets basic context shared_ptr
	shared_ptr<const IProperty::BasicObserverContext> basicContext=
		dynamic_pointer_cast<const IProperty::BasicObserverContext, const IProperty::ObserverContext>(context); 

	// gets original value
	const shared_ptr<IProperty> oldValue=basicContext->getOriginalValue();

	PropertyType oldType=oldValue->getType(),
				 newType=newValue->getType();

	kernelPrintDbg(debug::DBG_DBG, "oldType="<<oldType<<" newType="<<newType);

	// one of values can be CNull, but not both. If this happens, there is
	// nothing to do
	if(oldType==pNull && newType==pNull)
	{
		kernelPrintDbg(debug::DBG_WARN, "Both newValue and oldValue are CNull");
		return;
	}

	// handle original value - one which is removed or replaced 
	// this has to be invalidated and removed from annotStorage
	if(isRef(oldValue))
	{
		// member of Annots array is changed - associated annotation has to be
		// invalidated
		try
		{
			shared_ptr<CDict> annotDict=getDictFromRef(oldValue);
			CPage::AnnotStorage::iterator i;
			for(i=page->annotStorage.begin(); i!=page->annotStorage.end(); i++)
			{
				shared_ptr<CAnnotation> annot=*i;
				if(annot->getDictionary()!=annotDict)
				{
					kernelPrintDbg(debug::DBG_DBG, "Annotation maintaining oldValue found and removed. Invalidating annotation");	
					annot->invalidate();
					page->annotStorage.erase(i);
					break;
				}
			}
			if(i==page->annotStorage.end())
				kernelPrintDbg(debug::DBG_WARN, "Removed value is not in annotStorage.")
		}catch(ElementBadTypeException & e)
		{
			kernelPrintDbg(debug::DBG_WARN, "oldValue dereferenced value is not dictionary.");
		}
	}else
		if(isArray(oldValue))
		{
			// whole array has been removed or replaced - we have to remove all
			// array members and this observer will handle each separately - this
			// is not the most effective but the simplest way how to handle
			// situation
			kernelPrintDbg(debug::DBG_DBG, "oldValue is an array.");
			shared_ptr<CArray> annotsArray=IProperty::getSmartCObjectPtr<CArray>(oldValue);
			while(annotsArray->getPropertyCount())
				// this will trigger observer
				annotsArray->delProperty(0);
		}

	kernelPrintDbg(debug::DBG_DBG, "oldValue is handled now.");

	// handle new value - one which is added or replaces an old value
	// this has to be added to annotStorage
	vector<shared_ptr<IProperty> > valuesToAdd;
	if(isRef(newValue))
		valuesToAdd.push_back(newValue);
	else
		if(isArray(newValue))
		{
			// whole array has raplaced an old one, so all elements have to be
			// handled
			kernelPrintDbg(debug::DBG_DBG, "newValue is an array.");
			shared_ptr<CArray> newValueArray=IProperty::getSmartCObjectPtr<CArray>(newValue);
			for(size_t i=0; i<newValueArray->getPropertyCount(); i++)
				valuesToAdd.push_back(newValueArray->getProperty(i));
		}

	// adds all referencies from newValueArray
	kernelPrintDbg(debug::DBG_DBG, "Inserting "<<valuesToAdd.size()<<" elements.");
	for(vector<shared_ptr<IProperty> >::iterator i=valuesToAdd.begin(); i!=valuesToAdd.end(); i++)
	{
		// considers just referencies
		if(isRef(newValue))
		{
			// registers itself (as observer) to newValue
			newValue->registerObserver(page->annotsWatchDog);
			
			// checks whether dereferenced property is dictionary. If not it 
			// means that some mass is provided and so it is ignored
			try
			{
				shared_ptr<CDict> annotDict=getDictFromRef(newValue);		

				// creates CAnnotation instance from dereferenced dictionary 
				// and adds it to annotStorage
				shared_ptr<CAnnotation> annot(new CAnnotation(annotDict));
				page->annotStorage.push_back(annot);
			}catch(ElementBadTypeException & e)
			{
				kernelPrintDbg(debug::DBG_WARN, "Dereferenced newValue is not dictionary.");
			}
		}else
		{
			kernelPrintDbg(debug::DBG_WARN, "Element is not reference. Skiping.");
		}
	}

	kernelPrintDbg(debug::DBG_INFO, "Annotation consolidation done.");
}

//
//
//
void 
CPage::ContentsWatchDog::notify (boost::shared_ptr<IProperty>, 
								boost::shared_ptr<const IProperty::ObserverContext>) const throw()
{
	kernelPrintDbg(debug::DBG_DBG, "");

	// Parse content streams (add or delete of object)
	page->parseContentStream ();

	// Indicate change
	page->_objectChanged ();
}


//
// Constructor
//
CPage::CPage (boost::shared_ptr<CDict>& pageDict) : 
		dictionary(pageDict), annotsWatchDog(new AnnotsWatchDog(this)),	contentsWatchDog (new ContentsWatchDog(this))

{
	kernelPrintDbg (debug::DBG_DBG, "");
	assert (pageDict);

// Better not throw in a constructor
//  if (!isPage (pageDict))
//		throw CObjInvalidObject ();		
	
	// Fill inheritable properties but do not dispatch the change
	// if no change on this document occurs, we do not want to change it (but we
	// do it in the setInheritablePageAttr function)
	//dictionary->lockChange();
	//setInheritablePageAttr (dictionary);
	//dictionary->unlockChange();

	// collects all annotations from this page and registers observer to Annots
	// array and all its members
	collectAnnotations(dictionary, annotStorage);
	registerAnnotsWatchDog();

	// Workaround: pages that miss contents entry
	//  -- it is not a global solution one can delete and add
	//  contents entry of a page
	if (!dictionary->containsProperty("Contents"))
	{
		CArray array;
		dictionary->addProperty ("Contents",array);
	}	
	
	// Create contents observer
	registerContentsObserver ();
}

//
//
//
void
CPage::registerAnnotsWatchDog()
{
	kernelPrintDbg(debug::DBG_DBG, "");

	try
	{
		shared_ptr<IProperty> annotsProp=dictionary->getProperty("Annots");
		if(!isArray(annotsProp))
		{
			kernelPrintDbg(debug::DBG_ERR, "Annots field is not an array.");
			return;
		}

		kernelPrintDbg(debug::DBG_DBG, "Registering annotsWatchDog observer.");
		
		// registers annotsWatchDog observer to array and all array's element
		shared_ptr<CArray> annotsArray=IProperty::getSmartCObjectPtr<CArray>(annotsProp);
		annotsArray->registerObserver(annotsWatchDog);
		for(size_t i=0; i<annotsArray->getPropertyCount(); i++)
		{
			shared_ptr<IProperty> element=annotsArray->getProperty(i);
			if(isRef(element))
			{
				element->registerObserver(annotsWatchDog);
				continue;
			}
			kernelPrintDbg(debug::DBG_WARN, "Annots array contains non reference element.")
		}
	}catch(ElementNotFoundException & e)
	{
		kernelPrintDbg(debug::DBG_INFO, "Annots property is not page dictionary member.");
	}
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
	
	InheritedPageAttr atr;
	fillInheritedPageAttr (dictionary,atr);
	Rectangle rc;
	
  	rc.xleft  =	getDoubleFromArray (atr.mediaBox, 0);
	rc.yleft  =	getDoubleFromArray (atr.mediaBox, 1);
	rc.xright = getDoubleFromArray (atr.mediaBox, 2);
	rc.yright = getDoubleFromArray (atr.mediaBox, 3);

	return rc;
}

//
//
//
int
CPage::getRotation () const
{
	try {
		return utils::getIntFromDict(dictionary,"Rotate");
	}catch (CObjectException&)
		{ return 0; }
}


//
//
//
void
CPage::setRotation (int rot)
{
	CInt crot (rot);
	dictionary->setProperty ("Rotate", crot);
}


//
//
//
void
CPage::addAnnotation(boost::shared_ptr<CAnnotation> annot)
{
	kernelPrintDbg(debug::DBG_DBG, "");
	
	// checks whether this page is valid (has pdf and valid reference.
	if(!hasValidPdf(dictionary) || !hasValidPdf(dictionary))
	{
		kernelPrintDbg(debug::DBG_ERR, "This page doesn't belong to pdf or doesn't "
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
		annotsArray=getAnnotsArray(dictionary);
	}catch(ElementBadTypeException & e)
	{
		// TODO provide also bad type information
		kernelPrintDbg(debug::DBG_ERR, "Page's Annots field is malformed. Array property expected.");
		throw;
	}
	catch(ElementNotFoundException & e)
	{
		kernelPrintDbg(debug::DBG_INFO, "Page's Annots field missing. Creating one.");
		
		// Annots array doesn't exist - so creates one and registers observer to
		// it.
		// annotsArray must be set from getProperty method because addProperty
		// creates deep copy of given
		scoped_ptr<IProperty> tmpArray(CArrayFactory::getInstance());
		dictionary->addProperty("Annots", *tmpArray);
		annotsArray=IProperty::getSmartCObjectPtr<CArray>(
				dictionary->getProperty("Annots")
				);
		annotsArray->registerObserver(annotsWatchDog);	
	}

	kernelPrintDbg(debug::DBG_DBG, "Creating new indirect dictionary for annotation.");
	
	// addes annotation dictionary to the pdf - this will add deep copy if given
	// dictionary and also solves problems with annotation from different pdf
	IndiRef annotRef=pdf->addIndirectProperty(annot->getDictionary());
	
	// gets added annotation dictionary
	shared_ptr<CDict> annotDict=IProperty::getSmartCObjectPtr<CDict>(
			pdf->getIndirectProperty(annotRef)
			);

	kernelPrintDbg(debug::DBG_DBG, "Setting annotation dictionary field P="<<dictionary->getIndiRef());
	// updates P field with reference to this page
	// This is not explictly required by specification for all annotation types,
	// but is not an error to supply this information
	shared_ptr<CRef> pageRef(CRefFactory::getInstance(dictionary->getIndiRef()));
	utils::checkAndReplace(annotDict, "P", *pageRef);

	kernelPrintDbg(debug::DBG_INFO, "Adding reference "<<annotRef<<" to annotation dictionary to Annots array");
	// annotation dictionary is prepared and so its reference can be stored	
	// to Annots array from page dictionary - this will start observer which
	// will update annotStorage
	scoped_ptr<CRef> annotCRef(CRefFactory::getInstance(annotRef));
	annotsArray->addProperty(*annotCRef);
}

bool
CPage::delAnnotation(boost::shared_ptr<CAnnotation> annot)
{
	kernelPrintDbg(debug::DBG_DBG, "");

	// searches annotation in annotStorage - which is synchronized with current
	// state of Annots array
	size_t pos=0;
	for(AnnotStorage::iterator i=annotStorage.begin(); i!=annotStorage.end(); i++,pos++)
	{
		shared_ptr<CAnnotation> element=*i;	
		if(annot!=element)
			continue;
		
		// annotation found, removes dictionary reference from Annots array
		IndiRef annotRef=element->getDictionary()->getIndiRef();
		kernelPrintDbg(debug::DBG_DBG, "Annotation found. Indiref="<<annotRef);
		try
		{
			shared_ptr<CArray> annotArray=getAnnotsArray(dictionary);
			// deleting of this reference triggers annotsWatchDog observer which
			// will synchronize annotStorage with current state
			annotArray->delProperty(pos);
			kernelPrintDbg(debug::DBG_INFO, "Annotation referece "<<annotRef<<" removed from Annots array. "
					<<"Invalidating annotation instance.");
			annot->invalidate();
			return true;
		}catch(CObjectException & e)
		{
			kernelPrintDbg(debug::DBG_CRIT, "Unexpected Annots array missing.");
			return false;
		}
	}
	kernelPrintDbg(debug::DBG_ERR, "Given annotation couldn't have been found.");
	return false;
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

	static const string strMBox ("MediaBox");
	
	try {
		
		// Get the array representing media box
		shared_ptr<IProperty> mbox = dictionary->getProperty (strMBox);
		assert (isArray(mbox));
		if (!isArray(mbox))
			throw MalformedFormatExeption ("Page::MediaBox is not array.");

	  	setDoubleInArray (*mbox, 0, rc.xleft);
		setDoubleInArray (*mbox, 1, rc.yleft);
		setDoubleInArray (*mbox, 2, rc.xright);
		setDoubleInArray (*mbox, 3, rc.yright);

		return;

	}catch (CObjectException&)
	{
		if (dictionary->containsProperty (strMBox)) 
			dictionary->delProperty (strMBox);

		CArray array;
		
		CReal item (rc.xleft);
		array.addProperty (item);
		
		item.setValue (rc.yleft);
		array.addProperty (item);
		
		item.setValue (rc.xright);
		array.addProperty (item);
		
		item.setValue (rc.yright);
		array.addProperty (item);
		
		dictionary->addProperty (strMBox,array);
	}
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
	int rotation = lastParams.rotate - getRotation ();
	page.display   (&out, lastParams.hDpi, lastParams.vDpi, 
					rotation, lastParams.useMediaBox, 
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
// Contents Observer interface
//

//
//
//
void
CPage::registerContentsObserver () const
{
	if (contentsWatchDog)
	{ // Register contents observer
		
		if (dictionary->containsProperty("Contents"))
			dictionary->getProperty("Contents")->registerObserver (contentsWatchDog);
		
	}else
	{
		assert (!"Observer is not initialized.");
		throw CObjInvalidOperation ();
	}
}

//
//
//
void
CPage::unregisterContentsObserver () const
{
	if (contentsWatchDog)
	{ // Unregister contents observer
		
		if (dictionary->containsProperty("Contents"))
			dictionary->getProperty("Contents")->unregisterObserver (contentsWatchDog);
		
	}else
	{
		assert (!"Observer is not initialized.");
		throw CObjInvalidOperation ();
	}
}



//
//
//
void CPage::reparseContentStream ( )
{
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
	
	name->setValue ("Type1");
	font->addProperty (string ("SubType"), *name);
	
	name->setValue (fontname);
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
// Transform matrix
// 

//
//
//
void
CPage::setTransformMatrix (double tm[6])
{
	if (contentstreams.empty())
	{ // Try parsing streams
		parseContentStream ();
		if (contentstreams.empty())
			return;
	}

	assert (!contentstreams.empty());
	shared_ptr<CContentStream> str = contentstreams.front();
	assert(str);

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
	shared_ptr<PdfOperator> cmop (new SimpleGenericOperator ("cm", 6, operands));

	// Insert at the beginning
	str->frontInsertOperator (cmop);
}

//
//
//
void
CPage::_objectChanged ()
{
	// Do not notify anything if we are not in a valid pdf
	if (!hasValidPdf (dictionary))
		return;
	assert (hasValidRef (dictionary));

	// Notify observers
	boost::shared_ptr<CPage> current (this, EmptyDeallocator<CPage> ());
	this->notifyObservers (current, shared_ptr<const ObserverContext> (new BasicObserverContext (current)));
}


// =====================================================================================
// Helper functions
// =====================================================================================


//
//
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
