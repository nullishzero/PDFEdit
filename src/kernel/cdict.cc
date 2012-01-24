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

#include "kernel/static.h"
#include "kernel/cdict.h"
#include "kernel/cpdf.h"
#include "kernel/factories.h"

//=====================================================================================
namespace pdfobjects {
//=====================================================================================

using namespace std;
using namespace boost;

//=====================================================================================
// CDict
//=====================================================================================

//
// Constructors
//

//
// Protected constructor
//
CDict::CDict (boost::weak_ptr<CPdf> p, const Object& o, const IndiRef& rf) : IProperty (p,rf) 
{
	// Build the tree from xpdf object
	utils::complexValueFromXpdfObj<pDict,Value&> (*this, o, value);
}

//
// Protected constructor
//
CDict::CDict (const Object& o)
{
	// Build the tree from xpdf object
	utils::complexValueFromXpdfObj<pDict,Value&> (*this, o, value);
}


//
// Get methods
//

//
//
//
void 
CDict::getStringRepresentation (string& str) const 
{
	utils::complexValueToString<CDict> (value,str);
}

//
//
//
bool
CDict::containsProperty (const string& name) const
{
	//kernelPrintDbg (debug::DBG_DBG, "getAllPropertyNames()");

	for ( Value::const_iterator it = value.begin(); it != value.end(); ++it)
	{
		if ((*it).first == name)
			return true;
	}

	return false;
}

//
//
//
boost::shared_ptr<IProperty>
CDict::getProperty (PropertyId id) const
{
	//kernelPrintDbg (debug::DBG_DBG,"getProperty() " << id);
	//
	// BEWARE using find_if with stateful functors !!!!!
	//
	DictIdxComparator cmp (id);
	Value::const_iterator it = value.begin();
	for (; it != value.end(); ++it)
		if (cmp (*it))
			break;

	if (it == value.end())
		throw ElementNotFoundException ("", "");
	
	boost::shared_ptr<IProperty> ip = cmp.getIProperty ();

	// Set mode only if pdf is valid
	_setMode (ip,id);

	return ip;
}


//
// Set methods
//

//
//
//
void 
CDict::setPdf (boost::weak_ptr<CPdf> pdf)
{
	// Set pdf to this object
	IProperty::setPdf (pdf);

	// Set new pdf to all its children
	Value::iterator it = value.begin();
	for (; it != value.end(); ++it)
		(*it).second->setPdf (pdf);
}

//
//
//
void
CDict::init (const CDict& dict)
{
	std::copy (dict.value.begin(), dict.value.end(), std::back_inserter (value));
}

//
//
//
void 
CDict::setIndiRef (const IndiRef& rf)
{
	// Set pdf to this object
	IProperty::setIndiRef (rf);

	// Set new pdf to all its children
	Value::iterator it = value.begin();
	for (; it != value.end(); ++it)
		(*it).second->setIndiRef (rf);
}


//
// 
//
void
CDict::delProperty (PropertyId id)
{
	kernelPrintDbg (debug::DBG_DBG,"delProperty(" << id << ")");

	// Check whether we can make the change
	this->canChange();

	//
	// BEWARE using find_if with stateful functors !!!!!
	// We could have used getProperty but we also need the iterator
	//
	DictIdxComparator cmp (id);
	Value::iterator oldit = value.begin();
	for (; oldit != value.end(); ++oldit)
		if (cmp (*oldit))
			break;
	
	if (oldit == value.end())
		throw ElementNotFoundException ("CDict", "item not found");
	
	boost::shared_ptr<IProperty> oldip = cmp.getIProperty ();
	
	// Delete that item	
	value.erase (oldit);

	if (hasValidPdf (this))
	{
		assert (hasValidRef (this));
		
		// Indicate that this object has changed
		boost::shared_ptr<ObserverContext> context (_createContext (oldip,id));	
		
		try {
			// notify observers and dispatch the change
			_objectChanged (boost::shared_ptr<IProperty> (new CNull), context);
			
		}catch (PdfException&)
		{
			assert (!"Should not happen.. Condition must be included in CPdf::canChange()...");
			throw;
		}

		// Be sure
		oldip->setPdf (boost::shared_ptr<CPdf>());
		oldip->setIndiRef (IndiRef());

	}else
	{ 
		assert (!hasValidRef (this)); 
	}
}


//
// Correctly add an object (with name) can be done only to Dict and Stream object
//
boost::shared_ptr<IProperty>
CDict::addProperty (const string& propertyName, const IProperty& newIp)
{
	kernelPrintDbg (debug::DBG_DBG,"addProperty( " << propertyName << ",...)");

	// Check whether we can make the change
	this->canChange();

	// check whether we contain such a property
	//	-- pdf spec allows objects having two identical names, we forbid it!!!
	//		-- e.g. because of double contents entry etc..
	if (containsProperty(propertyName))
		throw CObjInvalidObject ();

	// Clone the added property
	boost::shared_ptr<IProperty> newIpClone = newIp.clone ();
	if (newIpClone)
	{
		// Inherit id, gen number and pdf
		newIpClone->setIndiRef (this->getIndiRef());
		newIpClone->setPdf (this->getPdf());
	
		// Store it
		value.push_back (make_pair (propertyName,newIpClone));
		
	}else
		throw CObjInvalidObject ();

	//
	// Dispatch change if we are in valid pdf
	// 
	if (hasValidPdf (this))
	{
		assert (hasValidRef (this));
		
		// notify observers and dispatch change
		boost::shared_ptr<ObserverContext> context (_createContext(boost::shared_ptr<IProperty>(new CNull ()), propertyName));

		try {
			// notify observers and dispatch the change
			_objectChanged (newIpClone, context);
			
		}catch (PdfException&)
		{
			assert (!"Should not happen.. Condition must be included in CPdf::canChange()...");
			throw;
		}
	
	}else
		{ assert (!hasValidRef (this)); }

	// Set mode only if pdf is valid
	_setMode (newIpClone, propertyName);
	
	return newIpClone;
}


//
//
//
boost::shared_ptr<IProperty>
CDict::setProperty (PropertyId id, IProperty& newIp)
{
	kernelPrintDbg (debug::DBG_DBG, "setProperty(" << id << ")");
	
	// Check whether we can make the change
	this->canChange();

	//
	// Find the item we want
	// BEWARE using find_if with stateful functors !!!!!
	//
	DictIdxComparator cmp (id);
	Value::iterator it = value.begin();
	for (; it != value.end(); ++it)
		if (cmp (*it))
			break;

	// Check the bounds, if fails add it
	if (it == value.end())
		return addProperty (id, newIp);

	// Save the old one
	boost::shared_ptr<IProperty> oldIp = cmp.getIProperty ();
	// Clone the added property
	boost::shared_ptr<IProperty> newIpClone = newIp.clone ();
	assert (newIpClone);
	
	// Inherit id, gen number and pdf
	newIpClone->setIndiRef (this->getIndiRef());
	newIpClone->setPdf (this->getPdf());

	// Construct item, and replace it with this one
	fill_n (it, 1, make_pair ((*it).first, newIpClone));

	//
	// Dispatch change if we are in valid pdf
	// 
	if (hasValidPdf (this))
	{	
		assert (hasValidRef (this));
		
		// Notify observers and dispatch change
		boost::shared_ptr<ObserverContext> context (_createContext (oldIp,id));

		try {
			// notify observers and dispatch the change
			_objectChanged (newIpClone, context);

		}catch (PdfException&)
		{
			assert (!"Should not happen.. Condition must be included in CPdf::canChange()...");
			throw;
		}

		// Be sure
		oldIp->setPdf (boost::shared_ptr<CPdf>());
		oldIp->setIndiRef (IndiRef());
	
	}else
	{ 
		assert (!hasValidRef (this)); 
	}

	// Set mode only if pdf is valid
	_setMode (newIpClone,id);
	
	return newIpClone;
}


//
// Helper methods
//

//
//
//
::Object*
CDict::_makeXpdfObject () const
{
	//kernelPrintDbg (debug::DBG_DBG,"_makeXpdfObject");
	::Object * dictObj = XPdfObjectFactory::getInstance();

	// We need to be carefull with pdf from property. Some
	// properties (usually artificial ones) can be without
	// any bound PDF instance. Nevertheless NULL xref is ok for
	// Object hierarchy (see Object::fetch method)
	XRef *xref = NULL;
	boost::shared_ptr<CPdf> pdf = getPdf().lock();
	if(pdf)
		xref = pdf->getCXref();
	dictObj->initDict(xref);

	Value::const_iterator it = value.begin();
	for (; it != value.end(); ++it)
	{
		boost::shared_ptr<IProperty> prop = it->second;
		Object * propObj = prop->_makeXpdfObject();
		dictObj->dictAdd(copyString((it->first).c_str()), propObj);
		gfree(propObj);
	}
	assert(static_cast<unsigned int>(dictObj->dictGetLength()) == getPropertyCount());

	return dictObj;
}


//
//
//
void 
CDict::_objectChanged 
	(boost::shared_ptr<IProperty> newValue, boost::shared_ptr<const ObserverContext> context)
{
	// Do not notify anything if we are not in a valid pdf
	if (!hasValidPdf (this))
		return;
	assert (hasValidRef (this));

	// Dispatch the change
	this->dispatchChange ();
	
	if (context)
	{
		// Notify everybody about this change
		this->notifyObservers (newValue, context);

	}else
	{
		assert (!"Invalid context");
		throw CObjInvalidOperation ();
	}
}

//
// Set mode
//
void
CDict::_setMode (boost::shared_ptr<IProperty> ip, PropertyId id) const
{
	configuration::ModeController* modecontroller = NULL;
	boost::shared_ptr<CPdf> p = this->getPdf().lock();
	if ( p && (NULL != (modecontroller=p->getModeController())))
	{
		assert (modecontroller);		
		PropertyMode mode;
		
		//
		// We can not use containsProperty and getValue because they call this
		// function and an infinite  cycle would occur
		//
		DictIdxComparator cmp ("Type");
		Value::const_iterator it = value.begin();
		for (; it != value.end(); ++it)
		{
			if (cmp (*it))
				break;
		}
		if (it == value.end())
		{ // No type found
			mode = modecontroller->getMode ("", id);
			
		}else	
		{ // We have found a type
			string tmp;
			boost::shared_ptr<IProperty> type = cmp.getIProperty ();
			if (isName (type))
				IProperty::getSmartCObjectPtr<CName>(type)->getValue(tmp);
			mode = modecontroller->getMode (tmp, id);

			/* TODO Can we add parentName and name (id) which can 
			 * be set to the child at this moment? So if child is 
			 * an array, it can use this information for mode 
			 * controllorer for elements.
			 */
		}
	
		if (mdUnknown == mode)
			ip->setMode (this->getMode());
		else
			ip->setMode (mode);
	}
}



//
// Clone method
//
IProperty*
CDict::doClone () const
{
	//kernelPrintDbg (debug::DBG_DBG,"");
	
	// Make new complex object
	// NOTE: We do not want to inherit any IProperty variable
	CDict* clone_ = _newInstance ();
	
	//
	// Loop through all items and clone them as well and finally add them to the new object
	//
	 Value::const_iterator it = value.begin ();
	for (; it != value.end (); ++it)
		clone_->value.push_back (make_pair ((*it).first, (*it).second->clone()));

	return clone_;
}

//
//
//
IProperty::ObserverContext* 
CDict::_createContext (boost::shared_ptr<IProperty> changedIp, PropertyId id)
{
	// Create the context
	return new CDictComplexObserverContext (changedIp, id);
}


//=====================================================================================
} // namespace pdfobjects
//=====================================================================================
