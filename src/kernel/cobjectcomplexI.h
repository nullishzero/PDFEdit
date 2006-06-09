// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80
/*
 * =====================================================================================
 *        Filename:  cobjectcomplexI.h
 *         Created:  02/02/2006 
 *          Author:  jmisutka (06/01/19), 
 * 			
 * =====================================================================================
 */

#ifndef _COBJECTCOMPLEXI_H
#define _COBJECTCOMPLEXI_H

#include "cpdf.h"

//=====================================================================================
namespace pdfobjects {
//=====================================================================================


//=====================================================================================
// CObjectComplex
//=====================================================================================

//
// Constructors
//

//
// Protected constructor
//
template<PropertyType Tp, typename Checker>
CObjectComplex<Tp,Checker>::CObjectComplex (CPdf& p, Object& o, const IndiRef& rf) : IProperty (&p,rf) 
{
	Checker check; check.objectCreated (this);
	//kernelPrintDbg (debug::DBG_DBG,"CObjectComplex <" << debug::getStringType<Tp>() << ">(p,o,rf) constructor.");
	
	// Build the tree from xpdf object
	utils::complexValueFromXpdfObj<Tp,Value&> (*this, o, value);
}

//
// Protected constructor
//
template<PropertyType Tp, typename Checker>
CObjectComplex<Tp,Checker>::CObjectComplex (Object& o)
{
	Checker check; check.objectCreated (this);
	//kernelPrintDbg (debug::DBG_DBG,"CObjectComplex <" << debug::getStringType<Tp>() << ">(o) constructor.");
	
	// Build the tree from xpdf object
	utils::complexValueFromXpdfObj<Tp,Value&> (*this, o, value);
}


//
// Public constructor
//
template<PropertyType Tp, typename Checker>
CObjectComplex<Tp,Checker>::CObjectComplex ()
{
	Checker check; check.objectCreated (this);
	//kernelPrintDbg (debug::DBG_DBG,"CObjectComplex <" << debug::getStringType<Tp>() << ">() constructor.");
}


//
// Get methods
//

//
//
//
template<PropertyType Tp, typename Checker>
void 
CObjectComplex<Tp,Checker>::getStringRepresentation (std::string& str) const 
{
	utils::complexValueToString<Tp> (value,str);
}

//
// Template  member functions can't be virutal (at least according to nowadays specification)
//
template<PropertyType Tp, typename Checker>
template<typename Container>
void
CObjectComplex<Tp,Checker>::getAllPropertyNames (Container& container) const
{
	STATIC_CHECK ((Tp != pArray), INCORRECT_USE_OF_getAllPropertyNames_FUNCTION);
	//kernelPrintDbg (debug::DBG_DBG, "getAllPropertyNames()");

	utils::getAllNames (container, value);
}

//
//
//
template<PropertyType Tp, typename Checker>
bool
CObjectComplex<Tp,Checker>::containsProperty (const std::string& name) const
{
	STATIC_CHECK ((Tp != pArray), INCORRECT_USE_OF_getAllPropertyNames_FUNCTION);
	//kernelPrintDbg (debug::DBG_DBG, "getAllPropertyNames()");

	for (typename Value::const_iterator it = value.begin(); it != value.end(); ++it)
	{
		if ((*it).first == name)
			return true;
	}

	return false;
}

//
//
//
template<PropertyType Tp, typename Checker>
boost::shared_ptr<IProperty>
CObjectComplex<Tp,Checker>::getProperty (PropertyId id) const
{
	//kernelPrintDbg (debug::DBG_DBG,"getProperty() " << id);
	//
	// BEWARE using std::find_if with stateful functors !!!!!
	//
	IndexComparator cmp (id);
	typename Value::const_iterator it = value.begin();
	for (; it != value.end(); ++it)
	{
			if (cmp (*it))
					break;
	}

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
template<PropertyType Tp, typename Checker>
void 
CObjectComplex<Tp,Checker>::setPdf (CPdf* pdf)
{
	// Set pdf to this object
	IProperty::setPdf (pdf);

	// Set new pdf to all its children
	typename Value::iterator it = value.begin();
	for (; it != value.end(); ++it)
		utils::getIPropertyFromItem (*it)->setPdf (pdf);
}

//
//
//
template<PropertyType Tp, typename Checker>
void 
CObjectComplex<Tp,Checker>::setIndiRef (const IndiRef& rf)
{
	// Set pdf to this object
	IProperty::setIndiRef (rf);

	// Set new pdf to all its children
	typename Value::iterator it = value.begin();
	for (; it != value.end(); ++it)
		utils::getIPropertyFromItem (*it)->setIndiRef (rf);
}


//
// 
//
template<PropertyType Tp, typename Checker>
void
CObjectComplex<Tp,Checker>::delProperty (PropertyId id)
{
	kernelPrintDbg (debug::DBG_DBG,"delProperty(" << id << ")");

	//
	// BEWARE using std::find_if with stateful functors !!!!!
	// We could have used getProperty but we also need the iterator
	//
	IndexComparator cmp (id);
	typename Value::iterator it = value.begin();
	for (; it != value.end(); ++it)
	{
			if (cmp (*it))
					break;
	}
	
	if (it == value.end())
		throw ElementNotFoundException ("CObjectComplex", "item not found");
	
	boost::shared_ptr<IProperty> ip = cmp.getIProperty ();
	if (ip)
	{
		if (hasValidPdf (this))
		{
			assert (hasValidRef (this));
			
			// Create the context
			boost::shared_ptr<ObserverContext> context (this->_createContext(ip));
			// Delete that item
			value.erase (it);
			// Indicate that this object has changed
			boost::shared_ptr<IProperty> changedObj (new CNull());
			_objectChanged (changedObj, context);
			
			// Be sure
			ip->setPdf (NULL);
			ip->setIndiRef (IndiRef());
	
		}else
		{
			assert (!hasValidRef (this));
			// Delete that item
			value.erase (it);
		}
		
	}else
		throw CObjInvalidObject ();

}


//
// Correctly to add an object (without name) can be done only to Array object
//
template<PropertyType Tp, typename Checker>
boost::shared_ptr<IProperty>
CObjectComplex<Tp,Checker>::addProperty (const IProperty& newIp)
{
	STATIC_CHECK ((Tp == pArray), INCORRECT_USE_OF_addProperty_FUNCTION);
	kernelPrintDbg (debug::DBG_DBG,"addProperty(...)");

	return addProperty (value.size(), newIp);
}

//
// Correctly to add an object (without name) can be done only to Array object
//
// REMARK: because of the compiler, we can't put PropertyId here
//
template<PropertyType Tp, typename Checker>
boost::shared_ptr<IProperty>
CObjectComplex<Tp,Checker>::addProperty (size_t position, const IProperty& newIp)
{
	STATIC_CHECK ((Tp == pArray), INCORRECT_USE_OF_addProperty_FUNCTION);
	kernelPrintDbg (debug::DBG_DBG,"addProperty(" << position << ")");

	//
	// Check if we add to a valid position
	//
	if (position > value.size())
		throw OutOfRange ();
	
	// Clone the added property
	boost::shared_ptr<IProperty> newIpClone = newIp.clone ();
	assert (newIpClone);
	
	typename Value::iterator it;
	// Find the correct position
	if (value.size() == position)
		it = value.end ();
	else
		for (it = value.begin(); 0 != position; ++it, --position)
			;

	// Inherit id, gen number and pdf
	newIpClone->setPdf (this->getPdf());
	newIpClone->setIndiRef (this->getIndiRef());
	// Insert it
	value.insert (it,newIpClone);
	
	if (hasValidPdf (this))
	{
		assert (hasValidRef (this));
		
		// Notify observers and dispatch change
		_objectChanged (newIpClone, 
					boost::shared_ptr<ObserverContext>
						(new BasicObserverContext 
						 	(boost::shared_ptr<IProperty> (new CNull ()))
						)
				   );
	}
	
	// Set mode only if pdf is valid
	_setMode (newIpClone, position);
	
	return newIpClone;
}


//
// Correctly add an object (with name) can be done only to Dict and Stream object
//
template<PropertyType Tp, typename Checker>
boost::shared_ptr<IProperty>
CObjectComplex<Tp,Checker>::addProperty (const std::string& propertyName, const IProperty& newIp)
{
	STATIC_CHECK ((Tp == pDict), INCORRECT_USE_OF_addProperty_FUNCTION);
	kernelPrintDbg (debug::DBG_DBG,"addProperty( " << propertyName << ",...)");

	// Clone the added property
	boost::shared_ptr<IProperty> newIpClone = newIp.clone ();
	if (newIpClone)
	{
		// Inherit id, gen number and pdf
		newIpClone->setIndiRef (this->getIndiRef());
		newIpClone->setPdf (this->getPdf());
	
		// Store it
		value.push_back (std::make_pair (propertyName,newIpClone));
		
	}else
		throw CObjInvalidObject ();

	//
	// Dispatch change if we are in valid pdf
	// 
	if (hasValidPdf (this))
	{
		assert (hasValidRef (this));
		
		// notify observers and dispatch change
		_objectChanged (newIpClone,
					boost::shared_ptr<ObserverContext> 
						(new BasicObserverContext 
						 	(boost::shared_ptr<IProperty> (new CNull ()))
						)
					);
	}

	// Set mode only if pdf is valid
	_setMode (newIpClone, propertyName);
	
	return newIpClone;
}


//
//
//
template<PropertyType Tp, typename Checker>
boost::shared_ptr<IProperty>
CObjectComplex<Tp,Checker>::setProperty (PropertyId id, IProperty& newIp)
{
	kernelPrintDbg (debug::DBG_DBG, "setProperty(" << id << ")");
	
	//
	// Find the item we want
	// BEWARE using std::find_if with stateful functors !!!!!
	//
	IndexComparator cmp (id);
	typename Value::iterator it = value.begin();
	for (; it != value.end(); ++it)
	{
		if (cmp (*it))
				break;
	}

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
	typename Value::value_type newVal = utils::constructItemFromIProperty (*it, newIpClone);
	std::fill_n (it, 1, newVal);

	//
	// Dispatch change if we are in valid pdf
	// 
	if (hasValidPdf (this))
	{	
		assert (hasValidRef (this));
		
		// Create context
		boost::shared_ptr<ObserverContext> context (_createContext (oldIp));	
		// Notify observers and dispatch change
		_objectChanged (newIpClone, context);

		// Be sure
		oldIp->setPdf (NULL);
		oldIp->setIndiRef (IndiRef());
	}

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
template <PropertyType Tp, typename Checker>
::Object*
CObjectComplex<Tp,Checker>::_makeXpdfObject () const
{
	//kernelPrintDbg (debug::DBG_DBG,"_makeXpdfObject");
	
	std::string rpr;
	getStringRepresentation (rpr);

	if (hasValidPdf (this))
		return utils::xpdfObjFromString (rpr, this->getPdf()->getCXref());
	else
		return utils::xpdfObjFromString (rpr);
}


//
//
//
template<PropertyType Tp, typename Checker>
template <typename Storage>
void 
CObjectComplex<Tp,Checker>::_getAllChildObjects (Storage& store) const
{
	typename Value::const_iterator it = value.begin ();
	for	(; it != value.end (); ++it)
	{
		boost::shared_ptr<IProperty> ip = utils::getIPropertyFromItem (*it);
		store.push_back (ip);
	}
}

//
//
//
template<PropertyType Tp, typename Checker>
void 
CObjectComplex<Tp,Checker>::_objectChanged 
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
template <PropertyType Tp, typename Checker>
void
CObjectComplex<Tp,Checker>::_setMode (boost::shared_ptr<IProperty> ip, PropertyId id) const
{
	configuration::ModeController* modecontroller = NULL;
	if (hasValidPdf (this) && (NULL != (modecontroller=this->getPdf()->getModeController())))
	{
		assert (modecontroller);		
		PropertyMode mode = utils::getModeForComplexObjects (value, id, *modecontroller);
		if (mdUnknown == mode)
			ip->setMode (this->getMode());
		else
			ip->setMode (mode);
	}
}



//
// Clone method
//
template<PropertyType Tp, typename Checker>
IProperty*
CObjectComplex<Tp,Checker>::doClone () const
{
	//kernelPrintDbg (debug::DBG_DBG,"");
	
	// Make new complex object
	// NOTE: We do not want to inherit any IProperty variable
	CObjectComplex<Tp,Checker>* clone_ = _newInstance ();
	
	//
	// Loop through all items and clone them as well and finally add them to the new object
	//
	typename Value::const_iterator it = value.begin ();
	for (; it != value.end (); ++it)
	{
		boost::shared_ptr<IProperty> newIp = utils::getIPropertyFromItem(*it)->clone ();
		assert (newIp);
		typename Value::value_type item =  utils::constructItemFromIProperty (*it, newIp);
		clone_->value.push_back (item);
	}

	return clone_;
}

//
//
//
template<PropertyType Tp, typename Checker>
IProperty::ObserverContext* 
CObjectComplex<Tp,Checker>::_createContext (boost::shared_ptr<IProperty>& changedIp)
{
	//kernelPrintDbg (debug::DBG_DBG, "");

	// Create the context
	return new BasicObserverContext (changedIp);
}


//
// Destructor
//
template<PropertyType Tp, typename Checker>
CObjectComplex<Tp,Checker>::~CObjectComplex ()
{
	Checker check; check.objectDeleted (this);
}

//=====================================================================================
} // namespace pdfobjects
//=====================================================================================



#endif // _COBJECTCOMPLEXI_H
