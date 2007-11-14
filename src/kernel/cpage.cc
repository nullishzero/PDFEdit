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

// static
#include "static.h"

// Page
#include "cpage.h"
// CContenteStream
#include "ccontentstream.h"
//Cpdf
#include "cpdf.h"
// Helper functions
#include "cobject.h"
#include "factories.h"
#include "observer.h"

// =====================================================================================
namespace pdfobjects {
// =====================================================================================

using namespace std;
using namespace boost;
using namespace utils;
using namespace observer;

static const char * PDFEDIT_FONTID = "PDFEDIT_F";

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
				attrs.resources=getCObjectFromRef<CDict>(prop);
				++initialized;
			}
			else
				if(isDict(prop))
				{
					attrs.resources=IProperty::getSmartCObjectPtr<CDict>(prop);
					++initialized;
				}
		}
	}else
		++initialized;

	// mediabox field
	if(!attrs.mediaBox.get())
	{
		// attrs.mediaBox field is not specified yet, so tries this array
		if(pageDict->containsProperty("MediaBox"))
		{
			shared_ptr<IProperty> prop=pageDict->getProperty("MediaBox");
			if(isRef(prop))
			{
				attrs.mediaBox=getCObjectFromRef<CArray>(prop);
				++initialized;
			}else
				if(isArray(prop))
				{
					attrs.mediaBox=IProperty::getSmartCObjectPtr<CArray>(prop);
					++initialized;
				}
		}
	}else
		++initialized;

	// cropbox field
	if(!attrs.cropBox.get())
	{
		// attrs.cropBox field is not specified yet, so tries this array
		if(pageDict->containsProperty("CropBox"))
		{
			shared_ptr<IProperty> prop=pageDict->getProperty("CropBox");
			if(isRef(prop))
			{
				attrs.cropBox=getCObjectFromRef<CArray>(prop);
				++initialized;
			}else
				if(isArray(prop))
				{
					attrs.cropBox=IProperty::getSmartCObjectPtr<CArray>(prop);
					++initialized;
				}
		}
	}else
		++initialized;

	// rotate field
	if(!attrs.rotate.get())
	{
		// attrs.rotate field is not specified yet, so tries this array
		if(pageDict->containsProperty("Rotate"))
		{
			shared_ptr<IProperty> prop=pageDict->getProperty("Rotate");
			if(isRef(prop))
			{
				attrs.rotate=getCObjectFromRef<CInt>(prop);
				++initialized;
			}else
				if(isInt(prop))
				{
					attrs.rotate=IProperty::getSmartCObjectPtr<CInt>(prop);
					++initialized;
				}
		}
	}else
		++initialized;

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

			shared_ptr<CDict> parentDict=getCObjectFromRef<CDict>(parentRef);
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
			annotsArray=getCObjectFromRef<CArray>(arrayProp);
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
			for(size_t i=0; i<annotsArray->getPropertyCount(); ++i)
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
					shared_ptr<CDict> annotDict=getCObjectFromRef<CDict>(elem);

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

typedef std::vector<boost::shared_ptr<IProperty> > ChildrenStorage;


//=====================================================================================
// CPage
//=====================================================================================
void CPage::unregisterAnnotsObservers(boost::shared_ptr<IProperty> & annots)
{
using namespace boost;
using namespace debug;
using namespace utils;

	kernelPrintDbg(DBG_DBG, "");

	shared_ptr<CArray> annotsArray;
	if(isRef(annots))
	{
		UNREGISTER_SHAREDPTR_OBSERVER(annots, annotsPropWatchDog);
		try
		{
			annotsArray=getCObjectFromRef<CArray>(annots);
		}catch(CObjectException & e)
		{
			IndiRef ref=getValueFromSimple<CRef>(annots);
			kernelPrintDbg(DBG_WARN, ref<<" doesn't point to array.");
		}
	}else
		annotsArray=IProperty::getSmartCObjectPtr<CArray>(annots);

	// if no array is present, skips further steps
	if(!annotsArray.get())
	{
		kernelPrintDbg(DBG_WARN, "Given annots is not or doesn't refer to array.");
		return;
	}

	ChildrenStorage children;
	UNREGISTER_SHAREDPTR_OBSERVER(annotsArray, annotsArrayWatchDog);
	annotsArray->_getAllChildObjects(children);
	for(ChildrenStorage::iterator i=children.begin(); i!=children.end(); ++i)
	{
		shared_ptr<IProperty> child=*i;
		if(isRef(child))
			UNREGISTER_SHAREDPTR_OBSERVER(child, annotsArrayWatchDog);
	}
}

void CPage::registerAnnotsObservers(shared_ptr<IProperty> & annots)
{
using namespace boost;
using namespace debug;

	kernelPrintDbg(DBG_DBG, "");
	shared_ptr<CArray> annotsArray;
	if(isRef(annots))
	{
		REGISTER_SHAREDPTR_OBSERVER(annots, annotsPropWatchDog);
		try
		{
			annotsArray=getCObjectFromRef<CArray>(annots);
		}catch(CObjectException & e)
		{
			IndiRef ref=getValueFromSimple<CRef>(annots);
			kernelPrintDbg(DBG_WARN, ref<<" doesn't point to array.");
		}
	}else
		annotsArray=IProperty::getSmartCObjectPtr<CArray>(annots);

	// if no array is present, skips further steps
	if(!annotsArray.get())
	{
		kernelPrintDbg(DBG_WARN, "Given annots is not or doesn't refer to array.");
		return;
	}

	ChildrenStorage children;
	REGISTER_SHAREDPTR_OBSERVER(annotsArray, annotsArrayWatchDog);
	annotsArray->_getAllChildObjects(children);
	for(ChildrenStorage::iterator i=children.begin(); i!=children.end(); ++i)
	{
		shared_ptr<IProperty> child=*i;
		if(isRef(child))
			REGISTER_SHAREDPTR_OBSERVER(child, annotsArrayWatchDog);
	}

}

void CPage::consolidateAnnotsStorage(boost::shared_ptr<IProperty> & oldValue, boost::shared_ptr<IProperty> & newValue)
{
using namespace boost;
using namespace debug;

	// handle original value - one which is removed or replaced 
	// this has to be invalidated and removed from annotStorage
	// this is skipped if oldValue is not reference
	if(isRef(oldValue))
	{
		try
		{
			shared_ptr<CDict> annotDict=getCObjectFromRef<CDict>(oldValue);
			CPage::AnnotStorage::iterator i;
			for(i=annotStorage.begin(); i!=annotStorage.end(); ++i)
			{
				shared_ptr<CAnnotation> annot=*i;
				if(annot->getDictionary()==annotDict)
				{
					kernelPrintDbg(debug::DBG_DBG, "Annotation maintaining oldValue found and removed. Invalidating annotation");	
					annot->invalidate();
					annotStorage.erase(i);
					break;
				}
			}
			if(i==annotStorage.end())
				kernelPrintDbg(debug::DBG_WARN, "Removed value is not in annotStorage.")
		}catch(ElementBadTypeException & e)
		{
			kernelPrintDbg(debug::DBG_WARN, "oldValue dereferenced value is not dictionary.");
		}
	}

	kernelPrintDbg(debug::DBG_DBG, "oldValue is handled now.");

	// handle new value - one which is added or replaces an old value
	// this has to be added to annotStorage

	// checks whether dereferenced property is dictionary. If not it 
	// means that some mass is provided and so it is ignored
	// this is skipped if newValue is not reference
	if(isRef(newValue))
	{
		try
		{
			shared_ptr<CDict> annotDict=getCObjectFromRef<CDict>(newValue);		

			// creates CAnnotation instance from dereferenced dictionary 
			// and adds it to annotStorage
			shared_ptr<CAnnotation> annot(new CAnnotation(annotDict));
			annotStorage.push_back(annot);
		}catch(ElementBadTypeException & e)
		{
			kernelPrintDbg(debug::DBG_WARN, "Dereferenced newValue is not dictionary.");
		}
	}
}

//
//
//
void 
CPage::AnnotsPropWatchDog::notify(
		boost::shared_ptr<IProperty> newValue, 
		boost::shared_ptr<const IProperty::ObserverContext> context) const throw()
{
using namespace debug;
using namespace observer;

	kernelPrintDbg(debug::DBG_DBG, "");
	if(!context.get())
	{
		kernelPrintDbg(DBG_DBG, "No context information.");
		return;
	}
	
	kernelPrintDbg(DBG_DBG, "context type="<<context->getType());
	shared_ptr<IProperty> oldValue;
	switch(context->getType())
	{
		case BasicChangeContextType:
			{
				// This means that Annots property is reference and it has
				// changed its reference value
				shared_ptr<const observer::BasicChangeContext<IProperty> > basicContext=
					dynamic_pointer_cast<const observer::BasicChangeContext<IProperty>, const observer::IChangeContext<IProperty> >(context); 
				oldValue=basicContext->getOriginalValue();
				assert(isRef(newValue));
				assert(isRef(oldValue));
			}
			break;
		case ComplexChangeContextType:
			{
				// page dictionary has changed
				// checks identificator of changed property and if it is not
				// Annots, immediately returns
				shared_ptr<const CDict::CDictComplexObserverContext > basicContext=
					dynamic_pointer_cast<const CDict::CDictComplexObserverContext, 
					const IChangeContext<IProperty> >(context); 
				if(!basicContext)
				{
					kernelPrintDbg(DBG_WARN, "Bad property identificator type.");
					return;
				}
				if(basicContext->getValueId()!="Annots")
					return;

				oldValue=basicContext->getOriginalValue();

				// reference oldValue needs unregistration of this observer
				if(isRef(oldValue))
					UNREGISTER_SHAREDPTR_OBSERVER(oldValue, page->annotsArrayWatchDog);

				// if new value is reference, register this observer to it
				if(isRef(newValue))
					REGISTER_SHAREDPTR_OBSERVER(newValue, page->annotsPropWatchDog);
			}
			break;
		default:
			kernelPrintDbg(DBG_WARN, "Unsupported context type");
	}

	// gets original annots array and unregisters all observers
	// doesn't unregister observer from Annots property, because it is done only
	// in case of complex context
	shared_ptr<IProperty> oldArray;
	if(isRef(oldValue))
	{
		try
		{
			oldArray=getCObjectFromRef<CArray>(oldValue);
		}catch(CObjectException & e)
		{
			IndiRef ref=getValueFromSimple<CRef>(oldValue);
			kernelPrintDbg(DBG_WARN, "Target of Annots "<<ref<<" is not an array");
		}
	}else
		if(isArray(oldValue))
			oldArray=IProperty::getSmartCObjectPtr<CArray>(oldValue);
	if(oldArray.get())
		page->unregisterAnnotsObservers(oldArray);
	
	// clears and invalidates all annotations from annotStorage
	kernelPrintDbg(DBG_DBG, "Discarding annotStorage.");
	for(AnnotStorage::iterator i=page->annotStorage.begin(); i!=page->annotStorage.end(); ++i)
		(*i)->invalidate();
	page->annotStorage.clear();

	// creates new annotStorage with new annotations
	kernelPrintDbg(DBG_DBG, "Creating new annotStorage.");
	collectAnnotations(page->dictionary, page->annotStorage);

	// registers obsevers to newValue annotation array - Annots property doesn't
	// need obsever registration for same reason as oldValue doesn't need
	// unregistration
	shared_ptr<IProperty> newArray;
	if(isRef(newValue))
	{
		try
		{
			newArray=getCObjectFromRef<CArray>(newValue);
		}catch(CObjectException & e)
		{
			IndiRef ref=getValueFromSimple<CRef>(newValue);
			kernelPrintDbg(DBG_WARN, "Target of Annots "<<ref<<" is not an array");
		}
	}else
		if(isArray(oldValue))
			newArray=IProperty::getSmartCObjectPtr<CArray>(oldValue);
	if(newArray.get())
		page->registerAnnotsObservers(newArray);
	
	
	kernelPrintDbg(debug::DBG_INFO, "Annotation consolidation done.");
}

//
//
//
void 
CPage::AnnotsArrayWatchDog::notify(
		boost::shared_ptr<IProperty> newValue, 
		boost::shared_ptr<const IProperty::ObserverContext> context) const throw()
{
using namespace debug;
using namespace observer;

	kernelPrintDbg(debug::DBG_DBG, "");
	if(!context.get())
	{
		kernelPrintDbg(DBG_DBG, "No context information.");
		return;
	}
	
	kernelPrintDbg(DBG_DBG, "context type="<<context->getType());
	shared_ptr<IProperty> oldValue;
	switch(context->getType())
	{
		case BasicChangeContextType:
			{
				// this means that Annots element reference value has changed	
				shared_ptr<const BasicChangeContext<IProperty> > basicContext=
					dynamic_pointer_cast<const BasicChangeContext<IProperty>, const observer::IChangeContext<IProperty> >(context); 
				oldValue=basicContext->getOriginalValue();
				assert(isRef(newValue));
				assert(isRef(oldValue));

				// nothing with observers has to be done here
			}
			break;
		case ComplexChangeContextType:
			{
				// Annots array content has changed
				shared_ptr<const CArray::CArrayComplexObserverContext > basicContext=
					dynamic_pointer_cast<const CArray::CArrayComplexObserverContext, 
					const IChangeContext<IProperty> >(context); 
				if(!context)
				{
					kernelPrintDbg(DBG_WARN, "Bad property identificator type.");
					return;
				}
				oldValue=basicContext->getOriginalValue();

				// if oldValue is reference, unregisters this observer from it 
				// because it is no more available
				if(isRef(oldValue))
					UNREGISTER_SHAREDPTR_OBSERVER(oldValue, page->annotsArrayWatchDog);

				// if new value is reference registers this observer to it
				if(isRef(newValue))
					REGISTER_SHAREDPTR_OBSERVER(newValue, page->annotsArrayWatchDog);
			}
			break;
		default:
			kernelPrintDbg(DBG_WARN, "Unsupported context type");
	}

	page->consolidateAnnotsStorage(oldValue, newValue);
	kernelPrintDbg(debug::DBG_INFO, "Annotation consolidation done.");
}

//
//
//
void 
CPage::ContentsWatchDog::notify (boost::shared_ptr<IProperty> newValue, 
								boost::shared_ptr<const IProperty::ObserverContext> context) const throw()
{
	kernelPrintDbg(debug::DBG_DBG, "");
	assert (context);
	kernelPrintDbg (debug::DBG_DBG, "context type=" << context->getType());
	
	switch(context->getType())
	{
		case ComplexChangeContextType:
			{
				//
				// Is it a dictionary (probably Page dictionary)
				//
				shared_ptr<const CDict::CDictComplexObserverContext> ctxtdict =
					dynamic_pointer_cast<const CDict::CDictComplexObserverContext, const IChangeContext<IProperty> > (context); 
				if (ctxtdict)
				{
					shared_ptr<IProperty> oldValue = ctxtdict->getOriginalValue ();
			
					// Contents entry was removed
					if (isNull(newValue))
					{
						// Unregister observer
						UNREGISTER_SHAREDPTR_OBSERVER(oldValue, page->contentsWatchDog);
						
					}else
					{
						// New contents entry
						if ("Contents" == ctxtdict->getValueId ())
								page->registerContentsObserver ();
					}
					break;
				}
				//
				// Is it an array (Probably Contents) -- do nothing just reparse
				//
				shared_ptr<const CArray::CArrayComplexObserverContext> ctxtarray =
					dynamic_pointer_cast<const CArray::CArrayComplexObserverContext, const IChangeContext<IProperty> > (context); 
				if (ctxtarray)
					break;
			}
			break;

		default:
			assert (!"Unsupported context type");
			break;
	}

	// Parse content streams (add or delete of object)
	page->parseContentStream ();

	// Indicate change
	page->_objectChanged ();
}


//
// Constructor
//
CPage::CPage (boost::shared_ptr<CDict>& pageDict) : 
		dictionary(pageDict), 
		valid (true),
		annotsPropWatchDog(new AnnotsPropWatchDog(this)), 
		annotsArrayWatchDog(new AnnotsArrayWatchDog(this)), 
		contentsWatchDog (new ContentsWatchDog(this))

{
	kernelPrintDbg (debug::DBG_DBG, "");
	assert (pageDict);
	assert (valid);

// Better not throw in a constructor
//  if (!isPage (pageDict))
//		throw CObjInvalidObject ();		
	
	// Fill inheritable properties but do not dispatch the change
	// if no change on this document occurs, we do not want to change it (but we
	// do it in the setInheritablePageAttr function)
	dictionary->lockChange();
	setInheritablePageAttr (dictionary);

	// Workaround: pages that miss contents entry
	//  -- it is not a global solution one can delete and add
	//  contents entry of a page
	if (!dictionary->containsProperty("Contents"))
	{
		CArray array;
		dictionary->addProperty ("Contents",array);
	}	
	dictionary->unlockChange();
	
	// collects all annotations from this page and registers observers
	// annotsPropWatchDog has to be registered to page dictionary and the 
	// rest is done by registerAnnotsObservers
	collectAnnotations(dictionary, annotStorage);
	REGISTER_SHAREDPTR_OBSERVER(dictionary, annotsPropWatchDog);
	if(dictionary->containsProperty("Annots"))
	{
		shared_ptr<IProperty> annotsProp=dictionary->getProperty("Annots");
		registerAnnotsObservers(annotsProp);
	}

	// Create contents observer
	registerContentsObserver ();
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
	assert (valid);
	
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
	assert (valid);
	InheritedPageAttr atr;
	fillInheritedPageAttr (dictionary,atr);
	return atr.rotate->getValue();
}


//
//
//
void
CPage::setRotation (int rot)
{
	assert (valid);
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
	assert (valid);
	
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
		REGISTER_SHAREDPTR_OBSERVER(annotsArray, annotsArrayWatchDog);	
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
	checkAndReplace(annotDict, "P", *pageRef);

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
	assert (valid);

	// searches annotation in annotStorage - which is synchronized with current
	// state of Annots array
	size_t pos=0;
	for(AnnotStorage::iterator i=annotStorage.begin(); i!=annotStorage.end(); ++i,++pos)
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

//
//
//
void
CPage::getText (std::string& text, const string* encoding, const Rectangle* rc) const
{
	kernelPrintDbg (debug::DBG_DBG, "");
	assert (valid);

	// Create text output device
	boost::scoped_ptr<TextOutputDev> textDev (new ::TextOutputDev (NULL, gFalse, gFalse, gFalse));
	assert (textDev->isOk());
	if (!textDev->isOk())
		throw CObjInvalidOperation ();

	// Display page
	displayPage (*textDev);	


	// Set encoding
	if (encoding)
    	globalParams->setTextEncoding(const_cast<char*>(encoding->c_str()));

	// Get the text
	if (NULL == rc)
		rc = &(lastParams.pageRect);
	boost::scoped_ptr<GString> gtxt (textDev->getText(rc->xleft, rc->yleft, rc->xright, rc->yright));
	text = gtxt->getCString();
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
	assert (valid);
	
	CArray mb;
	CReal r (rc.xleft);
	mb.addProperty (r);

	r.setValue (rc.yleft);
	mb.addProperty (r);

	r.setValue (rc.xright);
	mb.addProperty (r);

	r.setValue (rc.yright);
	mb.addProperty (r);
		
	dictionary->setProperty ("MediaBox",mb);
}

//
// Display a page
//
void
CPage::displayPage (::OutputDev& out, shared_ptr<CDict> pagedict, int x, int y, int w, int h) const
{
	// Are we in valid pdf
	assert (hasValidPdf (dictionary));
	assert (hasValidRef (dictionary));
	assert (valid);
	if (!hasValidPdf (dictionary) || !hasValidRef (dictionary))
		throw XpdfInvalidObject ();

	// Get xref
	XRef* xref = dictionary->getPdf()->getCXref ();
	assert (NULL != xref);

	//
	// Create xpdf object representing CPage
	//
	if (!(pagedict))
		pagedict = dictionary;
	shared_ptr<Object> xpdfPage (pagedict->_makeXpdfObject(), xpdf::object_deleter());
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
	int rotation = lastParams.rotate - getRotation ();
	page.displaySlice (&out, lastParams.hDpi, lastParams.vDpi,
			rotation, lastParams.useMediaBox, lastParams.crop,
			x, y, w, h, 
			false, xpdfCatalog.get());

}

//
//
//
void 
CPage::displayPage (::OutputDev& out, const DisplayParams& params, int x, int y, int w, int h)
{
	setDisplayParams (params);
	displayPage (out,x,y,w,h);
}

void 
CPage::displayPage (::OutputDev& out, int x, int y, int w, int h) 
{
	assert (valid);
	// display page
	displayPage (out,dictionary,x,y,w,h);
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

	// Get resource dictionary
	InheritedPageAttr atr;
	fillInheritedPageAttr (dictionary,atr);
	
	// Start the resource stack
	XRef* xref = dictionary->getPdf()->getCXref();
	assert (xref);
	Object* obj = atr.resources->_makeXpdfObject ();
	assert (obj); assert (objDict == obj->getType());
	res = boost::shared_ptr<GfxResources> (new GfxResources(xref, obj->getDict(), NULL));
	freeXpdfObject (obj);
	
	//
	// Init Gfx state
	//
	
	// Create Media (Bounding) box
	shared_ptr<PDFRectangle> rc (new PDFRectangle (lastParams.pageRect.xleft,  lastParams.pageRect.yleft,
												  lastParams.pageRect.xright, lastParams.pageRect.yright));
	state = shared_ptr<GfxState> (new GfxState (lastParams.hDpi, lastParams.vDpi, 
												rc.get(), lastParams.rotate, lastParams.upsideDown));
}

//
//
//
bool CPage::parseContentStream ()
{
	assert (valid);
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

	if (!dictionary->containsProperty ("Contents"))
		return true;
	shared_ptr<IProperty> contents = getReferencedObject (dictionary->getProperty ("Contents"));
	assert (contents);
	
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
	// True if Contents is not [ ]
	//assert (!streams.empty());
	while (!streams.empty())
	{
		shared_ptr<CContentStream> cc (new CContentStream(streams,state,res));
		// Save smart pointer of the content stream so pdfoperators can return it
		cc->setSmartPointer (cc);
		contentstreams.push_back (cc);
	}

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
	assert (valid);
	if (contentsWatchDog)
	{ // Register contents observer
		
		if (dictionary->containsProperty("Contents"))
		{
			// Register dictionary and Contents observer
			REGISTER_SHAREDPTR_OBSERVER(dictionary, contentsWatchDog);
			shared_ptr<IProperty> prop = dictionary->getProperty("Contents");
			REGISTER_SHAREDPTR_OBSERVER(prop, contentsWatchDog);
		}
		
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
	assert (valid);

	// If contentsWatchDog is null it can mean that Contents entry was removed
	if (contentsWatchDog)
	{ 
		// Unregister dictionary observer
		UNREGISTER_SHAREDPTR_OBSERVER(dictionary, contentsWatchDog);
		// Unregister contents observer
		if (dictionary->containsProperty("Contents"))
		{
			shared_ptr<IProperty> prop = dictionary->getProperty("Contents");
			REGISTER_SHAREDPTR_OBSERVER(prop, contentsWatchDog);
		}
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
	assert (valid);
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
std::string
CPage::addSystemType1Font (const std::string& fontname, bool winansienc)
{
	// Create font dictionary
	// << 
	//    /Type /Font
	//    /Subtype /Type1
	//    /BaseFont / ...
	// >>
	boost::shared_ptr<CDict> font (new CDict ());
	boost::shared_ptr<CName> name (new CName ("Font"));
	font->addProperty (string("Type"), *name);
	name->setValue ("Type1");
	font->addProperty (string ("Subtype"), *name);
	name->setValue (fontname);
	font->addProperty (string ("BaseFont"), *name);
	
	// For accents
	if (winansienc)
	{
		name->setValue ("WinAnsiEncoding");
		font->addProperty (string ("Encoding"), *name);
	}

	// Resources is an inheritable property, must be present
	if (!dictionary->containsProperty ("Resources"))
	{
		InheritedPageAttr atr;
		fillInheritedPageAttr (dictionary,atr);
		dictionary->addProperty ("Resources", *(atr.resources));
	}
	
	// Get "Resources"
	boost::shared_ptr<CDict> res = getCDictFromDict (dictionary, "Resources");
	
	if (!res->containsProperty ("Font"))
	{	
		boost::shared_ptr<CDict> fontdict (new CDict ());
		res->addProperty ("Font", *fontdict);
	}
	
	// Get "Fonts"
	boost::shared_ptr<CDict> fonts = getCDictFromDict (res, "Font");

	// Get all avaliable fonts
	CPage::FontList fs;
	getFontIdsAndNames (fs);

	// Try PDFEDIT_FONTID{1,2,3,..}, etc., until we find one that's not in 
	// use
	std::ostringstream newfontname;
	int i = 1;
	do {
		newfontname.str(PDFEDIT_FONTID);
		newfontname << i++;
	} while (findFont(fs, newfontname.str()) != fs.end());

	// Add it
	fonts->addProperty (newfontname.str(), *font);
	
	//
	// Create state and resources and update our contentstreams
	//
	shared_ptr<GfxResources> gfxres;
	shared_ptr<GfxState> gfxstate;
	createXpdfDisplayParams (gfxres, gfxstate);

	for (ContentStreams::iterator it = contentstreams.begin(); it != contentstreams.end(); ++it)
		(*it)->setGfxParams (gfxstate, gfxres);

	return newfontname.str();
}
//
//
// TODO maybe change to FontList type?
template<typename Container>
void 
CPage::getFontIdsAndNames (Container& cont) const
{
	// Clear container
	cont.clear ();
	
	InheritedPageAttr atr;
	fillInheritedPageAttr (dictionary, atr);
	boost::shared_ptr<CDict> res = atr.resources;
	
	try 
	{
		boost::shared_ptr<CDict> fonts = utils::getCDictFromDict (res, "Font");
		typedef std::vector<std::string> FontNames;
		FontNames fontnames;
		// Get all font names (e.g. R14, R15, F19...)
		fonts->getAllPropertyNames (fontnames);
		// Get all base names (Symbol, csr12, ...)
		for (FontNames::iterator it = fontnames.begin(); it != fontnames.end(); ++it)
		{
			boost::shared_ptr<CDict> font = utils::getCDictFromDict (fonts, *it);
			try {
				std::string fontbasename;
				
				if (font->containsProperty ("BaseFont")) // Type{1,2} font
					fontbasename = utils::getNameFromDict (font, "BaseFont");
				else									// TrueType font
					fontbasename = utils::getNameFromDict (font, "Subtype");
				cont.push_back (std::make_pair (*it, fontbasename));

			}catch (ElementNotFoundException&)
			{}
		}

	}catch (ElementNotFoundException&)
	{
		kernelPrintDbg (debug::DBG_ERR, "No resource dictionary.");
	}
}
template void CPage::getFontIdsAndNames<vector<pair<string,string> > > (vector<pair<string,string> >& cont) const;

//
// Transform matrix
// 

//
//
//
void
CPage::setTransformMatrix (double tm[6])
{
	assert (valid);
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
CPage::_objectChanged (bool invalid)
{
	assert (valid);
	// Do not notify anything if we are not in a valid pdf
	if (!hasValidPdf (dictionary))
		return;
	assert (hasValidRef (dictionary));

	boost::shared_ptr<CPage> current (this, EmptyDeallocator<CPage> ());

	// Notify observers
	if (invalid)
		this->notifyObservers (current, shared_ptr<const ObserverContext> ());
	else
		this->notifyObservers (current, shared_ptr<const ObserverContext> (new BasicObserverContext (current)));
}


// =====================================================================================
namespace {
// =====================================================================================

	/**
	 * Create stream from container of objects that implement
	 * getStringRepresentation function. And inserts this stream into supplied pdf.
	 */
	template<typename Container>
	shared_ptr<CStream> 
	createStreamFromObjects (const Container& cont, CPdf& pdf)
	{
		// Create stream with one default property Length
		shared_ptr<CStream> newstr (new CStream());
		
		// Insert our change tag
		std::string str;
		{
			std::string tmpop;
			createChangeTag()->getStringRepresentation (tmpop);
			str += tmpop + " ";
		}
		
		//
		// Get string representation of new content stream
		//
		typename Container::const_iterator it = cont.begin();
		for (; it != cont.end(); ++it)
		{
			std::string tmpop;
			(*it)->getStringRepresentation (tmpop);
			str += tmpop;
		}
		kernelPrintDbg (debug::DBG_DBG, str);

		// Set the stream
		newstr->setBuffer (str);

		// Set ref and indiref reserve free indiref for the new object
		IndiRef newref = pdf.addIndirectProperty (newstr);
		newstr = IProperty::getSmartCObjectPtr<CStream> (pdf.getIndirectProperty (newref));
		assert (newstr);

		return newstr;
	}


	/**
	 * Get all cstreams from a container of content streams.
	 */
	template<typename In, typename Out>
	void getAllCStreams (const In& in, Out& out)
	{
		for (typename In::const_iterator it = in.begin(); it != in.end(); ++it)
		{
			Out tmp;
			(*it)->getCStreams (tmp);
			copy (tmp.begin(), tmp.end(), back_inserter (out));
		}
	}
		
	
	/** 
	 * Helper class providing convinient access and modify operations on
	 * "Contents" entry of a page dictionary.
	 */
	struct ContentsHandler
	{
		shared_ptr<CDict> _dict;
			
		ContentsHandler (shared_ptr<CDict> dict) : _dict(dict) {}

		void toFront (CRef& ref)
		{
			if (!_dict->containsProperty ("Contents"))
			{
				CArray arr;
				arr.addProperty (ref);
				_dict->addProperty ("Contents", arr);
				
			}else
			{
				shared_ptr<IProperty> content = _dict->getProperty ("Contents");
				shared_ptr<IProperty> realcontent = getReferencedObject(content);
				assert (content);
				// Contents can be either stream or an array of streams
				if (isStream (realcontent))	
				{
					CArray arr;
					arr.addProperty (ref);
					arr.addProperty (*content);
					_dict->setProperty ("Contents", arr);
			
				}else if (isArray (realcontent))
				{
					// We can be sure that streams are indirect objects (pdf spec)
					shared_ptr<CArray> array = IProperty::getSmartCObjectPtr<CArray> (realcontent);
					array->addProperty (0, ref);
				
				}else // Neither stream nor array
				{
					kernelPrintDbg (debug::DBG_CRIT, "Content stream type: " << realcontent->getType());
					throw ElementBadTypeException ("Bad content stream type.");
				}
			}
		}
	
		void toBack (CRef& ref)
		{
			if (!_dict->containsProperty ("Contents"))
			{
				CArray arr;
				arr.addProperty (ref);
				_dict->addProperty ("Contents", arr);
				
			}else
			{
				shared_ptr<IProperty> content = _dict->getProperty ("Contents");
				shared_ptr<IProperty> realcontent = getReferencedObject(content);
				assert (content);
				// Contents can be either stream or an array of streams
				if (isStream (realcontent))	
				{
					CArray arr;
					arr.addProperty (*content);
					arr.addProperty (ref);
					_dict->setProperty ("Contents", arr);
			
				}else if (isArray (realcontent))
				{
					// We can be sure that streams are indirect objects (pdf spec)
					shared_ptr<CArray> array = IProperty::getSmartCObjectPtr<CArray> (realcontent); 
					array->addProperty (array->getPropertyCount(), ref);
				
				}else // Neither stream nor array
				{
					kernelPrintDbg (debug::DBG_CRIT, "Content stream type: " << realcontent->getType());
					throw ElementBadTypeException ("Bad content stream type.");
				}
			}
		}

		/**
		 * Set Contents entry from a container of content streams.
		 */
		template<typename Cont>
		void setContents (const Cont& cont)
		{
			if (_dict->containsProperty ("Contents"))
				_dict->delProperty ("Contents");
			
			//
			// Loop throug all content streams and add all cstreams from each
			// content streams to "Contents" entry of page dictionary
			//
			typedef vector<shared_ptr<CStream> > Css;
			Css css;
			getAllCStreams (cont, css);
			
			// Loop through all cstreams
			for (Css::iterator it = css.begin(); it != css.end(); ++it)
			{
				assert (hasValidPdf (*it));
				assert (hasValidRef (*it));
				if (!hasValidPdf (*it) || !hasValidRef (*it))
					throw XpdfInvalidObject ();

				CRef rf ((*it)->getIndiRef ());
				toBack (rf);
			}
		}

		/**
		 * Remove content streams references from Contents entry.
		 */
		void remove (shared_ptr<const CContentStream> cs)
		{
			if (!_dict->containsProperty ("Contents"))
				throw CObjInvalidOperation ();
			
			//
			// Loop throug all content streams and add all cstreams from each
			// content streams to "Contents" entry of page dictionary
			//
			typedef vector<shared_ptr<CStream> > Css;
			Css css;
			cs->getCStreams (css);
			
			// Loop through all cstreams
			for (Css::iterator it = css.begin(); it != css.end(); ++it)
			{
				assert (hasValidPdf (*it));
				assert (hasValidRef (*it));
				if (!hasValidPdf (*it) || !hasValidRef (*it))
					throw CObjInvalidObject ();

				// Remove the reference 
				remove ((*it)->getIndiRef ());
			}
		}

private:
		/**
		 * Remove one indiref from Contents entry.
		 */
		void remove (const IndiRef& rf)
		{
			shared_ptr<IProperty> content = _dict->getProperty ("Contents");
			shared_ptr<IProperty> realcontent = getReferencedObject (content);
			assert (content);
			// Contents can be either stream or an array of streams
			if (isStream (realcontent))	
			{
				// Set empty contents
				CArray arr;
				_dict->setProperty ("Contents", arr);
		
			}else if (isArray (realcontent))
			{
				// We can be sure that streams are indirect objects (pdf spec)
				shared_ptr<CArray> array = IProperty::getSmartCObjectPtr<CArray> (realcontent);
				for (size_t i = 0; i < array->getPropertyCount(); ++i)
				{
					IndiRef _rf = getRefFromArray (array,i);
					if (_rf == rf)
					{
						array->delProperty (i);
						return;
					}
				}
			
			}else // Neither stream nor array
			{
				kernelPrintDbg (debug::DBG_CRIT, "Content stream type: " << realcontent->getType());
				throw ElementBadTypeException ("Bad content stream type.");
			}
		}

	
	}; // struct ContentsHandler

// =====================================================================================
} // namespace
// =====================================================================================


//
//
//
template<typename Container>
void 
CPage::addContentStreamToFront (const Container& cont)
{ 
	// Create cstream from container of pdf operators
	CPdf* pdf = dictionary->getPdf();
	assert (pdf);
	shared_ptr<CStream> stream = createStreamFromObjects (cont, *pdf);
	assert (hasValidRef (stream)); assert (hasValidPdf (stream));
	if (!hasValidPdf(stream) || !hasValidPdf(stream))
		throw CObjInvalidObject ();

	// Change the contents entry
	unregisterContentsObserver ();
	ContentsHandler cnts (dictionary);
	CRef ref (stream->getIndiRef());
	cnts.toFront (ref);
	registerContentsObserver ();

	// Parse new stream to content stream and add it to the streams
	CContentStream::CStreams streams;
	streams.push_back (stream);
	boost::shared_ptr<GfxResources> res;
	boost::shared_ptr<GfxState> state;
	createXpdfDisplayParams (res, state);
	ContentStreams _tmp;
	boost::shared_ptr<CContentStream> cc (new CContentStream(streams,state,res));
	// Save smart pointer of the content stream so pdfoperators can return it
	cc->setSmartPointer (cc);
	_tmp.push_back (cc);

	std::copy (contentstreams.begin(), contentstreams.end(), std::back_inserter(_tmp));
	contentstreams = _tmp;

	// Indicate change
	_objectChanged ();
}
template void CPage::addContentStreamToFront<vector<shared_ptr<PdfOperator> > > (const vector<shared_ptr<PdfOperator> >& cont);
template void CPage::addContentStreamToFront<deque<shared_ptr<PdfOperator> > > (const deque<shared_ptr<PdfOperator> >& cont);

//
//
//
template<typename Container>
void 
CPage::addContentStreamToBack (const Container& cont)
{
	// Create cstream from container of pdf operators
	CPdf* pdf = dictionary->getPdf();
	assert (pdf);
	shared_ptr<CStream> stream = createStreamFromObjects (cont, *pdf);
	assert (hasValidRef (stream)); assert (hasValidPdf (stream));
	if (!hasValidPdf(stream) || !hasValidPdf(stream))
		throw CObjInvalidObject ();

	// Change the contents entry
	unregisterContentsObserver ();
	ContentsHandler cnts (dictionary);
	CRef ref (stream->getIndiRef());
	cnts.toBack (ref);
	registerContentsObserver ();

	// Parse new stream to content stream and add it to the streams
	CContentStream::CStreams streams;
	streams.push_back (stream);
	boost::shared_ptr<GfxResources> res;
	boost::shared_ptr<GfxState> state;
	createXpdfDisplayParams (res, state);
	boost::shared_ptr<CContentStream> cc (new CContentStream(streams,state,res));
	// Save smart pointer
	cc->setSmartPointer (cc);
	contentstreams.push_back (cc);

	// Indicate change
	_objectChanged ();
}
template void CPage::addContentStreamToBack<vector<shared_ptr<PdfOperator> > > (const vector<shared_ptr<PdfOperator> >& cont);
template void CPage::addContentStreamToBack<deque<shared_ptr<PdfOperator> > > (const deque<shared_ptr<PdfOperator> >& cont);


//
//
//
void 
CPage::removeContentStream (size_t csnum)
{
	// Create cstream from container of pdf operators
	assert (dictionary->getPdf());

	if (csnum >= contentstreams.size())
		throw OutOfRange ();

	// Change the contents entry
	unregisterContentsObserver ();
	ContentsHandler cnts (dictionary);
	cnts.remove (contentstreams[csnum]);
	registerContentsObserver ();

	// Remove contentstream from container
	contentstreams.erase (contentstreams.begin() + csnum, contentstreams.begin() + csnum + 1);

	// Indicate change
	_objectChanged ();
}


//
// Our changes
//

// =====================================================================================
namespace {
// =====================================================================================
	
	/** Sort according to the time of change. 
	 * Least means the change was the last one.
	 */
	struct ccs_change_sorter 
	{
		bool operator() (shared_ptr<CContentStream> frst, shared_ptr<CContentStream> scnd)
		{
			typedef vector<shared_ptr<PdfOperator> > Ops;
			static const bool FIRST_IS_OUR_LAST = true;
			static const bool SECOND_IS_OUR_LAST = false;
			
			Ops opFrst, opScnd;

			frst->getPdfOperators (opFrst);
			scnd->getPdfOperators (opScnd);
			assert (!opFrst.empty());
			assert (!opScnd.empty());
			
			ChangePdfOperatorIterator itFrst = PdfOperator::getIterator<ChangePdfOperatorIterator> (opFrst.front());
			ChangePdfOperatorIterator itScnd = PdfOperator::getIterator<ChangePdfOperatorIterator> (opScnd.front());
			assert (itFrst.valid());
			assert (itScnd.valid());

			time_t tmfrst = getChangeTagTime (itFrst.getCurrent());
			time_t tmscnd = getChangeTagTime (itScnd.getCurrent());
			
			if (tmfrst > tmscnd)
				return FIRST_IS_OUR_LAST;
			else
				return SECOND_IS_OUR_LAST;
		}
	};
// =====================================================================================
} // namespace
// =====================================================================================

//
//
//
template<typename Container>
void
CPage::getChanges (Container& cont) const
{
	cont.clear();
	for (ContentStreams::const_iterator it = contentstreams.begin(); it != contentstreams.end(); ++it)
	{
		typedef vector<shared_ptr<PdfOperator> > Ops;
		Ops ops;
		(*it)->getPdfOperators (ops);
		
		// Empty contentstream is not our change
		if (ops.empty())
			continue;
		
		ChangePdfOperatorIterator chng = PdfOperator::getIterator<ChangePdfOperatorIterator> (ops.front());
		// Not containing our change tag meaning not our change
		if (!chng.valid()) 
			continue;

		cont.push_back (*it);
	}
	sort (cont.begin(), cont.end(), ccs_change_sorter());
}
template void CPage::getChanges<vector<shared_ptr<CContentStream> > > (vector<shared_ptr<CContentStream> >& cont) const;


//
//
//
shared_ptr<CContentStream>
CPage::getChange (size_t nthchange) const
{
	typedef vector<shared_ptr<CContentStream> > CCs;
	
	CCs ccs;
	getChanges (ccs);

	if (nthchange >= ccs.size())
		throw OutOfRange ();
	
	return ccs[nthchange];
}

//
//
//
size_t
CPage::getChangeCount () const
{
	size_t cnt = 0;
	for (ContentStreams::const_iterator it = contentstreams.begin(); it != contentstreams.end(); ++it)
	{
		typedef vector<shared_ptr<PdfOperator> > Ops;
		Ops ops;
		(*it)->getPdfOperators (ops);
		
		// Empty contentstream is not our change
		if (ops.empty())
			continue;
		
		ChangePdfOperatorIterator chng = PdfOperator::getIterator<ChangePdfOperatorIterator> (ops.front());
		// Not containing our change tag meaning not our change
		if (!chng.valid()) 
			continue;

		// Found another change
		++cnt;
	}

	return cnt;
}


//
//
//
template<typename Container>
void 
CPage::displayChange (::OutputDev& out, const Container& cont) const
{

	shared_ptr<CDict> fakeDict (IProperty::getSmartCObjectPtr<CDict>(dictionary->clone()));
	assert (fakeDict);
	ContentsHandler hnd (fakeDict);
	hnd.setContents (cont);

	//
	// Beware: from this point, fakeDict is not changed so xpdf can be set
	// otherwise the condition that when pdf is valid ref must be also is not
	// met
	//
	if (hasValidPdf (dictionary))
		fakeDict->setPdf(dictionary->getPdf());

	// Display page using our dictionary
	displayPage (out, fakeDict);
}
template void CPage::displayChange<vector<shared_ptr<CContentStream> > > (::OutputDev& out, const vector<shared_ptr<CContentStream> >& cont) const;


//
//
//
void 
CPage::moveAbove (shared_ptr<const CContentStream> ct)
{
	// Get the next item
	ContentStreams::iterator itNext = find (contentstreams.begin(), contentstreams.end(), ct);
	if (itNext == contentstreams.end())
		throw CObjInvalidOperation ();
	++itNext;
	if (itNext == contentstreams.end())
		throw OutOfRange ();

	// Delete next item but store it
	shared_ptr<CContentStream> tmp = *itNext;
	contentstreams.erase (itNext, itNext + 1);
	// Insert stored item before supplied (simply swap ct with the next item)
	contentstreams.insert (find (contentstreams.begin(), contentstreams.end(), ct), tmp);

	// Also change Contents entry of page dictionary
	unregisterContentsObserver ();
	ContentsHandler cnts (dictionary);
	cnts.setContents (contentstreams);
	registerContentsObserver ();

	// Indicate change
	_objectChanged ();
}

//
//
//
void 
CPage::moveBelow (shared_ptr<const CContentStream> ct)
{
	// Get the item index
	unsigned int pos = 0;
	for (pos = 0; pos < contentstreams.size(); ++pos)
		if (contentstreams[pos] == ct)
			break;
	
	// If first or not found throw exception
	if (pos == contentstreams.size() || 0 == pos)
		throw CObjInvalidOperation ();
	assert (0 < pos);
	
	// Swap
	shared_ptr<CContentStream> tmp = contentstreams[pos];
	contentstreams[pos] = contentstreams[pos - 1];
	contentstreams[pos - 1] = tmp;

	// Also change Contents entry of page dictionary
	unregisterContentsObserver ();
	ContentsHandler cnts (dictionary);
	cnts.setContents (contentstreams);
	registerContentsObserver ();

	// Indicate change
	_objectChanged ();
}


//
// Page position
//
size_t
CPage::getPagePosition () const
{
	if (hasValidPdf (dictionary))
		return dictionary->getPdf()->getPagePosition (shared_ptr<CPage> (const_cast<CPage*>(this),EmptyDeallocator<CPage> ()));

	throw CObjInvalidOperation ();
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

	if ("Page" != getStringFromDict (dict, "Type"))
		throw CObjInvalidObject ();

	return true;
}

// =====================================================================================
} // namespace pdfobjects
// =====================================================================================
