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
#include "kernel/carray.h"
#include "kernel/cpdf.h"
#include "kernel/factories.h"

//=====================================================================================
namespace pdfobjects {
//=====================================================================================

using namespace std;
using namespace boost;
	

//=====================================================================================
// CArray
//=====================================================================================

//
// Constructors
//

//
// Protected constructor
//
CArray::CArray (boost::weak_ptr<CPdf> p, const Object& o, const IndiRef& rf) : IProperty (p,rf) 
{
	// Build the tree from xpdf object
	utils::complexValueFromXpdfObj<pArray,Value&> (*this, o, value);
}

//
// Protected constructor
//
CArray::CArray (const Object& o)
{
	// Build the tree from xpdf object
	utils::complexValueFromXpdfObj<pArray,Value&> (*this, o, value);
}


//
// Get methods
//

//
//
//
void 
CArray::getStringRepresentation (string& str) const 
{
	utils::complexValueToString<CArray> (value,str);
}


//
//
//
boost::shared_ptr<IProperty>
CArray::getProperty (PropertyId id) const
{
	//kernelPrintDbg (debug::DBG_DBG,"getProperty() " << id);

	if (id >= value.size())
		throw OutOfRange ();
	
	boost::shared_ptr<IProperty> ip = value[id];
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
CArray::setPdf (boost::weak_ptr<CPdf> pdf)
{
	// Set pdf to this object
	IProperty::setPdf (pdf);

	// Set new pdf to all its children
	Value::iterator it = value.begin();
	for (; it != value.end(); ++it)
		(*it)->setPdf (pdf);
}

//
//
//
void 
CArray::setIndiRef (const IndiRef& rf)
{
	// Set pdf to this object
	IProperty::setIndiRef (rf);

	// Set new pdf to all its children
	Value::iterator it = value.begin();
	for (; it != value.end(); ++it)
		(*it)->setIndiRef (rf);
}


//
// 
//
void
CArray::delProperty (PropertyId id)
{
	//kernelPrintDbg (debug::DBG_DBG,"delProperty(" << id << ")");

	// Check if we are out of bounds
	if (id >= value.size())
		throw OutOfRange ();
	
	// Check whether we can make the change
	this->canChange();

	boost::shared_ptr<IProperty> oldip = value[id];

	// Delete that item
	value.erase (remove (value.begin(), value.end(), oldip));

	if (hasValidPdf (this))
	{
		assert (hasValidRef (this));
		
		// Create contest
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
		{ assert (!hasValidRef (this)); }
}


//
// Correctly to add an object (without name) can be done only to Array object
//
boost::shared_ptr<IProperty>
CArray::addProperty (const IProperty& newIp)
{
	//kernelPrintDbg (debug::DBG_DBG,"addProperty(...)");
	return addProperty (value.size(), newIp);
}

//
// Correctly to add an object (without name) can be done only to Array object
//
// REMARK: because of the compiler, we can't put PropertyId here
//
boost::shared_ptr<IProperty>
CArray::addProperty (PropertyId position, const IProperty& newIp)
{
	//kernelPrintDbg (debug::DBG_DBG,"addProperty(" << position << ")");

	//
	// Check if we add to a valid position
	//
	if (position > value.size())
		throw OutOfRange ();
	
	// Check whether we can make the change
	this->canChange();

	// Clone the added property
	boost::shared_ptr<IProperty> newIpClone = newIp.clone ();
	assert (newIpClone);
	
	// Inherit id, gen number and pdf
	newIpClone->setPdf (this->getPdf());
	newIpClone->setIndiRef (this->getIndiRef());
	
	// Find the correct position
	Value::iterator it;
	if (value.size() == position)
		it = value.end ();
	else
		for (it = value.begin(); 0 != position; ++it, --position)
			;
	// Insert it
	value.insert (it,newIpClone);
	
	if (hasValidPdf (this))
	{
		assert (hasValidRef (this));
		
		// Create contest
		boost::shared_ptr<ObserverContext> context (_createContext(boost::shared_ptr<IProperty>(new CNull ()), position));

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
	_setMode (newIpClone, position);
	
	return newIpClone;
}


//
//
//
boost::shared_ptr<IProperty>
CArray::setProperty (PropertyId id, IProperty& newIp)
{
	//kernelPrintDbg (debug::DBG_DBG, "setProperty(" << id << ")");

	// Check the bounds, if fails add it
	if (id >= value.size())
		return addProperty (id, newIp);

	// Check whether we can make the change
	this->canChange();

	// Save the old one
	boost::shared_ptr<IProperty> oldip = value[id];
	// Clone the added property
	boost::shared_ptr<IProperty> newIpClone = newIp.clone ();
	assert (newIpClone);
	
	// Inherit id, gen number and pdf
	newIpClone->setIndiRef (this->getIndiRef());
	newIpClone->setPdf (this->getPdf());

	// Construct item, and replace it with this one
	replace (value.begin(), value.end(), oldip, newIpClone);

	//
	// Dispatch change if we are in valid pdf
	// 
	if (hasValidPdf (this))
	{	
		assert (hasValidRef (this));
		
		// Create contest
		boost::shared_ptr<ObserverContext> context (_createContext (oldip,id));	

		try {
			// notify observers and dispatch the change
			_objectChanged (newIpClone, context);

		}catch (PdfException&)
		{
			assert (!"Should not happen.. Condition must be included in CPdf::canChange()...");
			throw;
		}

		// Be sure
		oldip->setPdf (boost::shared_ptr<CPdf>());
		oldip->setIndiRef (IndiRef());

	}else
		{ assert (!hasValidRef (this)); }

	// Set mode only if pdf is valid
	_setMode (newIpClone, id);
	
	return newIpClone;
}


//
// Helper methods
//

//
//
//
::Object*
CArray::_makeXpdfObject () const
{
	//kernelPrintDbg (debug::DBG_DBG,"_makeXpdfObject");
	::Object * arrayObj = XPdfObjectFactory::getInstance();

	// We need to be carefull with pdf from property. Some
	// properties (usually artificial ones) can be without
	// any bound PDF instance. Nevertheless NULL xref is ok for
	// Object hierarchy (see Object::fetch method)
	XRef *xref = NULL;
	boost::shared_ptr<CPdf> pdf = getPdf().lock();
	if(pdf)
		xref = pdf->getCXref();
	arrayObj->initArray(xref);

	Value::const_iterator it = value.begin();
	for (; it != value.end(); ++it)
	{
		Object * propObj = (*it)->_makeXpdfObject();
		arrayObj->arrayAdd(propObj);
		gfree(propObj);
	}
	assert(static_cast<unsigned int>(arrayObj->arrayGetLength()) == getPropertyCount());
	return arrayObj;
}


//
//
//
void 
CArray::_objectChanged (boost::shared_ptr<IProperty> newValue, boost::shared_ptr<const ObserverContext> context)
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
CArray::_setMode (boost::shared_ptr<IProperty> ip, PropertyId /*id*/) const
{
	/* FIXME this is not good implementation because we don't have
	 * information about array name here. So if it is direct child
	 * of some dictionary, we will set mode for the array correctly
	 * (according mode rules) but all children properties have
	 * different mode which match the most general setting (
	 * .: mdAdvanced in our default configuration).
	 * We need to get the name of direct property from the parent 
	 * dictionary. Until this is implemented we will simply set 
	 * child's mode to the array mode.
	 */
	/*
	configuration::ModeController* modecontroller = NULL;
	if (hasValidPdf (this) && (NULL != (modecontroller=this->getPdf()->getModeController())))
	{
		assert (modecontroller);		
		
		ostringstream oss;
		oss << id;

		PropertyMode mode = modecontroller->getMode ("",oss.str());
		if (mdUnknown == mode)
			ip->setMode (this->getMode());
		else
			ip->setMode (mode);
	}
	*/
	ip->setMode(this->getMode());
}



//
// Clone method
//
IProperty*
CArray::doClone () const
{
	//kernelPrintDbg (debug::DBG_DBG,"");
	
	// Make new complex object
	// NOTE: We do not want to inherit any IProperty variable
	CArray* clone_ = _newInstance ();
	
	// Loop through all items and clone them as well and finally add them to the new object
	Value::const_iterator it = value.begin ();
	for (; it != value.end (); ++it)
		clone_->value.push_back ((*it)->clone());

	return clone_;
}

//
//
//
IProperty::ObserverContext* 
CArray::_createContext (boost::shared_ptr<IProperty> changedIp, PropertyId id)
{
	//kernelPrintDbg (debug::DBG_DBG, "");

	// Create the context
	return new CArrayComplexObserverContext (changedIp, id);
}



//=====================================================================================
} // namespace pdfobjects
//=====================================================================================

