/** 
 * =====================================================================================
 *        Filename:  cobject.h
 *     Description:  Header file containing definition of IProperty and CObject classes.
 *         Created:  01/18/2006 
 *        Revision:  none
 *          Author:  jmisutka (06/01/19), 
 * 			
 * 			2006/01     constructor, getStringRepresentation, observer stuff, writeValue,
 *			2006/02/04	started implementing release () function (not trivial)
 *						implemented several easy functions for complex types
 *			2006/02/08  for the past several days i have had a battle with g++ and ls + collect
 *						i think that i have won at last...
 *			2006/02/20	changed constructor, addProperty has to be more clever
 *						getStringRepresentation,writeValue,release finished
 *			2006/02/21	changed getPropertyValue to non - template, bacause we can't partially specialize member functions
 *					also the way of passing return from return value to function argument, because
 *						* do not want to alloc something in kernel and dealloc elsewhere
 *						* pass by stack copying can be tricky when copying big amounts and we do not know 
 *						  in pdf
 *			2006/02/23	cobjectsimple implementation finished
 *			2006/02/23	cobjectcomplex 
 *					- getStringRepresentation
 *					- writeValue
 *					- release?, getPropertyCount
 *					- getAllPropertyNames -- problem templates virtual
 *			2006/02/24	- implementing addProperty for
 *					IProperty*. (template f. can't be virtual)
 *					all other functions, problems witch Object internal structure
 *					  not public functions etc., etc., etc.
 *					- several questions has arisen, that i cannot decide by myself
 *					- delProperty
 *
 * ------------ 2006/03/05 21:42 ----------- FUCK xpdf
 *  				- remove use of XPDF as value holders, question is Stream, but better 1 bad class
 *  				than 7.
 *  				03:24 -- CObjectSimple implemented withou XPDF
 *  				23:11 -- CObjectComplex 1st version
 *
 * 			2006/03/06
 * 					added Memory checker and finishing implementation of cobjects, waiting for cpdf to be
 * 						compilable
 * 					CObjectSimple finished
 * 					CObjectComplex finished except setStringRepresentation and writeValue (which is actually the same)
 * 
 *			TODO:
 *					testing
 *					!! function for getting unused Ref in CPdf
 *					can't add IProperty when pdf is not NULL (just to simplify things)
 *					   no real obstruction for removing this
 *					better public/protected dividing
 *
 * =====================================================================================
 */
#ifndef COBJECT_H
#define COBJECT_H

#include "static.h"

#include "iproperty.h"



//=====================================================================================
namespace pdfobjects
{

		
//
// Forward declarations of memory checkers
//

class NoMemChecker;
class BasicMemChecker;
		
/** Operations we can check. */
enum _eOperations { OPER_CREATE, OPER_DELETE };


	
/**
 * Additional information that identifies variable type, e.g. for writeValue function.
 *
 * If someone tries to use unsupported type (pCmd,....), she should get compile error
 * because PropertyTraitSimple<> has no body.
 *
 * REMARK: BE CAREFUL when manipulating these ones.
 */
template<PropertyType T> struct PropertyTraitSimple; 
template<> struct PropertyTraitSimple<pNull>
{	public: typedef NullType		value;
	public: typedef NullType		writeType; 
};
template<> struct PropertyTraitSimple<pBool>
{	public: typedef bool			value;
	public: typedef bool			writeType; 
};
template<> struct PropertyTraitSimple<pInt>	
{	public: typedef int				value;
	public: typedef int				writeType; 
};
template<> struct PropertyTraitSimple<pReal>	
{	public: typedef double			value;
	public: typedef double 			writeType; 
};
template<> struct PropertyTraitSimple<pString> 
{	public: typedef std::string			value;
	public: typedef const std::string& 	writeType; 
};
template<> struct PropertyTraitSimple<pName>	
{	public: typedef std::string			value;
	public: typedef const std::string& 	writeType; 
};
template<> struct PropertyTraitSimple<pRef> 	
{	public: typedef IndiRef				value;
	public: typedef const IndiRef&	 	writeType; 
};


//=====================================================================================
// CObjectSimple
//

/** 
 * Template class representing simple PDF objects from specification v1.5.
 *
 * This class represents simple objects like null, string, number etc.
 * It does not have special functions like CObjectComplex.
 *
 * Other xpdf objects like objCmd can't be instantiated although the PropertyType 
 * exists. It is because PropertyTraitSimple is not specified for these types.
 *
 * We can use memory checking with this class which save information about living IProperties.
 * Can be used to detect memory leaks etc.
 *
 */
template <PropertyType Tp, typename Checker = BasicMemChecker>
class CObjectSimple : public IProperty
{

public:
	/** Write type for writeValue function. */
	typedef typename PropertyTraitSimple<Tp>::writeType	 WriteType;
	/** Value holder. */
	typedef typename PropertyTraitSimple<Tp>::value 	 Value;  
			
private:
	/** Object's value. */
	Value value;
	

private:
	
	/**
	 * Copy constructor
	 */
	CObjectSimple (const CObjectSimple&);
	
	
public/*protected*/:
	
	/**
	 * Constructor. Only kernel can call this constructor. It depends on the object, that we have
	 * parsed.
	 *
	 * @param p		Pointer to pdf object.
	 * @param o		Xpdf object. 
	 * @param rf	Indirect id and gen id.
	 */
	CObjectSimple (CPdf& p, Object& o, const IndiRef& rf);


public:	

	/**
	 * Public constructor. Can be used to create direct/indirect objects.
	 *
	 * @param p		Pointer to pdf object in which this object will exist.
	 */
	CObjectSimple (CPdf& p);

	/**
	 * Public constructor. Can be used to create direct/indirect objects.
	 *
	 * @param p		Pointer to pdf object in which this object will exist.
	 * @param val	Value that will this object hold.
	 */
	CObjectSimple (CPdf& p, const Value& val);


#ifdef DEBUG
CObjectSimple () : value(Value()) {Checker check (this, OPER_CREATE);};
#endif
	
	/**
	 * Return type of this property.
	 *
	 * @return Type of this property.
	 */
	virtual PropertyType getType () const {return Tp;};
			

	/**
	 * Returns string representation of (x)pdf object. 
	 * 
	 * @param str 	After successful call, it will hold the string representation 
	 * 				of current object.
	 */
	virtual void getStringRepresentation (std::string& str) const;
	
	
	/**
	 * Make object from string.
	 * <exception cref="..." />
	 *
	 * @param str0 Object in a text form.
	 */
	void setStringRepresentation (const std::string& strO);

	
	/**
	 * Change the value of an object. The variable type depends
	 * on CObject type.
	 * 
	 * We can define the best to represent an pdf object.
	 *
 	 * <a cref="ObjBadValueE" /> Thrown when the string, can't be parsed correctly.
	 *
	 * @param val	Value that will be set.
	 */
	void writeValue (WriteType val);
 

	/**
	 * Return property value. Each property has its own return type.
	 *
	 * @param val Out parameter where property value will be stored.
	 */
	void getPropertyValue (Value& val) const;
	
	/**
	 * Indicate that you do not want to use this object again.
	 * 
	 * If it is an indirect object, we have to notify CXref.
	 */
  	virtual void release ();
	

//protected: 
/*DEBUG*/public:	
	/**
	 * Destructor
	 */
	~CObjectSimple () {Checker check (this, OPER_DELETE);};
	
protected:
	/**
 	 * Notify Writer object that this object has changed. We have to call this
	 * function to make changes visible.
	 *
	 * It is necessary for undo operation, that means saving previous state of the object.
	 * We obtain reference to CXref from CObject::pdf.
	 */
	void dispatchChange () const; 


	//
	// Helper functions
	//
protected:
	
	/**
	 * Make xpdf Object from this object. This function allocates xpdf object, caller has to free it.
	 *
	 * @return Xpdf object representing actual value of this simple object.
	 */
	virtual Object*	_makeXpdfObject () const;
	
private:
	
	/**
	 * Finds out if this object is indirect.
	 *
	 * @return true if this object is indirect, false otherwise.
	 */
	bool _isIndirect () const;

	/**
	 * Indicate that the object has changed.
	 */
	inline void
	_objectChanged ()
	{
		// Dispatch the change
		dispatchChange ();
		
		// Notify everybody about this change
		IProperty::notifyObservers ();
	}

};




//=====================================================================================
// CObjectComplex
//


//
// Forward declaration of element finders
//
class ArrayIdxComparator;
class DictIdxComparator;


/**
 * Additional information that identifies variable type, e.g. for writeValue function.
 *
 * If someone tries to use unsupported type (pCmd,....), she should get compile error
 * because PropertyTrait<> has no body.
 */
template<PropertyType T> struct PropertyTraitComplex; 
template<> struct PropertyTraitComplex<pArray>	
{	public: 
		typedef std::vector<IProperty*>		value; 
		typedef const std::string& 			writeType; 
		typedef unsigned int	 			propertyId;
		typedef class ArrayIdxComparator	indexComparator;
};
template<> struct PropertyTraitComplex<pStream> 
{	public: 
		typedef std::list<std::pair<std::string,IProperty*> >	value; 
		typedef const std::string& 			writeType; 
		typedef const std::string& 			propertyId;
		typedef class DictIdxComparator		indexComparator;
};
template<> struct PropertyTraitComplex<pDict>	
{	public: 
		typedef std::list<std::pair<std::string,IProperty*> >	value; 
		typedef const std::string& 			writeType; 
		typedef const std::string& 			propertyId;
		typedef class DictIdxComparator		indexComparator;
};



/** 
 * Template class representing complex PDF objects from specification v1.5.
 *
 * The specific features are implemented using c++ feature called Incomplete Instantiation.
 * It means that, when it is not used, it is not instatiated, so e.g. CArray won't have
 * addProperty (IProperty& newIp, const std::string& propertyName) method.
 *
 * This class can be both, a final class (no child objects) or a parent to a special class.
 *
 * When it is not a final class, it is a special object (CPdf, CPage,...). We can
 * find this out by calling virtual method getSpecialObjType(). 
 * This can be helpful for example for special manipulation with content stream, xobjects, ...
 */
template <PropertyType Tp, typename Checker = BasicMemChecker>
class CObjectComplex : public IProperty
{
public:
	/** Write type for writeValue function. */
	typedef typename PropertyTraitComplex<Tp>::writeType  		WriteType;
	/** This type identifies a property. */
	typedef typename PropertyTraitComplex<Tp>::propertyId 		PropertyId;
	/** This functor can find an item in the value holder. */
	typedef typename PropertyTraitComplex<Tp>::indexComparator	IndexComparator;
	/** Value holder. */
	typedef typename PropertyTraitComplex<Tp>::value 	  		Value;  

private:
	/** Object's value. */
	Value value;

private:
	/**
	 * Copy constructor
	 */
	CObjectComplex (const CObjectComplex&) {};
	

protected:
	/**
	 * Pdf constructor.
	 */
	CObjectComplex ();

/*debug*/public:
	/**
	 * Constructor. Only kernel can call this constructor
	 *
	 * @param p		Pointer to pdf object.
	 * @param o		Xpdf object. 
	 * @param rf	Indirect id and gen id.
	 */
	CObjectComplex (CPdf& p, Object& o, const IndiRef& rf);


public:	

	/**
	 * Public constructor. Can be used to create objects.
	 *
	 * @param p	Pointer to pdf object in which this object will exist.
	 */
	CObjectComplex (CPdf& p);

#ifdef DEBUG
CObjectComplex (int /*i*/) : value(Value()) {Checker check (this,OPER_CREATE);};
#endif


	/**
	 * Return type of this property.
	 *
	 * @return Type of this property.
	 */
	virtual PropertyType getType () const {return Tp;};
	

	/**
	 * Returns string representation of (x)pdf object. 
	 * 
	 * @param str 	After successful call, it will hold string representation 
	 * 				of current object.
	 */
	virtual void getStringRepresentation (std::string& str) const;
	
	
	/**
	 * Try to make an (x)pdf object from string.
	 * <exception cref="..." />
	 *
	 * @param str0 Object in a text form.
	 */
	void setStringRepresentation (const std::string& strO) {assert (!"is this function really needed???");};

	
	/**
	 * Change the value of an object. The variable type depends
	 * on CObject type. 
	 * For complex types, it is equal to setStringRepresentation().
	 * 
	 * <exception cref="ObjBadValueE "/> Thrown When a value cannot be set due to bad value e.g. in complex types.
	 *
	 * @param val	Value that will be set.
	 */
	void writeValue (WriteType val) {assert (!"is this function really needed???");};
  
	
	/**
	 * Returns if it is one of special objects CPdf,CPage etc.
	 *
	 * @return Type of special object.
	 */
	virtual SpecialObjectType getSpecialObjType() const {return sNone;};

	
	/**
	 * Returns pointer to special object.
	 *
	 * @return Pointer to special child object specified by template argument. 
	 */
	template<typename T>
	T* getSpecialObjectPtr () const
	{
		STATIC_CHECK(sizeof(T)>=sizeof(CObjectComplex<Tp,Checker>),DESTINATION_TYPE_TOO_NARROW); 
		return dynamic_cast<T*>(this);
	}

	
	/**
	 * Indicate that you do not want to use this object again.
	 * If it is an indirect object, we have to notify CXref.
	 */
  	virtual void release ();
	

//protected:
/*DEBUG*/public:
	/**
	 * Destructor
	 */
	~CObjectComplex ()	{Checker check (this,OPER_DELETE);};

protected:
	/**
 	 * Notify Writer object that this object has changed. We have to call this
	 * function to make changes visible.
	 *
	 * It is necessary for undo operation, that means saving previous state of the object.
	 * We obtain reference to CXref from CObject::pdf.
	 */
	void dispatchChange () const; 


	//
	//
	// Specific features by Incomplete Instantiation
	//
	//
public:
	/** 
	 * Returns property count.
	 * 
	 * @return Property count.
	 */
	inline size_t getPropertyCount () const {return value.size();};
 

	/**
	 * Inserts all property names to container supplied by caller. 
	 * 
	 * REMARK: Specific for pDict and pStream.
     	 *
	 * @param container Container of string objects. STL vector,list,deque.
	 */
	virtual void getAllPropertyNames (std::list<std::string>& container) const;


	/**
	 * Returns value of property identified by its name/position depending on type of this object.
   	 *
   	 * @param 	id 	Variable identifying position of the property.
	 * @return	Variable where the value will be stored.
   	 */
   	IProperty* getPropertyValue (PropertyId id) const;

	
	/**
	 * Returns property type of an item identified by name/position.
	 *
	 * <exception cref="ObjInvalidPositionInComplex "/> When the id does not correctly identify an item.
	 *
	 * @param	name	Name of the property.
	 * @return		Property type.	
	 */
	PropertyType getPropertyType (PropertyId id) const 
		{return getPropertyValue(id)->getType();};

	
	/**
	 * Sets property type of an item.
	 *
	 * @param	id		Name/Index of property
	 * @param	value	Value, for simple types (int,string,...) and for complex types IProperty*
	 */
	virtual IProperty* setPropertyValue (PropertyId id, IProperty& ip);
	
	/**
	 * Adds property to array/dict/stream. The property that should be added will
	 * automaticly be associated with the pdf this object lives in.
	 * Finally indicate that this object has changed.
	 *
	 * @param newIp 	New property.
	 * @param propertyName 	Name of the created property
	 */
	void addProperty (IProperty& newIp);
	void addProperty (const std::string& propertyName, IProperty& newIp);
	
	
	/**
	 * Remove property from array/dict/stream. If the xpdf Object to be removed is 
	 * associated with an IProperty call release(). Otherwise just free the memory
	 * occupied by the xpdf object.
	 * Finally indicate that this object has changed.
	 *
	 * <exception cref="ObjInvalidPositionInComplex "/> When the id does not correctly identify an item.
	 * 
	 * @param id Name/Index of property
	 */
	void delProperty (PropertyId /*id*/);



	//
	// Helper functions
	//
protected:
	/**
	 * Make xpdf Object from this object.
	 *
	 * @return Xpdf object representing actual value of this simple object.
	 */
	virtual Object*	_makeXpdfObject () const;

private:
	
	/**
	 * Finds out if this object is indirect.
	 *
	 * @return true if this object is indirect, false otherwise.
	 */
	bool _isIndirect () const;

	/**
	 * Make everything needed to indicate that this object has changed.
	 * Notifies all obervers associated with this property.
	 */
	inline void
	_objectChanged ()
	{
		// Dispatch the change
		dispatchChange ();
		// Notify everybody about this change
		IProperty::notifyObservers ();
	}


public:
	/**
	 * Return all object we have access to.
	 *
	 * @param store Container of objects.
	 */
	template <typename Storage>
	void _getAllChildObjects (Storage& store) const;

};



//=====================================================================================
//
// CObject types
//

typedef CObjectSimple<pNull>	CNull;
typedef CObjectSimple<pBool>	CBool;
typedef CObjectSimple<pInt>		CInt;
typedef CObjectSimple<pReal>	CReal;
typedef CObjectSimple<pString> 	CString;
typedef CObjectSimple<pName> 	CName;
typedef CObjectSimple<pRef> 	CRef;

typedef CObjectComplex<pStream> CStream;
typedef CObjectComplex<pArray>	CArray;
typedef CObjectComplex<pDict>	CDict;




//=====================================================================================
//
//	Memory checker classes
//

/**
 * No memory checks done.
 */
class NoMemChecker 
{public: 
	NoMemChecker (IProperty*, _eOperations) {};
};

/**
 * This class stores pointer to every created class in a container. When a class is destroyed, it is removed
 * from the container.
 *
 * After the end of a program, we can count how many objects have not been released.
 * 
 */
class BasicMemChecker
{
public:
	typedef std::list<const IProperty*> _IPsList;
private:
	static _IPsList ips;

public:
	//
	//
	//	   
	BasicMemChecker (const IProperty* ip, _eOperations operation)
	{
		std::cout << std::setw (10) << std::setfill ('<') << "\t";
		std::cout << std::setbase (16);
		std::cout << "IProperty [0x"<< (unsigned)ip << "] ";
				
		switch (operation)
		{
			case OPER_DELETE:
					{
					std::cout << "deleted.";
					_IPsList::iterator it = find (ips.begin(), ips.end(), ip);
					if (it != ips.end())
							ips.erase (it);
					else
							std::cout << std::endl << "!!!!!!!!!! deleting what was not created !!!!!!!!!!1" << std::endl;
					}
					break;

			case OPER_CREATE:
					std::cout << "created.";
					ips.push_back (ip);
					break;
					
			default:
					break;
		}
		
		std::cout << std::setbase (10);
		std::cout << "\t" << std::setw (10) << std::setfill ('>') << "" << std::endl;
	};
	
	//
	// Get living IProperty count
	//
	size_t getCount () {return ips.size (); };
};



//=====================================================================================
//
//	Find element functors
//


/**
 * This class is used as functor to stl find algorithm.
 * Finds out an item specified by its position. 
 *
 * More effective algorithms could be used but this approach is 
 * used to get more generic.
 */
class ArrayIdxComparator
{
private:
	unsigned int pos;
	IProperty* ip;
public:
		ArrayIdxComparator (unsigned int p) : pos(p),ip(NULL) {};
		
		inline IProperty* getIProperty () {return ip;};
		
		bool operator() (IProperty* _ip)
		{	
			if (0 == pos)
			{
				ip = _ip;
				return true;
			}
			pos--;
			return false;
		}
};


/**
 * This class is used as functor to stl find algorithm.
 * Finds out an item specified by name. 
 *
 * Perhaps more effective algorithms could be used but this approach is 
 * used to get more generic.
 */
class DictIdxComparator
{
private:
	std::string str;
	IProperty* ip;
public:
		DictIdxComparator (const std::string& s) : str(s),ip(NULL) {};
		
		inline IProperty* getIProperty () {return ip;};
		
		bool operator() (std::pair<std::string,IProperty*> item)
		{	
			if (item.first == str)
			{
				ip = item.second;
				return true;
			}
			
			return false;
		};
};



//=====================================================================================
//
//  Helper functions located in cobject.cc
//
namespace utils {


		
/**
 * Return simple xpdf object (null,number,string...) in string representation.
 * 
 * REMARK: String can represent more different objects, so we have to distinguish among them.
 * This is done at compile time with use of templates, but because of this we have to
 * make other functions also template.
 *
 * @param Value that will be converted to string.
 * @param Output string
 */
template <PropertyType Tp> void simpleValueToString (bool val,std::string& str);
template <PropertyType Tp> void simpleValueToString (int val,std::string& str);
template <PropertyType Tp> void simpleValueToString (double val,std::string& str);
template <PropertyType Tp> void simpleValueToString (const std::string& val,std::string& str);
template <PropertyType Tp> void simpleValueToString (const NullType& val,std::string& str);
template <PropertyType Tp> void simpleValueToString (const IndiRef& val,std::string& str);

/**
 * Return complex xpdf object (null,number,string...) in string representation.
 *
 * REMARK: It is a template function because I think stream won't be converted to string
 * as easily as a dictionary. So we specialize these function for pArray,pStream and pDict.
 *
 * @param Value that will be converted to string.
 * @param Output string
 */
template <PropertyType Tp> void complexValueToString (const std::vector<IProperty*>& val,std::string& str);
template <PropertyType Tp> void complexValueToString (const std::list<std::pair<std::string,IProperty*> >& val,std::string& str);

/**
 * Save real xpdf object value to val.
 * 
 * @param obj	Xpdf object which holds the value.
 * @param val	Variable where the value will be stored.
 */
template <PropertyType Tp,typename T> void simpleValueFromXpdfObj (Object& obj, T val);
template <PropertyType Tp,typename T> void complexValueFromXpdfObj (IProperty& ip, Object& obj, T val);

/**
 * Create xpdf Object which represents value.
 * 
 * @param obj	Value where the value is stored.
 * @return 		Xpdf object where the value is stored.
 */
template <PropertyType Tp,typename T> Object* simpleValueToXpdfObj (T val);

/**
 * Template functions can't be virutal, so this is a helper
 * function that has the same functionality as getAllPropertyNames() but
 * can take as parameter any container type that supports push_back function.
 *
 * @param container Container to which all names will be added.
 */
template<typename T>
inline void 
getAllNames (T& container, const std::list<std::pair<std::string,IProperty*> >& store)
{
	for (std::list<std::pair<std::string,IProperty*> >::const_iterator it = store.begin();
		it != store.end(); it++)
	{
			container.push_back ((*it).first);
	}
}
template<typename T, typename U> void getAllNames (T&, U&) {}


/**
 * Parses string to get simple values like int, name, bool etc.
 * 
 * <a cref="ObjBadValueE" /> Thrown when the string, can't be parsed correctly.
 * 
 * @param strO	String to be parsed.
 * @param val	Desired value.
 */
void simpleValueFromString (const std::string& str, bool& val);
void simpleValueFromString (const std::string& str, int& val);
void simpleValueFromString (const std::string& str, double& val);
void simpleValueFromString (const std::string& str, std::string& val);
void simpleValueFromString (const std::string& str, IndiRef& val);

/**
 * Free an object. We assume that all child objects (if any)
 * have been already freed.
 *
 * Copy & paste from Object.h
 * 
 * @param obj	Object to be freed.
 */
void freeXpdfObject (Object* obj);

/**
 * Create xpdf object from string.
 *
 * @param str String that should represent an xpdf object.
 *
 * @return Xpdf object whose string representation is in str.
 */
Object* xpdfObjFromString (const std::string& str);


/**
 * Constructs an item containing IProperty of a special container from a value that we want to replace.
 *
 * @param item  Item that will be replaced
 * @patam ip	IProperty that will be inserted;
 */
inline IProperty* 
constructItemFromIProperty (IProperty*,IProperty& ip) {return &ip;}
inline std::pair<std::string,IProperty*>
constructItemFromIProperty (std::pair<std::string,IProperty*>& item,IProperty& ip) {return std::make_pair(item.first,&ip);}

/**
 * Get IProperty from an item of a special container.
 *
 * @param item Item of a special container.
 */
inline IProperty* 
getIPropertyFromItem (IProperty* item) {return item;}
inline IProperty*
getIPropertyFromItem (const std::pair<std::string,IProperty*>& item) {return item.second;}




} /* namespace utils */
} /* namespace pdfobjects */




//
// Include the actual implementation of CObject class
//
#include "cobjectI.h"


#endif // COBJECT_H

