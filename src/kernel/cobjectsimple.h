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
#ifndef _COBJECTSIMPLE_H
#define _COBJECTSIMPLE_H

// all basic includes
#include "kernel/static.h"
#include "kernel/iproperty.h"
#include <algorithm>


//=====================================================================================
namespace pdfobjects {
//=====================================================================================

/**
 * CObjectSimple type trait.
 *
 * This is an advance c++ template technique which allows us to store
 * additional information that identify variable type (e.g. function 
 * type specification of a function template -- method in a template 
 * class).
 * <br>
 * If someone tries to use unsupported type (e.g pCmd, etc.), she 
 * should get compile error because PropertyTraitSimple<> has no body.
 * <br>
 * REMARK: BE CAREFUL when manipulating these ones. Small change could 
 * resulting in an error could be very difficult to find.
 *
 * \see CDict, CArray, CObjectStream
 */
template<PropertyType T> struct PropertyTraitSimple; 
template<> struct PropertyTraitSimple<pNull>
{	public: typedef NullType value;
	public: typedef NullType writeType; 
};
template<> struct PropertyTraitSimple<pBool>
{	public: typedef bool value;
	public: typedef bool writeType; 
};
template<> struct PropertyTraitSimple<pInt>
{	public: typedef int value;
	public: typedef int writeType; 
};
template<> struct PropertyTraitSimple<pReal>
{	public: typedef double value;
	public: typedef double writeType; 
};
template<> struct PropertyTraitSimple<pString> 
{	public: typedef std::string value;
	public: typedef const std::string& writeType; 
};
template<> struct PropertyTraitSimple<pName>
{	public: typedef std::string value;
	public: typedef const std::string& writeType; 
};
template<> struct PropertyTraitSimple<pRef> 
{	public: typedef IndiRef value;
	public: typedef const IndiRef& writeType; 
};


//=====================================================================================
// CObjectSimple
//=====================================================================================

/** 
 * Template class representing simple PDF objects from pdf specification v1.5.
 *
 * This is a very generic class representing simple objects like null, string, number etc.
 * Simple types only store information like string, name, number, two numbers etc. 
 * 
 * Other xpdf objects like objCmd can not be instantiated although the PropertyType 
 * exists. It is because PropertyTraitSimple is not specialized for these types.
 *
 * This class does not provide default copy constructor because copying a
 * property could be understood either as deep copy or shallow copy. 
 * Copying complex types could be very expensive so we have made the decision to
 * avoid it.
 */
template <PropertyType Tp>
class CObjectSimple : noncopyable, public IProperty
{
public:
	typedef typename PropertyTraitSimple<Tp>::writeType WriteType;
	typedef typename PropertyTraitSimple<Tp>::value Value;
	typedef observer::BasicChangeContext<IProperty> BasicObserverContext;

	/** 
	 * Type of the property.
	 * Static field common to all simple objects with same type.
	 * This association allows us to get the PropertyType from object type.
	 */
	static const PropertyType type = Tp;
private:
	/** Simple value. */
	Value value;
	
	//
	// Constructors
	//
public:
	
	/**
	 * Constructor. 
	 * The object will read value from o and store it. We do NOT save any reference to o.
	 *
	 * @param p Pointer to pdf object.
	 * @param o Xpdf object. 
	 * @param rf Indirect id and gen id.
	 */
	CObjectSimple (boost::weak_ptr<CPdf> p, const Object& o, const IndiRef& rf);
	
	/**
	 * Constructor.
	 * The object will read value from o and store it. We do NOT save any reference to o.
	 *
	 * @param o Xpdf object. 
	 */
	CObjectSimple (const Object& o);
public:	

	/**
	 * Public constructor. Can be used to create pdf objects. This object is created
	 * as a standalone and does not belong to any pdf. When added to a pdf
	 * the relation will be saved to IProperty::pdf.
	 *
	 * @param val Value that will this object hold.
	 */
	CObjectSimple (const Value& val = Value());


	//
	// Cloning
	//
protected:

	/**
	 * Clone. 
	 *
	 * Performs deep copy.
	 * REMARK: CRef does NOT copy the referenced object just itself.
	 *
	 * @return Deep copy of this object.
	 */
	virtual IProperty* doClone () const;


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
	 * @param str Output string that will hold the string representation 
	 * of current object.
	 */
	virtual void getStringRepresentation (std::string& str) const;
	
	/**
	 * Return property value. Each property has its own return type.
	 *
	 * @param val Out parameter where property value will be stored.
	 */
	void getValue (Value& val) const;

	/**
	 * Return property value. Each property has its own return type.
	 *
	 * @return Property value.
	 */
	Value getValue () const;

	//
	// Set methods
	//
public:
	/**
	 * Convert string to an object value.
	 * \exception ObjBadValueE Thrown when we can't parse the string correctly.
	 *
	 * @param strO String object that will be parsed and saved to this object's value.
	 */
	void setStringRepresentation (const std::string& strO);

	
	/**
	 * Change the value of an object. The variable type depends
	 * on CObject type.
	 * 
	 * We can define the best type to represent an pdf object in
	 * PropertyTraitSimple.
	 *
	 * \see PropertyTraitSimple
	 *
	 * @param val Value that will be set.
	 */
	void setValue (WriteType val);

	//
	// Destructor
	//
public:
	/**
	 * Destructor
	 */
	~CObjectSimple () {}
	

	//
	// Helper methods
	//
public:	

	/**
	 * Make xpdf Object from this object. This function allocates and initializes xpdf object.
	 * Caller has to free the xpdf Object (call Object::free and then
	 * deallocating)
	 *
	 * \exception ObjBadValueE Thrown when xpdf can't parse the string representation of this
	 * object correctly.
	 * 
	 * @return Xpdf object representing value of this simple object.
	 */
	virtual ::Object* _makeXpdfObject () const;
	
private:

	/**
	 * Create context of a change.
	 *
	 * REMARK: Be carefull. Deallocate this object.
	 * 
	 * @return Context in which a change occured.
	 */
	ObserverContext* _createContext () const
	{
		// Save original value for the context
		boost::shared_ptr<IProperty> oldValue (this->clone());
		// Set original values
		oldValue->setPdf (this->getPdf());
		oldValue->setIndiRef (this->getIndiRef());
		// Create the context
		return new BasicObserverContext (oldValue);
	}

	/**
	 * Indicate that the object has changed.
	 * Notifies all observers associated with this property about the change.
	 *
	 * @param context Context in which a change occured.
	 */
	void _objectChanged (boost::shared_ptr<const ObserverContext> context);
};


//
// Typedefs
//
typedef CObjectSimple<pNull> CNull;
typedef CObjectSimple<pBool> CBool;
typedef CObjectSimple<pInt> CInt;
typedef CObjectSimple<pReal> CReal;
typedef CObjectSimple<pString> CString;
typedef CObjectSimple<pName> CName;
typedef CObjectSimple<pRef> CRef;


//=====================================================================================
/** Namespace includes all helper functions and classes.*/
namespace utils {
//=====================================================================================

//=========================================================
// Validate functions
//=========================================================

/**
 * Make name valid.
 * 
 * Beginning with PDF 1.2, any character except null (character code 0) may be included
 * in a name by writing its 2-digit hexadecimal code, preceded by the number
 * sign character (#); see implementation notes 3 and 4 in Appendix H. This
 * syntax is required to represent any of the delimiter or white-space characters or
 * the number sign character itself; it is recommended but not required for characters
 * whose codes are outside the range 33 (!) to 126 (~). The examples shown in
 * Table 3.3 are valid literal names in PDF 1.2 and later.
 * @param it Start iterator.
 * @param end End iterator.
 */
template<typename Iter>
std::string 
makeNamePdfValid (Iter it, Iter end)
{
	typedef std::string::value_type Item;
	std::string tmp;
	for (; it != end; ++it)
	{
		unsigned ch = *it;

		if (ch < sizeof(specialChars) && specialChars[*it])
		{ // Convert it to ascii
			char hexstr[4];
			snprintf(hexstr, sizeof(hexstr), "#%02x", ch & 0xff);
			assert(hexstr[sizeof(hexstr)-1] == '\0');
			tmp += hexstr;
		}else
			tmp += ch;
	}
	
	return tmp;
}

/**
 * Make name valid.
 * 
 * Beginning with PDF 1.2, any character except null (character code 0) may be included
 * in a name by writing its 2-digit hexadecimal code, preceded by the number
 * sign character (#); see implementation notes 3 and 4 in Appendix H. This
 * syntax is required to represent any of the delimiter or white-space characters or
 * the number sign character itself; it is recommended but not required for characters
 * whose codes are outside the range 33 (!) to 126 (~). The examples shown in
 * Table 3.3 are valid literal names in PDF 1.2 and later.
 *
 * @param str Character string.
 */
inline std::string 
makeNamePdfValid (const char * str)
{ 
	return makeNamePdfValid (str, str + strlen(str)); 
}

/**
 * Make string pdf valid.
 *
 * Any characters may appear in a string except unbalanced parentheses and
 * the backslash, which must be treated specially.
 * <br>
 * This includes also \0 (NUL) character, therefore we can't use standard
 * str* functions for strings.
 *
 * @param str String to escape.
 */
inline std::string
makeStringPdfValid (const std::string &str)
{
        typedef std::string::const_reverse_iterator Iter;
        std::string tmp;
        bool need_escape=false;
        for (Iter it=str.rbegin(); it != str.rend(); ++it)
        {
                if ( '\\' == (*it))
                        need_escape ^= true;
                else if(need_escape)
                {
                        // escape the previously processed character
                        tmp+='\\';
                        need_escape = false;
                }
                if (!need_escape && ('(' == (*it) || ')' == (*it)))
                        need_escape = true;
                tmp += *it;
        }
        if(need_escape)
                tmp+='\\';

        // we have processed the string from the end so we have to
        // reverse it now (we could insert the character always to the
        // front but that would require N^2 moves so the reverse is more
        // optimal)
        reverse(tmp.begin(), tmp.end());
        return tmp;
}

/**
 * Make string pdf valid.
 *
 * Any characters may appear in a string except unbalanced parentheses and
 * the backslash, which must be treated specially.
 *
 * @param str Character string.
 */
inline std::string 
makeStringPdfValid (GString * str)
{ 
	char * string = str->getCString();
	return makeStringPdfValid (string); 
}

//=========================================================
// Creation functions
//=========================================================

/**
 * Creates CObject* from xpdf object.
 *
 * @param pdf Pdf in which the created object will live. Parameter to CObject* constructor.
 * @param obj Xpdf object from which the object will be created. Parameter to CObject* constructor.
 * @param ref Indirect reference number of this object (or its parent).
 *
 * @return Pointer to newly created object.
 */
IProperty* createObjFromXpdfObj (boost::shared_ptr<CPdf> pdf, const ::Object& obj,const IndiRef& ref);

/**
 * Creates CObject* from xpdf object.
 *
 * @param obj Xpdf object from which the object will be created. Parameter to CObject* constructor.
 *
 * @return Pointer to newly created object.
 */
IProperty* createObjFromXpdfObj (const ::Object& obj);

/**
 * Save real xpdf object value to val.
 * 
 * @param obj Xpdf object which holds the value.
 * @param val Variable where the value will be stored.
 */
template <PropertyType Tp,typename T> void simpleValueFromXpdfObj (const ::Object& obj, T val);

/**
 * Create xpdf Object which represents value.
 * 
 * @param val Value where the value is stored.
 * @return Xpdf object where the value is stored.
 */
template <PropertyType Tp,typename T> Object* simpleValueToXpdfObj (T val);

/**
 * Create xpdf object from string.
 *
 * @param str String that should represent an xpdf object.
 * @param xref Xref of actual pdf if any.
 *
 * @return Xpdf object whose string representation is in str.
 */
::Object* xpdfObjFromString (const std::string& str, ::XRef* xref = NULL);

/**
 * Parses string to get simple values like int, name, bool etc.
 * 
 * @param str String to be parsed.
 * @param val Desired value.
 *
 * @throw ObjBadValueE Thrown when the string, can't be parsed correctly.
 */
void simpleValueFromString (const std::string& str, bool& val);
void simpleValueFromString (const std::string& str, int& val);
void simpleValueFromString (const std::string& str, double& val);
void simpleValueFromString (const std::string& str, std::string& val);
void simpleValueFromString (const std::string& str, IndiRef& val);


//=========================================================
// To string functions
//=========================================================

/**
 * Return simple xpdf object (null,number,string...) in string representation.
 * 
 * REMARK: String can represent more different objects, so we have to distinguish among them.
 * This is done at compile time with use of templates, but because of this we have to
 * make other functions also template.
 *
 * @param val that will be converted to string.
 * @param str string.
 */
template <PropertyType Tp> void simpleValueToString (bool val, std::string& str);

/** \copydoc simpleValueToString */
template <PropertyType Tp> void simpleValueToString (int val,std::string& str);

/** \copydoc simpleValueToString */
template <PropertyType Tp> void simpleValueToString (double val,std::string& str);

/** \copydoc simpleValueToString */
template <PropertyType Tp> void simpleValueToString (const std::string& val,std::string& str);

/** \copydoc simpleValueToString */
template <PropertyType Tp> void simpleValueToString (const NullType& val,std::string& str);

/** \copydoc simpleValueToString */
template <PropertyType Tp> void simpleValueToString (const IndiRef& val,std::string& str);

//=========================================================
// From string functions
//=========================================================

/**
 * Create text representation of an indirect object from string and IndiRef.
 *
 * @param rf IndiRef.
 * @param val Value of an object.
 * @param output Output string.
 */
 void createIndirectObjectStringFromString (const IndiRef& rf, const std::string& val, std::string& output);

 

//=========================================================
// Other functions
//=========================================================

/**
 * Returns true if object has a parent.
 *
 * \exception ObjInvalidOperation Thrown when the object does not belong
 * to a valid pdf.
 *
 * @param ip IProperty of object. 
 */
bool objHasParent (const IProperty& ip);

/**
 * Returns true if object has a parent.
 *
 * \exception ObjInvalidOperation Thrown when the object does not belong
 * to a valid pdf.
 *
 * @param ip IProperty of object. 
 * @param indiObj Out parameter wheren indirect object will be saved, because
 * finding the object can be an expensive operation.
 */
bool objHasParent (const IProperty& ip, boost::shared_ptr<IProperty>& indiObj);


//=========================================================
// CObject* "delegate" helper methods
//=========================================================

/**
 * If the object is a reference, fetch the "real" object
 *
 * @param ip IProperty.
 */
boost::shared_ptr<IProperty> getReferencedObject (boost::shared_ptr<IProperty> ip);


//=========================================================
// CObjectSimple "get value" helper methods
//=========================================================

/**
 * Get simple value from simple cobject.
 *
 * @param ip IProperty.
 *
 * @return Value.
 */
template<typename ItemType, PropertyType ItemPType, typename Value>
inline Value
getValueFromSimple (const boost::shared_ptr<IProperty>& ip) DEPRECATED;

// function definition (gcc doesn't like __attribute__ in function definition
template<typename ItemType, PropertyType ItemPType, typename Value>
inline Value
getValueFromSimple (const boost::shared_ptr<IProperty>& ip)
{
	if (ItemPType == ip->getType ())
	{
		// Cast it to the correct type and return it
		boost::shared_ptr<ItemType> item = IProperty::getSmartCObjectPtr<ItemType> (ip);
		return item->getValue ();

	}else
	{
		utilsPrintDbg (debug::DBG_DBG, "wanted type " << ItemPType << " got " << ip->getType ());
		throw ElementBadTypeException ("getValueFromSimple");
	}
}

/**
 * Get simple value from simple cobject.
 *
 * Given property must have correct type (ItemType::type).
 *
 * @param ip IProperty.
 *
 * @return Value.
 */
template<typename ItemType>
inline typename ItemType::Value
getValueFromSimple (const boost::shared_ptr<IProperty>& ip)
{
	if (ItemType::type == ip->getType ())
	{
		// Cast it to the correct type and return it
		boost::shared_ptr<ItemType> item = IProperty::getSmartCObjectPtr<ItemType> (ip);
		return item->getValue ();

	}else
	{
		utilsPrintDbg (debug::DBG_DBG, "wanted type " << ItemType::type << " got " << ip->getType ());
		throw ElementBadTypeException ("getValueFromSimple");
	}
}

/** Get int from ip. */
inline int 
getIntFromIProperty (const boost::shared_ptr<IProperty>& ip)
{
	return getValueFromSimple<CInt> (ip);
}

/** Get double from ip. */
inline double 
getDoubleFromIProperty (const boost::shared_ptr<IProperty>& ip)
{
	return (isInt (ip)) ? getValueFromSimple<CInt> (ip) : 
		getValueFromSimple<CReal> (ip);
}

/** Get string from ip. */
inline std::string
getStringFromIProperty (const boost::shared_ptr<IProperty>& ip)
{
	return getValueFromSimple<CString> (ip);
}

/** Get name from ip. */
inline std::string
getNameFromIProperty (const boost::shared_ptr<IProperty>& ip)
{
	return getValueFromSimple<CName> (ip);
}

//=========================================================
// CObjectSimple "set value" helper methods
//=========================================================

/**
 * Set simple value.
 *
 * @param ip IProperty.
 * @param val Value to set.
 */
template<typename ItemType, PropertyType ItemPType, typename Value>
inline void
setValueToSimple (const boost::shared_ptr<IProperty>& ip, const Value& val)
{
	if (ItemPType != ip->getType ())
	{
		utilsPrintDbg (debug::DBG_DBG, "wanted type " << ItemPType << " got " << ip->getType ());
		throw ElementBadTypeException ("");
	}

	// Cast it to the correct type and set value
	boost::shared_ptr<ItemType> item = IProperty::getSmartCObjectPtr<ItemType> (ip);
	item->setValue (val);
}



//=====================================================================================
} /* namespace utils */
//=====================================================================================
} /* namespace pdfobjects */
//=====================================================================================




//
// Include the actual implementation of CObject class
//
#include "kernel/cobjectsimpleI.h"


#endif // _COBJECTSIMPLE_H

