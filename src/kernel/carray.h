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
#ifndef _CARRAY_H
#define _CARRAY_H

// all basic includes
#include "kernel/static.h"
#include "kernel/cobjectsimple.h"


//=====================================================================================
namespace pdfobjects {
//=====================================================================================

//=====================================================================================
// CArray
//=====================================================================================

/** 
 * Class representing array object from pdf specification v1.5.
 *
 * We do not want to use xpdf array because it is a real mess and it is really
 * not suitable for editing. We use xpdf object just for initializing.
 * 
 * According to pdf specification, array is a container of values.
 * Value position identifies an item. You can rely on a value position.
 * Value can be any object from pdf specification. Pdf objects form a tree like
 * structure.
 * 
 * This class does not provide default copy constructor because copying a
 * property could be understood either as deep copy or shallow copy. 
 * Copying complex types could be very expensive so we have made the decision to
 * avoid it.
 *
 * REMARK: It is similar to CDict but it has also too much differences to be
 * cleanly implemented as one template class. (It has been implemented like one
 * template class but later was seperated to CArray and CDict)
 *
 * \see CObjectSimple, CDict, CStream
 */
class CArray : noncopyable, public IProperty
{
public:
	typedef std::vector<boost::shared_ptr<IProperty> > Value; 
	typedef const std::string&		 				 WriteType; 
	typedef size_t			 						 PropertyId;
	typedef observer::ComplexChangeContext<IProperty, PropertyId> CArrayComplexObserverContext;

	/** 
	 * Type of this property object.
	 * This association allows us to get the PropertyType from object type.
	 */
	static const PropertyType type = pArray;
private:
	
	/** Array representation. */
	Value value;


	//
	// Constructors
	//
public:
	/**
	 * Constructor.
	 *
	 * @param p		Pointer to pdf object.
	 * @param o		Xpdf object. 
	 * @param rf	Indirect id and gen id.
	 */
	CArray (boost::weak_ptr<CPdf> p, const Object& o, const IndiRef& rf);

	/**
	 * Constructor.
	 *
	 * @param o		Xpdf object. 
	 */
	CArray (const Object& o);


public:	
	
	/** 
	 * Public constructor. This object will not be associated with a pdf.
	 */
	CArray () {}


	//
	// Cloning
	//
protected:

	/**
	 * Clone. Performs deep copy.
	 * REMARK: It will not copy pdf indirect objects that are referenced from the pdf object tree 
	 * starting in this object.
	 *
	 * @return Deep copy of this object.
	 */
	virtual IProperty* doClone () const;

	/** 
	 * Return new instance. 
	 *
	 * This function is a factory method design pattern for creating complex instances.
	 *
	 * @return New complex object.
	 */
	virtual CArray* _newInstance () const
		{ return new CArray; }
	
	//
	// Get methods
	//
public:

	/**
	 * Return type of this property.
	 *
	 * @return Type of this property.
	 */
	virtual PropertyType getType () const {return type;}
	

	/**
	 * Returns string representation of this object according to pdf
	 * specification. 
	 * 
	 * @param str 	Output string that will hold the string representation 
	 * 				of current object.
	 */
	virtual void getStringRepresentation (std::string& str) const;

	
	/** 
	 * Returns property count.
	 * 
	 * @return Property count.
	 */
	size_t getPropertyCount () const 
	{
		//kernelPrintDbg (debug::DBG_DBG, "getPropertyCount(" << debug::getStringType<Tp>() << ") = " << value.size());
		return value.size();
	}
 

	/**
	 * Returns value of property identified by its position.
   	 *
   	 * @param 	id 	Variable identifying position of the property.
	 * @return	Output variable where the value will be stored.
   	 */
	boost::shared_ptr<IProperty> getProperty (PropertyId id) const;

	/** 
	 * Returns property identified by its name.
	 * This is a convenient method which also does the casting trickery
	 * and indirect object resolution if the given template type is not
	 * reference itself.
	 *
   	 * @param 	id 	Name of the property.
	 * @return	Output variable where the value will be stored.
	 */
	template<typename ItemType> boost::shared_ptr<ItemType> getProperty(PropertyId id)const {
		boost::shared_ptr<IProperty> ip = getProperty(id);

		if (ItemType::type != pRef && isRef(ip))
			ip = utils::getReferencedObject(ip);

		if(ItemType::type != ip->getType()) {
			kernelPrintDbg (debug::DBG_DBG, "wanted type " << ItemType::type 
					<< " got " << ip->getType () << " key[" << id << "]");
			throw ElementBadTypeException("");
		}

		return IProperty::getSmartCObjectPtr<ItemType>(ip);
	}

	/**
	 * Returns property type of an item identified by position.
	 *
	 * @param	id	Position of the property.
	 * @return Property type.	
	 *
	 * \exception ObjInvalidPositionInComplex When the id does not correctly identify an item.
	 */
	PropertyType getPropertyType (PropertyId id) const 
		{ return getProperty(id)->getType(); }

	
	//
	// Set methods
	//
public:
	/**
	 * Set pdf to this object and its children.
	 * @param pdf New pdf.
	 */
	virtual void setPdf (boost::weak_ptr<CPdf> pdf);

	/**
	 * Set ref to this object and its children.
	 * @param rf New indirect reference number.
	 */
	virtual void setIndiRef (const IndiRef& rf);

	/**
	 * Sets property type of an item.
	 * 
	 * Firstly, the property that is passed as argument is cloned and
	 * the cloned object replaces object specified by id. If the item does not
	 * exist it is added.
	 * 
	 * @param	id		Position of property
	 * @param	ip		Value, for simple types (int,string,...) and for complex types IProperty*
	 * @return Pointer to the new property.
	 */
	boost::shared_ptr<IProperty> setProperty (PropertyId id, IProperty& ip);
	
	/**
	 * Add property to array. 
	 *
	 * Firstly, the property that is passed as argument is cloned and 
	 * the cloned object is added.
	 * Indicate that this object has changed and return the pointer to the cloned object.
	 *
	 * REMARK: It is automatically associated with the pdf where the object is beeing added.
	 *
	 * @param newIp 		New property.
	 * @return Pointer to the new property.
	 *
	 * \exception OutOfRange Thrown when property not found.
	 */
	boost::shared_ptr<IProperty> addProperty (const IProperty& newIp);

	/** \copydoc addProperty */
	boost::shared_ptr<IProperty> addProperty (PropertyId position, const IProperty& newIp);
	
	
	/**
	 * Remove property from array. 
	 *
	 * @param id Position of property
	 *
	 * \exception ElementNotFoundException Thrown when object is not found.
	 */
	void delProperty (PropertyId id);


	//
	// Destructor
	//
public:
	/**
	 * Destructor
	 */
	~CArray () {}
		

	//
	// Helper methods
	//
public:

	/**
	 * Apply functor operator() on each element.
	 * The operator() will get std::pair<int, shared_ptr<IProperty>> as
	 * parameter. First item identifies the property.
	 * 
	 * @param fctor Functor that will do the work.
	 */
	template<typename Fctor>
	void forEach (Fctor& fctor)
	{
		typename Value::iterator it = value.begin ();
		for (int pos = 0; it != value.end (); ++it, ++pos)
			fctor (std::make_pair (pos, *it));
	}

	/**
	 * Make xpdf Object from this object. This function allocates and initializes xpdf object.
	 * Caller has to deallocate the xpdf Object.
	 *
	 * @return Xpdf object representing this object.
	 *
	 * \exception ObjBadValueE Thrown when xpdf can't parse the string representation of this
	 * object correctly.
	 */
	virtual ::Object* _makeXpdfObject () const;

private:
	/**
	 * Create context of a change.
	 *
	 * REMARK: Be carefull. Deallocate the object.
	 * 
	 * @param changedIp Pointer to old value.
	 * @param id		Id identifies changed property.
	 * 
	 * @return Context in which a change occured.
	 */
	ObserverContext* _createContext (boost::shared_ptr<IProperty> changedIp, PropertyId id);

	/**
	 * Indicate that the object has changed.
	 * Notifies all observers associated with this property about the change.
	 *
	 * @param newValue Pointer to new value of an object.
	 * @param context Context in which a change occured.
	 */
	void _objectChanged (boost::shared_ptr<IProperty> newValue, 
						 boost::shared_ptr<const ObserverContext> context);

	//
	// Mode interface
	//
protected:
	/**
	 * Set mode of a property.
	 *
	 * @param ip IProperty which mode will be set.
	 * @param id Position of the property.
	 */
	void _setMode (boost::shared_ptr<IProperty> ip, PropertyId id) const;

public:
	/**
	 * Return all child objects.
	 *
	 * @param store Output container of all child objects.
	 */
	template <typename Storage>
	void _getAllChildObjects (Storage& store) const
	{
		Value::const_iterator it = value.begin ();
		for	(; it != value.end (); ++it)
			store.push_back (*it);
	}

};

//=====================================================================================
namespace utils {
//=====================================================================================

//=========================================================
// Creation functions
//=========================================================

/**
 * Save real xpdf object value to val.
 * 
 * @param ip 	IProperty that will be initialized by obj.
 * @param obj	Xpdf object which holds the value.
 * @param val	Variable where the value will be stored.
 */
template <PropertyType Tp,typename T> void complexValueFromXpdfObj (IProperty& ip, const ::Object& obj, T val);

//=========================================================
// To string functions
//=========================================================


/**
 * Return complex xpdf object (null,number,string...) in string representation.
 *
 * REMARK: It is a template function because I think stream won't be converted to string
 * as easily as a dictionary. So we specialize these function for pArray and pDict.
 *
 * @param val that will be converted to string.
 * @param str string
 */
template <typename T> void complexValueToString (const typename T::Value& val, std::string& str);

//=========================================================
//	CArray "get value" helper methods
//=========================================================

/**
 * Get simple value from array.
 *
 * \todo Can use MPL because ItemType and ItemPType depend on each other.!!
 *
 * @param array	Array.
 * @param position 	Position in the array.
 */
template<typename SimpleValueType, typename ItemType, PropertyType ItemPType>
inline SimpleValueType
getSimpleValueFromArray (const boost::shared_ptr<CArray>& array, size_t position) DEPRECATED;

// gcc doesn't like __attribute__ in function definition
template<typename SimpleValueType, typename ItemType, PropertyType ItemPType>
inline SimpleValueType
getSimpleValueFromArray (const boost::shared_ptr<CArray>& array, size_t position)
{
	utilsPrintDbg (debug::DBG_DBG, "array[" << position << "]");
	
	// Get the item and check if it is the correct type
	boost::shared_ptr<IProperty> ip = array->getProperty (position);
	// Check the type and get the value
	return getValueFromSimple<ItemType> (ip);
}

/** Gets simple value from array.
 * Gets property from given position and uses getValueFromSimple to get its
 * simple value.
 *
 * @param array Array property.
 * @param position Index in array of property.
 *
 * @return simple value of property.
 */
template<typename ItemType>
inline typename ItemType::Value
getSimpleValueFromArray (const boost::shared_ptr<CArray>& array, size_t position)
{
	utilsPrintDbg (debug::DBG_DBG, "array[" << position << "]");
	
	// Get the item and check if it is the correct type
	boost::shared_ptr<IProperty> ip = array->getProperty (position);
	// Check the type and get the value
	return getValueFromSimple<ItemType> (ip);
}

/** \copydoc getSimpleValueFromArray */
template<typename SimpleValueType, typename ItemType, PropertyType ItemPType>
inline SimpleValueType
getSimpleValueFromArray (const boost::shared_ptr<IProperty>& array, size_t position) DEPRECATED;
	
// gcc doesn't like __attribute__ in function definition
template<typename SimpleValueType, typename ItemType, PropertyType ItemPType>
inline SimpleValueType
getSimpleValueFromArray (const boost::shared_ptr<IProperty>& ip, size_t position)
{
	assert (isArray (ip));
	if (!isArray (ip))
	{
		assert (!"Ip is not an array.");
		throw ElementBadTypeException ("getSimpleValueFromArray()");
	}

	// Cast it to array
	boost::shared_ptr<CArray> array = IProperty::getSmartCObjectPtr<CArray> (ip);
	// Get value from array
	return getSimpleValueFromArray<SimpleValueType, ItemType, ItemPType> (array, position);
}

/** Gets simple value from array.
 * Checks type of given ip and if it is array, casts it to CArray and gets value
 * of property with given position (uses getSimpleValueFromArray with CArray
 * parameter).
 *
 * @param ip Array property.
 * @param position Array index.
 * @throw ElementBadTypeException if given property is not CArray or property
 * with given index is not ItemType.
 */
template<typename ItemType>
inline typename ItemType::Value
getSimpleValueFromArray (const boost::shared_ptr<IProperty>& ip, size_t position)
{
	if (!isArray (ip))
		throw ElementBadTypeException ("getSimpleValueFromArray()");

	// Cast it to array
	boost::shared_ptr<CArray> array = IProperty::getSmartCObjectPtr<CArray> (ip);
	// Get value from array
	return getSimpleValueFromArray<ItemType> (array, position);
}

/** Get int from array. */
template<typename IP>
inline CInt::Value
getIntFromArray (const IP& ip, size_t position)
	{ return getSimpleValueFromArray<CInt> (ip, position);}

/** Get	double from array. */
template<typename IP>
inline CReal::Value
getDoubleFromArray (const IP& ip, size_t position)
{ 
	// Try getting int, if not successful try double
	try {
		
		return getIntFromArray (ip, position);
		
	}catch (ElementBadTypeException&) {}

	return getSimpleValueFromArray<CReal> (ip, position);
}

/** Get	string from array. */
template<typename IP>
inline CString::Value
getStringFromArray (const IP& ip, size_t position)
	{ return getSimpleValueFromArray<CString> (ip, position); }

/** Get	ref from array. */
template<typename IP>
inline CRef::Value
getRefFromArray (const IP& ip, size_t position)
	{ return getSimpleValueFromArray<CRef> (ip, position); }

//=========================================================
//	CArray "set value" helper methods
//=========================================================


/** 
 * Set simple value in array. 
 * If it is a reference, set fetch it and set it to the fetched object.
 *
 * @param array Array property.
 * @param position Position in the array.
 * @param val Value to be written.
 */
template<typename Value, typename ItemType, PropertyType ItemPType>
inline void
setSimpleValueInArray (const CArray& array, size_t position, const Value& val) DEPRECATED;

// gcc doesn't like __attribute__ in function definition
template<typename Value, typename ItemType, PropertyType ItemPType>
inline void
setSimpleValueInArray (const CArray& array, size_t position, const Value& val)
{
	utilsPrintDbg (debug::DBG_DBG, "array[" << position << "]");
	
	// Get the item and check if it is the correct type
	boost::shared_ptr<IProperty> ip = array.getProperty (position);
	// If it is a ref DELEGATE it
	ip = getReferencedObject (ip);
	
	// Cast it to the correct type and set value
	setValueToSimple<ItemType, ItemPType, Value> (ip, val);
}

/** Sets simple value to array element.
 * Uses setValueToSimple function to set property value.
 *
 * @param array Array property.
 * @param position Position of element to set.
 * @param val Simple value to set.
 */
template<typename ItemType>
inline void
setSimpleValueInArray (const CArray& array, size_t position, const typename ItemType::Value & val)
{
	utilsPrintDbg (debug::DBG_DBG, "array[" << position << "]");
	
	// Get the item and check if it is the correct type
	boost::shared_ptr<IProperty> ip = array.getProperty (position);
	// If it is a ref DELEGATE it
	ip = getReferencedObject (ip);
	
	// Cast it to the correct type and set value
	setValueToSimple<ItemType> (ip, val);
}

/** \copydoc setSimpleValueInArray */
template<typename Value, typename ItemType, PropertyType ItemPType>
inline void
setSimpleValueInArray (const IProperty& array, size_t position, const Value& val) DEPRECATED;
	
// gcc doesn't like __attribute__ in function definition
template<typename Value, typename ItemType, PropertyType ItemPType>
inline void
setSimpleValueInArray (const IProperty& ip, size_t position, const Value& val)
{
	assert (isArray (ip));
	if (!isArray (ip))
	{
		assert (!"Ip is not an array.");
		throw ElementBadTypeException ("");
	}

	// Cast it to array
	const CArray* array = dynamic_cast<const CArray*> (&ip);
	
	setSimpleValueInArray<Value, ItemType, ItemPType> (*array, position, val);
}

/** Sets simple value to array element.
 * Checks given property type and if it is pArray, casts it to CArray and calls
 * setSimpleValueInArray with CArray parameter.
 *
 * @param ip Array property.
 * @param position Position of element to set.
 * @param val Simple value to set.
 * @throw ElementBadTypeException if given property is not CArray instance.
 */
template<typename ItemType>
inline void
setSimpleValueInArray (const IProperty& ip, size_t position, const typename ItemType::Value& val)
{
	if (!isArray (ip))
		throw ElementBadTypeException ("");

	// Cast it to array
	const CArray* array = dynamic_cast<const CArray*> (&ip);
	
	setSimpleValueInArray<ItemType> (*array, position, val);
}

/** Set int in array. */
template<typename IP>
inline void
setIntInArray (const IP& ip, size_t position, int val)
	{ setSimpleValueInArray<CInt> (ip, position, val);}

/** Set	double in array. */
template<typename IP>
inline void
setDoubleInArray (const IP& ip, size_t position, double val)
{ 
	// Try setting double, if not successful try int
	try {
		
		return setSimpleValueInArray<CReal> (ip, position, val);
		
	}catch (ElementBadTypeException&) {}

	setIntInArray (ip, position, static_cast<int>(val));
}


//=========================================================
//	CArray "get type" helper methods
//=========================================================

/**
 * Get iproperty casted to specific type from array.
 *
 * Checks type of given property and if it pArray, casts it to CArray and uses
 * getTypeFromArray with CArray parameter.
 *
 * @param array Array.
 * @param id   Position in the array.
 * @throw ElementBadTypeException if given property is not an array or property
 * at given position doesn't have ItemType type.
 */
template<typename ItemType>
inline boost::shared_ptr<ItemType>
getTypeFromArray (const boost::shared_ptr<IProperty>& ip, size_t pos)
{
	if (!isArray (ip))
		throw ElementBadTypeException ("getTypeFromArray()");

	// Cast it to array
	boost::shared_ptr<CArray> array = IProperty::getSmartCObjectPtr<CArray> (ip);

	return array->getProperty<ItemType>(pos);
}

//=====================================================================================
} /* namespace utils */
//=====================================================================================
} /* namespace pdfobjects */
//=====================================================================================


#endif // _CARRAY_H
