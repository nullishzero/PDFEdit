/** 
 * Header file containing definition of IProperty and CObject classes.
 * Interface for GUI/...
 *
 * jmisutka 06/01/18
 */

#ifndef COBJECT_H
#define COBJECT_H

#include <vector>
#include <list>
#include <map>
#include <string>

//xpdf


//IProperty
#include "debug.h"
#include "iproperty.h"
//#include "cxref.h"


//=====================================================================================
namespace pdfobjects
{

using namespace std;

/**
 * Additional information that identifies variable type for writeValue function
 *
 * If someone tries to use unsupproted type (pCmd,....), she should get compile error
 * because PropertyTrait<> has no body.
 */
template<PropertyType T> class PropertyTrait; 
template<> class PropertyTrait<pNull>	{public: typedef NullType 		writeType; typedef const NullType 	PropertyId;};
template<> class PropertyTrait<pBool>	{public: typedef const bool		writeType; typedef const NullType 	PropertyId;};
template<> class PropertyTrait<pInt>	{public: typedef const int		writeType; typedef const NullType 	PropertyId;};
template<> class PropertyTrait<pReal>	{public: typedef const double 	writeType; typedef const NullType 	PropertyId;};
template<> class PropertyTrait<pString> {public: typedef const string 	writeType; typedef const NullType 	PropertyId;};
template<> class PropertyTrait<pName>	{public: typedef const string 	writeType; typedef const NullType 	PropertyId;};
template<> class PropertyTrait<pArray>	{public: typedef const string 	writeType; typedef const unsigned int 	PropertyId;};
template<> class PropertyTrait<pStream> {public: typedef const string 	writeType; typedef const NullType 	PropertyId;};
template<> class PropertyTrait<pDict>	{public: typedef const string 	writeType; typedef const string 	PropertyId;};
template<> class PropertyTrait<pRef> 	{public: typedef const string	writeType; typedef const NullType	PropertyId;};


/**
 * Typedefs
 */
typedef unsigned int	PropertyCount;
typedef unsigned int	PropertyIndex;
typedef string			PropertyName;


/** 
 * Template class representing all PDF objects from specification v1.5.
 *
 * This dividing is crucial when making specific changes to specific objects
 * e.g. adding/deleting of properties can be done to Dictionary but not to Integer.
 *
 * The specific features are implemented using c++ feature called Incomplete Instantiation.
 * It means that, when it is not used, it is not instatiated, so e.g. CInt won't have
 * addDictProperty() method
 *
 * This class can be both, a final class (no child objects) or not final class 
 * (has child object). 
 *
 * When it is not final class, it is a special object (CPdf, CPage,...). We can
 * find out the object by calling  virtual method getSpecialObjType(). 
 * This can be helpful for example for special manipulation with content stream, xobjects, ...
 *
 * REMARK: it will be the responsibility of CXref to remember a mapping
 * between each xpdf Object and our representation. e.g. when we want to save
 * a pdf, we also want to save undo's, and if we would have more of our objects 
 * pointing to one xpdf Object we would have multiple undos and it could cause 
 * problems.
 */
template <PropertyType Tp>
class CObject : public IId, public IProperty
{

private:
	SpecialObjectType specialObjectType;	/*< Type indicating whether this object is a special object, like Cpdf, CAnnotation...*/
//	CXref*	xref;							/*< Xref of pdf */	
	
public:
	/**
	 * Constructors. If no object is given, one is created.
	 *
	 * @param xr	our xref
	 * @param objTp	Type of this object, whether it is a special object (CPdf,CPage,..) or not 
	 */
	CObject (/*CXref* xr,*/SpecialObjectType objTp = sNone);
	CObject (Object& o);

	/**
	 * Returns string representation of (x)pdf object.
	 *
	 * @return String representing (x)pdf object.
	 */
	string& getStringRepresentation () const;
	
	/**
	 * Make object from string.
	 *
	 * @param txt object in string form
	 */
	void setStringRepresentation (const string& strO);

	/**
	 * We can define how best we want to represent an pdf object. E.g.
	 * we can represent a dictionary with list<pair<string,string> > etc...
	 */
	void writeValue (typename PropertyTrait<Tp>::writeType* val) {};
  
	/**
 	 * Notify Writer object that this object has changed. We have to call this
	 * function to make changes visible.
	 *
	 * It is necessary for saving previous state of the object.
	 *
	 * Reference to CXref is stored in CObject::xref.
	 *
	 * @param makeValidCheck True if we want to verify that our changes preserve
	 * 						 pdf validity.
	 */
	virtual void dispatchChange (bool makeValidCheck) const {}; 

	/**
	 * Returns xpdf object.
	 *
	 * @return Xpdf object.
	 */
	const Object* getRawObject () const {return obj;};
	/**
	 * Returns if it is one of special objects CPdf,CPage etc.
	 *
	 * @return Type of special object.
	 */
	virtual SpecialObjectType getSpecialObjType() const {return (SpecialObjectType)0;};

	/**
	 * Returns pointer to derived object. 
	 */
	template<typename T>
	T* getSpecialObjectPtr () const 
	{
			return dynamic_cast<T*>(this);
	}
  

    /**
     * Notify observers that a property has changed.
     */
    virtual void notifyObservers () {};
 
	 
	/**
	 * Destructor
	 */
	~CObject ()	{};


	//
	// Specific features by Incomplete Instantiation
	//

	/** 
     * Returns property count.
     *
     * @return Property count.
     */
    PropertyCount getPropertyCount ();
 

	/**
     * Inserts all property names to container supplied by caller. This container must support 
     * push_back() function.
     *
     * @param container Container of string objects. STL vector,list,deque,... must implement 
     * pust_back() function.
     */
    template <typename T>
    void getAllPropertyNames (T& container) const {};


	/**
     * Returns value of property identified by its name/position depending on type of this object.
	 *
	 * So f.e. when we want to access dicionary item, we use string representation, when array
	 * we use integer.
   	 *
     * @param name Name of the property
     */
    template <typename T>
    T getPropertyValue (typename PropertyTrait<Tp>::PropertyId& name) const;

	
	/**
     * Returns property type of an item.
     *
     * @param	name	Name of the property.
     * @return		Property type.	
     */
    PropertyType getPropertyType (const PropertyName& name) const
    {
  	   	//return static_cast<PropertyType>(obj->getType());
    	return pNull;
    }

	/**
	 * Sets property type of an item.
	 *
	 * @param	id		Name/Index of property
	 * @param	value	Value, for simple types (int,string,...) and for complex types IProperty*
	 */
	template<typename T>
	void setPropertyValue (typename PropertyTrait<Tp>::PropertyId& /*name*/, const T& /*val*/)
	{
		//
		// Find the name in Object obj dictionary and change the value (type) of simple Object
		// simple conversions from int to string etc. can be done here
		//
		// If we want to set complex type we just get the Object from IProperty passed to this function
		// and save the pointer
		//
		// All operations are directly written to xpdf Objects
		//
	};

	/**
	 * Adds property to array/dict.
	 *
	 * @param id Name/Index of property
	 */
	template<typename T>
	void addProperty (typename PropertyTrait<Tp>::PropertyId /*id*/, const T& /*val*/ ) {};
	
	/**
	 * Removes property from array/dict.
	 * REMARK: This can be tricky when having a complex type as a value.
	 *
	 * @param id Name/Index of property
	 */
	void delProperty (typename PropertyTrait<Tp>::PropertyId /*id*/) {};


private:
	void createObj () const;

};



//
// CObject types
//
typedef CObject<pNull>	 CNull;
typedef CObject<pBool>	 CBoolean;
typedef CObject<pInt>	 CInt;
typedef CObject<pReal>	 CReal;
typedef CObject<pString> CString;
typedef CObject<pStream> CStream;
typedef CObject<pArray>	 CArray;
typedef CObject<pDict>	 CDict;
typedef CObject<pRef>	 CRef;



//
// CObject types
//
typedef CObject<pNull>	 CNull;
typedef CObject<pBool>	 CBoolean;
typedef CObject<pInt>	 CInt;
typedef CObject<pReal>	 CReal;
typedef CObject<pString> CString;
typedef CObject<pStream> CStream;
typedef CObject<pArray>	 CArray;
typedef CObject<pDict>	 CDict;
typedef CObject<pRef>	 CRef;


//=====================================================================================

//
// Implementation of some member functions
//


template<PropertyType Tp>
CObject<Tp>::CObject (/*CXref* xr,*/SpecialObjectType objTp) : specialObjectType(objTp)//, xref (xr) 
{
	STATIC_CHECK (pOther != Tp,COBJECT_BAD_TYPE);
	STATIC_CHECK (pOther1 != Tp,COBJECT_BAD_TYPE);
	STATIC_CHECK (pOther2 != Tp,COBJECT_BAD_TYPE);
	STATIC_CHECK (pOther3 != Tp,COBJECT_BAD_TYPE);
	//assert (NULL != xr);
	printDbg (0,"CObject constructor.");
		
	IProperty::obj = new Object ();
	assert (NULL != obj);
	
	// Init object with "default values" according to type
	switch (Tp)
	{
		case pNull:
				obj = obj->initNull ();
				break;
		case pBool:
				obj = obj->initBool (false);
				break;
		case pInt:
				obj = obj->initInt (-1);
				break;
		case pReal:
				obj = obj->initReal (-1);
				break;
		case pString:
				// Empty string
				obj = obj->initString (new GString);
				break;
		case pName:
				obj = obj->initName ("");
				break;
		case pArray:
//				obj = obj->initArray (xref);
				break;
		case pDict:
//				obj = obj->initDict (xref);
				break;
		case pStream:
//				obj = obj->initStream (new MemStream(...));
				break;
		case pRef:
				obj = obj->initRef (-1,-1);
				break;
		default:
				// should not happen
				assert (false);
				break;
	}

};


//	: IId(o.,o.), IProperty (o) {};


//
// Better 
//
/*template<PropertyType Tp>
string& CObject<Tp>::getStringRepresentation () const
{
	//
	// We better do it ourselves, because we can produce 
	// errors during parsing and getting buffer from file
	// structure is 
	//
 Object.cc
  Object obj;
  int i;

  switch (type) {
  case objBool:
    fprintf(f, "%s", booln ? "true" : "false");
    break;
  case objInt:
    fprintf(f, "%d", intg);
    break;
  case objReal:
    fprintf(f, "%g", real);
    break;
  case objString:
    fprintf(f, "(");
    fwrite(string->getCString(), 1, string->getLength(), stdout);
    fprintf(f, ")");
    break;
  case objName:
    fprintf(f, "/%s", name);
    break;
  case objNull:
    fprintf(f, "null");
    break;
  case objArray:
    fprintf(f, "[");
    for (i = 0; i < arrayGetLength(); ++i) {
      if (i > 0)
	fprintf(f, " ");
      arrayGetNF(i, &obj);
      obj.print(f);
      obj.free();
    }
    fprintf(f, "]");
    break;
  case objDict:
    fprintf(f, "<<");
    for (i = 0; i < dictGetLength(); ++i) {
      fprintf(f, " /%s ", dictGetKey(i));
      dictGetValNF(i, &obj);
      obj.print(f);
      obj.free();
    }
    fprintf(f, " >>");
    break;
  case objStream:
    fprintf(f, "<stream>");
    break;
  case objRef:
    fprintf(f, "%d %d R", ref.num, ref.gen);
    break;
  case objCmd:
    fprintf(f, "%s", cmd);
    break;
  case objError:
    fprintf(f, "<error>");
    break;
  case objEOF:
    fprintf(f, "<EOF>");
    break;
  case objNone:
    fprintf(f, "<none>");
    break;
  }
};
*/



}; /* namespace pdfobjects */



#endif // COBJECT_H

