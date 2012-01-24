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

#include "kernel/cpageannots.h"
#include "kernel/cannotation.h"
#include "utils/observer.h"
#include "kernel/cobject.h"
#include "kernel/cobjecthelpers.h"
#include "kernel/factories.h"
#include "kernel/cpage.h"
#include "kernel/cpdf.h"

//==========================================================
namespace pdfobjects {
//==========================================================

using namespace std;
using namespace boost;
using namespace observer;
using namespace utils;
using namespace debug;


//==========================================================
namespace {
//==========================================================


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
	boost::shared_ptr<CArray> 
	getAnnotsArray(boost::shared_ptr<CDict> pageDict)
	{
		boost::shared_ptr<CArray> annotsArray = pageDict->getProperty<CArray>("Annots");

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
			boost::shared_ptr<CArray> annotsArray=getAnnotsArray(pageDict);

			// gets all Annots elements - these has to be referencies to
			// dictionaries
			for(size_t i=0; i<annotsArray->getPropertyCount(); ++i)
			{
				try
				{
					boost::shared_ptr<CDict> annotDict=annotsArray->getProperty<CDict>(i);

					// creates CAnnotation instance and inserts it to the container
					boost::shared_ptr<CAnnotation> annot(new CAnnotation(annotDict));
					container.push_back(annot);
				}catch(ElementBadTypeException &)
				{
					kernelPrintDbg(debug::DBG_WARN, "Annots["<<i<<"] target object is not dictionary. Ignoring.");
				}
			}
			
		}catch(CObjectException & e)
		{
			kernelPrintDbg(debug::DBG_INFO, "Unable to get Annots array - message="<<e.what());
		}
	}

//==========================================================
}	// namespace
//==========================================================


//
//
//
CPageAnnots::CPageAnnots (CPage* page) : 
		_page(page),
		_prop_wd(new PropWatchDog(this)), 
		_array_wd(new ArrayWatchDog(this))
{
	// collects all annotations from this page and registers observers
	// _prop_wd has to be registered to page dictionary and the 
	// rest is done by registerAnnotsObservers
	collectAnnotations(_page->getDictionary(), _annotations);

	// Register observers
	reg_observers ();
}

//
//
//
CPageAnnots::~CPageAnnots ()
{
	reset ();
}


//
// Observers
//

// =====================================================================================
namespace {
// =====================================================================================

	typedef struct reg_annots
	{
		void operator() (boost::shared_ptr<IProperty> obj, boost::shared_ptr<IPropertyObserver> obs)
			{ REGISTER_SHAREDPTR_OBSERVER (obj, obs); }
	}reg_annots;

	typedef struct unreg_annots
	{
		void operator() (boost::shared_ptr<IProperty> obj, boost::shared_ptr<IPropertyObserver> obs)
			{ UNREGISTER_SHAREDPTR_OBSERVER (obj, obs); }
	}unreg_annots;

	// generic {un}register function
	template<typename T>
	void egister_annots (boost::shared_ptr<IProperty> annots, 
						 T egister, 
						 boost::shared_ptr<IPropertyObserver> _prop_wd,
						 boost::shared_ptr<IPropertyObserver> _array_wd)
	{
		boost::shared_ptr<CArray> annotsArray;
		if(isRef(annots))
		{
			egister(annots, _prop_wd);
			try
			{
				annotsArray=getCObjectFromRef<CArray>(annots);
			}catch(CObjectException&)
			{
				kernelPrintDbg(DBG_WARN, getValueFromSimple<CRef>(annots)<<" doesn't point to an array.");
			}
		}else if (isArray(annots))
		{
			annotsArray=IProperty::getSmartCObjectPtr<CArray>(annots);
		}

			// chech validity
			if(!(annotsArray))
			{
				kernelPrintDbg(DBG_WARN, "Given annots is not or does not point to array.");
				return;
			}

		typedef std::vector<boost::shared_ptr<IProperty> > ChildrenStorage;
		ChildrenStorage children;
		egister(annotsArray, _array_wd);
		annotsArray->_getAllChildObjects(children);
		for(ChildrenStorage::iterator i=children.begin(); i!=children.end(); ++i)
			if(isRef(*i))
				egister(*i, _array_wd);
	}

//=====================================================================================
} // annonymous namespace
//=====================================================================================

//
//
//
void 
CPageAnnots::reg_observers(boost::shared_ptr<IProperty> annots)
{
		kernelPrintDbg(DBG_DBG, "");

	// annots object
	if (annots)
		egister_annots<reg_annots> (annots, reg_annots(), _prop_wd, _array_wd);

	// Default page dictionary
	else
	{
		REGISTER_SHAREDPTR_OBSERVER(_page->getDictionary(), _prop_wd);
		if(_page->getDictionary()->containsProperty(Specification::Page::ANNOTS))
		{
			boost::shared_ptr<IProperty> annotsProp=_page->getDictionary()->getProperty(Specification::Page::ANNOTS);
			reg_observers(annotsProp);
		}
	}
}

//
//
//
void 
CPageAnnots::unreg_observers(boost::shared_ptr<IProperty> annots)
{
		kernelPrintDbg(DBG_DBG, "");

	// annots object
	if (annots)
		egister_annots<unreg_annots> (annots, unreg_annots(), _prop_wd, _array_wd);

	// Default page dictionary
	else
	{
		UNREGISTER_SHAREDPTR_OBSERVER(_page->getDictionary(), _prop_wd);
		if(_page->getDictionary()->containsProperty(Specification::Page::ANNOTS))
		{
			boost::shared_ptr<IProperty> annotsProp=_page->getDictionary()->getProperty(Specification::Page::ANNOTS);
			unreg_observers(annotsProp);
		}
	}
}


//
// Annotations
//

//
//
//
void 
CPageAnnots::consolidate(boost::shared_ptr<IProperty>& oldValue, boost::shared_ptr<IProperty>& newValue)
{
	// handle original value - one which is removed or replaced 
	// this has to be invalidated and removed from _annotations
	// this is skipped if oldValue is not reference
	if(isRef(oldValue))
	{
		try
		{
			boost::shared_ptr<CDict> annotDict=getCObjectFromRef<CDict>(oldValue);
			Annotations::iterator i;
			bool erased = false;
			for(i=_annotations.begin(); i!=_annotations.end(); ++i)
			{
				boost::shared_ptr<CAnnotation> annot=*i;
				if(annot->getDictionary()==annotDict)
				{
					kernelPrintDbg(debug::DBG_DBG, "Annotation maintaining oldValue found and removed. Invalidating annotation");	
					annot->invalidate();
					_annotations.erase(i);
					erased = true;
					break;
				}
			}
			if(!erased)
				kernelPrintDbg(debug::DBG_WARN, "Removed value is not in _annotations.");
		}catch(ElementBadTypeException & )
		{
			kernelPrintDbg(debug::DBG_WARN, "oldValue dereferenced value is not dictionary.");
		}
	}

	kernelPrintDbg(debug::DBG_DBG, "oldValue is handled now.");

	// handle new value - one which is added or replaces an old value
	// this has to be added to _annotations

	// checks whether dereferenced property is dictionary. If not it 
	// means that some mass is provided and so it is ignored
	// this is skipped if newValue is not reference
	if(isRef(newValue))
	{
		try
		{
			boost::shared_ptr<CDict> annotDict=getCObjectFromRef<CDict>(newValue);		

			// creates CAnnotation instance from dereferenced dictionary 
			// and adds it to _annotations
			boost::shared_ptr<CAnnotation> annot(new CAnnotation(annotDict));
			_annotations.push_back(annot);
		}catch(ElementBadTypeException & )
		{
			kernelPrintDbg(debug::DBG_WARN, "Dereferenced newValue is not dictionary.");
		}
	}
}

//
//
//
void
CPageAnnots::add(boost::shared_ptr<CAnnotation> annot)
{
	kernelPrintDbg(debug::DBG_DBG, "");
	
	// gets pdf of this annotation - it is valid
	boost::shared_ptr<CPdf> pdf=_page->getDictionary()->getPdf().lock();
	if (!pdf)
		throw CObjInvalidObject ();
		
	// gets Annots array from page dictionary. If no found, creates it. If bad
	// typed, throws an exception
	boost::shared_ptr<CArray> annotsArray;
	try
	{
		annotsArray=getAnnotsArray(_page->getDictionary());
	}catch(ElementBadTypeException & )
	{
		// TODO provide also bad type information
		kernelPrintDbg(debug::DBG_ERR, "Page's Annots field is malformed. Array property expected.");
		throw;
	}
	catch(ElementNotFoundException &)
	{
		kernelPrintDbg(debug::DBG_INFO, "Page's Annots field missing. Creating one.");
		
		// Annots array doesn't exist - so creates one and registers observer to
		// it.
		// annotsArray must be set from getProperty method because addProperty
		// creates deep copy of given
		boost::scoped_ptr<IProperty> tmpArray(CArrayFactory::getInstance());
		_page->getDictionary()->addProperty(Specification::Page::ANNOTS, *tmpArray);
		annotsArray=_page->getDictionary()->getProperty<CArray>(Specification::Page::ANNOTS);
		REGISTER_SHAREDPTR_OBSERVER(annotsArray, _array_wd);	
	}

	kernelPrintDbg(debug::DBG_DBG, "Creating new indirect dictionary for annotation.");
	
	// addes annotation dictionary to the pdf - this will add deep copy if given
	// dictionary and also solves problems with annotation from different pdf
	IndiRef annotRef=pdf->addIndirectProperty(annot->getDictionary());
	
	// gets added annotation dictionary
	boost::shared_ptr<CDict> annotDict=IProperty::getSmartCObjectPtr<CDict>(
			pdf->getIndirectProperty(annotRef)
			);

	kernelPrintDbg(debug::DBG_DBG, "Setting annotation dictionary field P="<<_page->getDictionary()->getIndiRef());
	// updates P field with reference to this page
	// This is not explictly required by specification for all annotation types,
	// but is not an error to supply this information
	boost::shared_ptr<CRef> pageRef(CRefFactory::getInstance(_page->getDictionary()->getIndiRef()));
	checkAndReplace(annotDict, "P", *pageRef);

	kernelPrintDbg(debug::DBG_INFO, "Adding reference "<<annotRef<<" to annotation dictionary to Annots array");
	// annotation dictionary is prepared and so its reference can be stored	
	// to Annots array from page dictionary - this will start observer which
	// will update _annotations
	boost::scoped_ptr<CRef> annotCRef(CRefFactory::getInstance(annotRef));
	annotsArray->addProperty(*annotCRef);
}

//
//
//
bool
CPageAnnots::del(boost::shared_ptr<CAnnotation> annot)
{
		kernelPrintDbg(debug::DBG_DBG, "");

	// searches annotation in _annotations - which is synchronized with current
	// state of Annots array
	size_t pos=0;
	for(Annotations::iterator i=_annotations.begin(); i!=_annotations.end(); ++i,++pos)
	{
		boost::shared_ptr<CAnnotation> element=*i;	
		if(annot!=element)
			continue;
		
		// annotation found, removes dictionary reference from Annots array
		IndiRef annotRef=element->getDictionary()->getIndiRef();
		kernelPrintDbg(debug::DBG_DBG, "Annotation found. Indiref="<<annotRef);
		try
		{
			boost::shared_ptr<CArray> annotArray=getAnnotsArray(_page->getDictionary());
			// deleting of this reference triggers annotsWatchDog observer which
			// will synchronize _annotations with current state
			annotArray->delProperty(pos);
			kernelPrintDbg(debug::DBG_INFO, "Annotation referece "<<annotRef<<" removed from Annots array. "
					<<"Invalidating annotation instance.");
			annot->invalidate();
			return true;
		}catch(CObjectException &)
		{
			kernelPrintDbg(debug::DBG_ERR, "Unexpected Annots array missing.");
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
CPageAnnots::ArrayWatchDog::notify(
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
	boost::shared_ptr<IProperty> oldValue;
	switch(context->getType())
	{
		case BasicChangeContextType:
			{
				// this means that Annots element reference value has changed	
				boost::shared_ptr<const BasicChangeContext<IProperty> > basicContext=
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
				boost::shared_ptr<const CArray::CArrayComplexObserverContext > basicContext=
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
					UNREGISTER_SHAREDPTR_OBSERVER(oldValue, _annots->_array_wd);

				// if new value is reference registers this observer to it
				if(isRef(newValue))
					REGISTER_SHAREDPTR_OBSERVER(newValue, _annots->_array_wd);
			}
			break;
		default:
			kernelPrintDbg(DBG_WARN, "Unsupported context type");
	}

	_annots->consolidate(oldValue, newValue);
	kernelPrintDbg(debug::DBG_INFO, "Annotation consolidation done.");
}


//
//
//
void 
CPageAnnots::PropWatchDog::notify(
		boost::shared_ptr<IProperty> newValue, 
		boost::shared_ptr<const IProperty::ObserverContext> context) const throw()
{

	kernelPrintDbg(debug::DBG_DBG, "");
	if(!context.get())
	{
		kernelPrintDbg(DBG_DBG, "No context information.");
		return;
	}
	
	kernelPrintDbg(DBG_DBG, "context type="<<context->getType());
	boost::shared_ptr<IProperty> oldValue;
	switch(context->getType())
	{
		case BasicChangeContextType:
			{
				// This means that Annots property is reference and it has
				// changed its reference value
				boost::shared_ptr<const observer::BasicChangeContext<IProperty> > basicContext=
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
				boost::shared_ptr<const CDict::CDictComplexObserverContext > basicContext=
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
					UNREGISTER_SHAREDPTR_OBSERVER(oldValue, _annots->_array_wd);

				// if new value is reference, register this observer to it
				if(isRef(newValue))
					REGISTER_SHAREDPTR_OBSERVER(newValue, _annots->_prop_wd);
			}
			break;
		default:
			kernelPrintDbg(DBG_WARN, "Unsupported context type");
	}

	// gets original annots array and unregisters all observers
	// doesn't unregister observer from Annots property, because it is done only
	// in case of complex context
	boost::shared_ptr<IProperty> oldArray;
	if(isRef(oldValue))
	{
		try
		{
			oldArray=getCObjectFromRef<CArray>(oldValue);
		}catch(CObjectException & )
		{
			IndiRef ref=getValueFromSimple<CRef>(oldValue);
			kernelPrintDbg(DBG_WARN, "Target of Annots "<<ref<<" is not an array");
		}
	}else
		if(isArray(oldValue))
			oldArray=IProperty::getSmartCObjectPtr<CArray>(oldValue);
	if(oldArray.get())
		_annots->unreg_observers(oldArray);
	
	// clears and invalidates all annotations from annotStorage
	kernelPrintDbg(DBG_DBG, "Discarding annotStorage.");
	for(Annotations::iterator i=_annots->_annotations.begin(); i!=_annots->_annotations.end(); ++i)
		(*i)->invalidate();
	_annots->_annotations.clear();

	// creates new annotStorage with new annotations
	kernelPrintDbg(DBG_DBG, "Creating new annotStorage.");
	collectAnnotations(_annots->_page->getDictionary(), _annots->_annotations);

	// registers obsevers to newValue annotation array - Annots property doesn't
	// need obsever registration for same reason as oldValue doesn't need
	// unregistration
	boost::shared_ptr<IProperty> newArray;
	if(isRef(newValue))
	{
		try
		{
			newArray=getCObjectFromRef<CArray>(newValue);
		}catch(CObjectException & )
		{
			IndiRef ref=getValueFromSimple<CRef>(newValue);
			kernelPrintDbg(DBG_WARN, "Target of Annots "<<ref<<" is not an array");
		}
	}else
		if(isArray(newValue))
			newArray=IProperty::getSmartCObjectPtr<CArray>(newValue);
	if(newArray.get())
		_annots->reg_observers(newArray);
	
	
	kernelPrintDbg(debug::DBG_INFO, "Annotation consolidation done.");
}

//
//
//
void CPageAnnots::reset ()
{
		// we already made a reset
		if (!_page)
			return;
	// we cannot unregister annotation observers with unreg_annots because
	// this method requires access to properties which might be impossible
	// because its CPdf can be already dead by that time. We should, however
	// deactivate all observers and they will be deallocated automatically
	_prop_wd->setActive(false);
	_array_wd->setActive(false);
	_page = NULL;
	_annotations.clear ();
	_prop_wd.reset ();
	_array_wd.reset ();
		assert (!_prop_wd.use_count());
		assert (!_array_wd.use_count());
}

//==========================================================
} // namespace pdfobjects
//==========================================================
