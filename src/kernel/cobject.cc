/*
 * =====================================================================================
 *        Filename:  cobject.cc
 *     Description: CObject helper functions implementation.
 *         Created:  08/02/2006 02:08:14 PM CET
 *          Author:  jmisutka (), 
 *         Changes: Because anonymous namespaces in headers are in 99% not correct. 
 *         			(I knew that but I had mixed it with templates.)
 * =====================================================================================
 */

// static
#include "static.h"
// xpdf
#include "xpdf.h"

#include "cobject.h"


// =====================================================================================
namespace pdfobjects{
// =====================================================================================

//
// Case insensitive comparator
//
namespace
{
		bool nocase_compare (char c1, char c2)
		{
			return toupper(c1) == toupper(c2);
		}
}


//
// General object functions
//
// =====================================================================================
namespace utils {
// =====================================================================================

using namespace std;

//
// Forward declaration
//
void xpdfObjToString (Object& obj, string& str);


//
// String constants used when converting objects to strings
//

// pNull
const string CNULL_NULL = "null";
	
// SimpleObjects
const string CBOOL_TRUE  = 	"true";
const string CBOOL_FALSE = 	"false";

const string CNAME_PREFIX 	= "/";

const string CSTRING_PREFIX = "(";
const string CSTRING_SUFFIX = ")";

const string CREF_MIDDLE 	= " ";
const string CREF_SUFFIX 	= " R";

// ComplexObjects
const string CARRAY_PREFIX 	= "[";
const string CARRAY_MIDDLE 	= " ";
const string CARRAY_SUFFIX 	= " ]";

const string CDICT_PREFIX 	= "<<";
const string CDICT_MIDDLE 	= "\n/";
const string CDICT_BETWEEN_NAMES = " ";
const string CDICT_SUFFIX 	= "\n>>";

const string CSTREAM_STREAM = "<stream>";



// =====================================================================================
namespace {
// =====================================================================================



		//
		// ReadProcessors for simple types
		//
		template<typename Storage, typename Val>
		struct xpdfBoolReader
		{public:
				void operator() (Storage obj, Val val)
					{val = (0 != obj.getBool());}
		};

		template<typename Storage, typename Val>
		struct xpdfIntReader
		{public:
				void operator() (Storage obj, Val val)
					{val = obj.getInt ();}
		};

		template<typename Storage, typename Val>
		struct xpdfRealReader
		{public:
				void operator() (Storage obj, Val val)
					{val = obj.getNum ();}
		};

		template<typename Storage, typename Val>
		struct xpdfStringReader
		{public:
				void operator() (Storage obj, Val val)
					{val = obj.getString ()->getCString();}
		};

		template<typename Storage, typename Val>
		struct xpdfNameReader
		{public:
				void operator() (Storage obj, Val val)
					{val = obj.getName ();}
		};


		template<typename Storage, typename Val>
		struct xpdfRefReader
		{public:
				void operator() (Storage obj, Val val)
					{val.num = obj.getRefNum();
					 val.gen = obj.getRefGen();}
		};
		
		//
		// WriteProcessors
		//
		// We know that Storage is xpdf Object and value type depends on each writer type
		//
		template<typename Storage, typename Val>
		struct xpdfBoolWriter
		{public:
				Object* operator() (Storage obj, Val val)
					{return obj->initBool (GBool(val));}
		};

		template<typename Storage, typename Val>
		struct xpdfIntWriter
		{public:
				Object* operator() (Storage obj, Val val)
					{return obj->initInt (val);}
		};

		template<typename Storage, typename Val>
		struct xpdfRealWriter
		{public:
				Object* operator() (Storage obj, Val val)
					{return obj->initReal (val);}
		};

		template<typename Storage, typename Val>
		struct xpdfStringWriter
		{public:
				Object* operator() (Storage obj, Val val)
					{return obj->initString (new GString(val.c_str()));}
		};

		template<typename Storage, typename Val>
		struct xpdfNameWriter
		{public:
				Object* operator() (Storage obj, Val val)
					{return obj->initName (const_cast<char*>(val.c_str()));}
		};

		template<typename Storage, typename Val>
		struct xpdfNullWriter
		{public:
				Object* operator() (Storage obj, Val)
					{return obj->initNull ();}
		};

		template<typename Storage, typename Val>
		struct xpdfRefWriter
		{public:
				Object* operator() (Storage obj, Val val)
					{return obj->initRef (val.num, val.gen);}
		};


		/*
		 * This type trait holds information which writer class to use with specific CObject class.
		 * REMARK: If we want to abandon using Object as information holder, you need to rewrite
		 * these classes to be able to write values.
		 * 
		 * One class can be used with more CObject classes.
		 */
		template<typename T, typename U, PropertyType Tp> struct ProcessorTraitSimple; 
		template<typename T, typename U> struct ProcessorTraitSimple<T,U,pBool>  
			{typedef struct utils::xpdfBoolWriter<T,U>	xpdfWriteProcessor;
			 typedef struct utils::xpdfBoolReader<T,U> 	xpdfReadProcessor;};
		template<typename T, typename U> struct ProcessorTraitSimple<T,U,pInt>   
			{typedef struct utils::xpdfIntWriter<T,U> 	xpdfWriteProcessor;
			 typedef struct utils::xpdfIntReader<T,U> 	xpdfReadProcessor;};
		template<typename T, typename U> struct ProcessorTraitSimple<T,U,pReal>  
			{typedef struct utils::xpdfRealWriter<T,U> 	xpdfWriteProcessor;
			 typedef struct utils::xpdfRealReader<T,U> 	xpdfReadProcessor;};
		template<typename T, typename U> struct ProcessorTraitSimple<T,U,pString>
			{typedef struct utils::xpdfStringWriter<T,U>xpdfWriteProcessor;
			 typedef struct utils::xpdfStringReader<T,U> xpdfReadProcessor;};
		template<typename T, typename U> struct ProcessorTraitSimple<T,U,pName>  
			{typedef struct utils::xpdfNameWriter<T,U> 	xpdfWriteProcessor;
			 typedef struct utils::xpdfNameReader<T,U> 	xpdfReadProcessor;};
		template<typename T, typename U> struct ProcessorTraitSimple<T,U,pNull>   
			{typedef struct utils::xpdfNullWriter<T,U> 	xpdfWriteProcessor;};
		template<typename T, typename U> struct ProcessorTraitSimple<T,U,pRef>   
			{typedef struct utils::xpdfRefWriter<T,U> 	xpdfWriteProcessor;
			 typedef struct utils::xpdfRefReader<T,U> 	xpdfReadProcessor;};


		//
		//
		//
		template<typename Value, typename Storage>
		struct xpdfArrayReader
		{public:
				void operator() (IProperty& ip, const Value array, Storage val)
				{
					assert (objArray == array.getType());
					assert (0 <= array.arrayGetLength ());
					printDbg (debug::DBG_DBG, "xpdfArrayReader\tobjType = " << array.getTypeName() );
					
					CPdf* pdf = ip.getPdf ();
					assert (NULL != ip.getPdf ());
					if (NULL == pdf)
						throw CObjInvalidObject ();
					
					Object obj;

					int len = array.arrayGetLength ();
					for (int i = 0; i < len; ++i)
					{
							// Get Object at i-th position
							array.arrayGetNF (i, &obj);
							// Create CObject from it
							boost::shared_ptr<IProperty> cobj (createObjFromXpdfObj (*pdf, obj, ip.getIndiRef()));
							if (cobj)
							{
								// Store it in the storage
								val.push_back (cobj);
								// Free resources allocated by the object
								obj.free ();
								
							}else
								throw CObjInvalidObject ();

					}	// for
				}	// void operator
		};

		template<typename Value, typename Storage>
		struct xpdfDictReader
		{public:
				void operator() (IProperty& ip, const Value dict, Storage val)
				{
					assert (objDict == dict.getType());
					assert (0 <= dict.dictGetLength ());
					printDbg (debug::DBG_DBG, "xpdfDictReader\tobjType = " << dict.getTypeName() );
					
					CPdf* pdf = ip.getPdf ();
					assert (NULL != ip.getPdf ());
					if (NULL == pdf)
						throw CObjInvalidObject ();
				
					Object obj;

					int len = dict.dictGetLength ();
					for (int i = 0; i < len; ++i)
					{
							// Get Object at i-th position
							string key = dict.dictGetKey (i);
							dict.dictGetVal (i,&obj);
							// Create CObject from it
							boost::shared_ptr<IProperty> cobj (createObjFromXpdfObj (*pdf, obj, ip.getIndiRef()));
							if (cobj)
							{
								// Store it in the storage
								val.push_back (make_pair(key,cobj));
								// Free resources allocated by the object
								obj.free ();

							}else
								throw CObjInvalidObject ();
					}
				}
		};

		template<typename Value, typename Storage>
		struct xpdfStreamReader
		{public:
				void operator() (IProperty&, Value /*obj*/, Storage /*val*/)
				{
						assert (!"not implemented yet.");
				}
		};
				
		/*
		 * This type trait holds information which writer class to use with specific CObject class.
		 * REMARK: If we want to abandon using Object as information holder, you need to rewrite
		 * these classes to be able to write values.
		 * 
		 * One class can be used with more CObject classes.
		 */
		template<typename T, typename U, PropertyType Tp> struct ProcessorTraitComplex; 
		template<typename T, typename U> struct ProcessorTraitComplex<T,U,pArray>  
			{typedef struct utils::xpdfArrayReader<T,U> 	xpdfReadProcessor;};
		template<typename T, typename U> struct ProcessorTraitComplex<T,U,pDict>   
			{typedef struct utils::xpdfDictReader<T,U> 		xpdfReadProcessor;};


		/**
		 *
		 */
		void
		simpleXpdfObjToString (Object& obj,string& str)
		{
			printDbg (debug::DBG_DBG,"simpleXpdfObjToString(" << (unsigned int)&obj << ") objType = " << obj.getTypeName() );

			ostringstream oss;

			switch (obj.getType()) 
			{
			case objBool:
				oss << ((obj.getBool()) ? CBOOL_TRUE : CBOOL_FALSE);
				break;

			case objInt:
				oss << obj.getInt();
				break;

			case objReal:
				oss << obj.getReal ();
				break;

			case objString:
				oss << CSTRING_PREFIX  << obj.getString()->getCString() << CSTRING_SUFFIX;
				break;

			case objName:
				oss << CNAME_PREFIX << obj.getName();
				break;

			case objNull:
				oss << CNULL_NULL;
				break;

			case objRef:
				oss << obj.getRefNum() << CREF_MIDDLE << obj.getRefGen() << CREF_SUFFIX;
				break;

			default:
				assert (!"Bad object passed to simpleXpdfObjToString.");
				throw XpdfInvalidObject (); 
				break;
			}

			// convert oss to string
			str = oss.str ();
		}

		//
		//
		//
		void
		complexXpdfObjToString (Object& obj, string& str)
		{
		 	
			printDbg (debug::DBG_DBG,"complexXpdfObjToString(" << (unsigned int)&obj << ")");
			printDbg (debug::DBG_DBG,"\tobjType = " << obj.getTypeName() );

			ostringstream oss;
			Object o;
			int i;

			switch (obj.getType()) 
			{
			
			case objArray:
				oss << CARRAY_PREFIX;
				for (i = 0; i < obj.arrayGetLength(); ++i) 
				{
					oss << CARRAY_MIDDLE;
					obj.arrayGetNF (i,&o);
					string tmp;
					xpdfObjToString (o,tmp);
					oss << tmp;
					o.free();
				}
				oss << CARRAY_SUFFIX;
				break;

			case objDict:
				oss << CDICT_PREFIX;
				for (i = 0; i <obj.dictGetLength(); ++i) 
				{
					oss << CDICT_MIDDLE << obj.dictGetKey(i) << CDICT_BETWEEN_NAMES;
					obj.dictGetValNF(i, &o);
					string tmp;
					xpdfObjToString (o,tmp);
					oss << tmp;
					o.free();
				}
				oss << CDICT_SUFFIX;
				break;

			case objStream:
				oss << CSTREAM_STREAM;
				break;

			default:
				assert (false);	
				break;
			}

			// convert oss to string
			str = oss.str ();
		}

// =====================================================================================
} // anonymous namespace
// =====================================================================================


//
// Creates CObject from xpdf object.
// 
IProperty*
createObjFromXpdfObj (CPdf& pdf, Object& obj,const IndiRef& ref)
{

		switch (obj.getType ())
		{
			case objBool:
				return new CBool (pdf,obj,ref);

			case objInt:
				return new CInt (pdf,obj,ref);

			case objReal:
				return new CReal (pdf,obj,ref);

			case objString:
				return new CString (pdf,obj,ref);

			case objName:
				return new CName (pdf,obj,ref);

			case objNull:
				return new CNull (pdf,obj,ref);

			case objRef:
				return new CRef (pdf,obj,ref);

			case objArray:
				return new CArray (pdf,obj,ref);

			case objDict:
				return new CDict (pdf,obj,ref);

			case objStream:
				return new CStream (pdf,obj,ref);

			default:
				assert (!"Bad type.");
				break;
		}
}


//
//
//
void 
xpdfObjToString (Object& obj, string& str)
{
	switch (obj.getType())
	{
		case objArray:
		case objDict:
		case objStream:
				complexXpdfObjToString (obj,str);
				break;
		default:
				simpleXpdfObjToString (obj,str);
				break;
	}
}

//
// Convert simple value from CObjectSimple to string
//
template <>
void
simpleValueToString<pBool> (bool val, string& str)
{
	str = ((val) ? CBOOL_TRUE : CBOOL_FALSE);
}
template void simpleValueToString<pBool>	(bool val, string& str);
//
//
//
template <>
void
simpleValueToString<pInt> (int val, string& str)
{
	stringstream oss;
	oss << val;
	str = oss.str ();
}
template void simpleValueToString<pInt> (int val, string& str);
//
//
//
template <>
void
simpleValueToString<pReal> (double val, string& str)
{
	stringstream oss;
	oss << val;
	str = oss.str ();
}
template void simpleValueToString<pReal> (double val, string& str);
//
// Special case for pString
//
template<PropertyType Tp>
void
simpleValueToString (const std::string& val, std::string& str)
{
	STATIC_CHECK ((pString == Tp) || (pName == Tp), COBJECT_INCORRECT_USE_OF_simpleObjToString_FUNCTION);

	switch (Tp)
	{
			case pString:
				str = CSTRING_PREFIX + val + CSTRING_SUFFIX;
				break;

			case pName:
				str = CNAME_PREFIX + val;
				break;

			default:
				assert (0);
				break;
	}
}
template void simpleValueToString<pString> (const string& val, string& str);
template void simpleValueToString<pName> (const string& val, string& str);
//
// Special case for pNull
//
template<>
void
simpleValueToString<pNull> (const NullType&, string& str)
{
	str = CNULL_NULL;
}
template void simpleValueToString<pNull> (const NullType&, string& str);
//
// Special case for pRef
//
template<>
void
simpleValueToString<pRef> (const IndiRef& ref, string& str)
{
	ostringstream oss;
	oss << ref.num << CREF_MIDDLE << ref.gen << CREF_SUFFIX;
	// convert oss to string
	str = oss.str ();
}
template void simpleValueToString<pRef> (const IndiRef&, string& str);



//
// Convert complex value from CObjectComplex to string
//
template<>
void
complexValueToString<pArray> (const PropertyTraitComplex<pArray>::value& val, string& str)
{
		printDbg (debug::DBG_DBG,"complexValueToString<pArray>()" );
		
		ostringstream oss;

		// start tag
		str = CARRAY_PREFIX;
		
		//
		// Loop through all items and get each string and append it to the result
		//
		PropertyTraitComplex<pArray>::value::const_iterator it = val.begin();
		for (; it != val.end(); ++it) 
		{
			str += CARRAY_MIDDLE;
			string tmp;
			(*it)->getStringRepresentation (tmp);
			str += tmp;
		}
		
		// end tag
		str += CARRAY_SUFFIX;
}
template void complexValueToString<pArray> (const PropertyTraitComplex<pArray>::value& val, string& str);
//
//
//
template<>
void
complexValueToString<pDict> (const PropertyTraitComplex<pDict>::value& val, string& str)
{
	printDbg (debug::DBG_DBG,"complexValueToString<pDict>()");

	// start tag
	str = CDICT_PREFIX;

	//
	// Loop through all items and get each items name + string representation
	// and append it to the result
	//
	PropertyTraitComplex<pDict>::value::const_iterator it = val.begin ();
	for (; it != val.end(); ++it) 
	{
		str += CDICT_MIDDLE + (*it).first + CDICT_BETWEEN_NAMES;
		string tmp;
		(*it).second->getStringRepresentation (tmp);
		str += tmp;
	}

	// end tag
	str += CDICT_SUFFIX;
}
template void complexValueToString<pDict> (const PropertyTraitComplex<pDict>::value& val, string& str);

//
//
//
void
freeXpdfObject (Object* obj)
{
	assert (obj != NULL);
	if (NULL == obj)
		throw XpdfInvalidObject ();
	
	std::string str;
	xpdfObjToString (*obj,str); 
	printDbg (debug::DBG_DBG, "freeXpdfObject()\t ..." << str);
	
	// delete all member variables
	obj->free ();
	// delete the object itself
	delete obj;
}


//
//
//
template <PropertyType Tp,typename T>
Object* 
simpleValueToXpdfObj (T val)
{
	Object* obj = new Object ();
	
	typename ProcessorTraitSimple<Object*, T, Tp>::xpdfWriteProcessor wp;
	return wp (obj,val);
}

template Object* simpleValueToXpdfObj<pBool,const bool&> 	(const bool& val);
template Object* simpleValueToXpdfObj<pInt,const int&> 		(const int& val);
template Object* simpleValueToXpdfObj<pReal,const double&> 	(const double& val);
template Object* simpleValueToXpdfObj<pString,const string&>(const string& val);
template Object* simpleValueToXpdfObj<pName,const string&> 	(const string&  val);
template Object* simpleValueToXpdfObj<pNull,const NullType&>(const NullType& val);
template Object* simpleValueToXpdfObj<pRef,const IndiRef&> 	(const IndiRef& val);

//
//
//
template <PropertyType Tp,typename T> 
void
simpleValueFromXpdfObj (Object& obj, T val)
{
	typename ProcessorTraitSimple<Object&, T, Tp>::xpdfReadProcessor rp;
	rp (obj,val);
}

//
// Special case for pNull
//
template <> 
inline void
simpleValueFromXpdfObj<pNull,NullType&> (Object&, NullType&) {assert (!"operation not permitted...");/*THIS IS FORBIDDEN IN THE CALLER*/}

template void simpleValueFromXpdfObj<pBool, bool&> 		(Object& obj,  bool& val);
template void simpleValueFromXpdfObj<pInt, int&> 		(Object& obj,  int& val);
template void simpleValueFromXpdfObj<pReal, double&> 	(Object& obj,  double& val);
template void simpleValueFromXpdfObj<pString, string&>	(Object& obj,  string& val);
template void simpleValueFromXpdfObj<pName, string&> 	(Object& obj,  string&  val);
template void simpleValueFromXpdfObj<pNull, NullType&>	(Object& obj,  NullType& val);
template void simpleValueFromXpdfObj<pRef, IndiRef&> 	(Object& obj,  IndiRef& val);

//
//
//
template <PropertyType Tp,typename T> 
inline void
complexValueFromXpdfObj (IProperty& ip, Object& obj, T val)
{
	typename ProcessorTraitComplex<Object&, T, Tp>::xpdfReadProcessor rp;
	rp (ip, obj, val);
}

template void complexValueFromXpdfObj<pArray,PropertyTraitComplex<pArray>::value&> 
		(IProperty& ip, 
		 Object& obj, 
		 PropertyTraitComplex<pArray>::value& val);

template void complexValueFromXpdfObj<pDict, PropertyTraitComplex<pDict>::value&>
		(IProperty& ip, 
		 Object& obj, 
		 PropertyTraitComplex<pDict>::value& val);


//
//
//
void
simpleValueFromString (const std::string& str, bool& val)
{
	static const string __true  ("true");
	static const string __false ("false");
	
	if ( equal (str.begin(), str.end(), __true.begin(), nocase_compare))
		val = true;
	else if ( equal (str.begin(), str.end(), __false.begin(), nocase_compare)) 
		val = false;
	else
		throw CObjBadValue ();
}

void
simpleValueFromString (const std::string& str, int& val)
{
	std::stringstream ss (str);
	ss.exceptions (stringstream::failbit | stringstream::badbit);
	try {
		ss >> val;
	}catch (stringstream::failure& e) 
	{
		throw CObjBadValue ();
  	}					
}

void
simpleValueFromString (const std::string& str, double& val)
{
	boost::scoped_ptr<Object> ptrObj (xpdfObjFromString (str));
	
	assert (objReal == ptrObj->getType ());
	if (objReal != ptrObj->getType() && objInt != ptrObj->getType())
		throw CObjBadValue ();
					
	ProcessorTraitSimple<Object&, double&, pReal>::xpdfReadProcessor rp;
	rp (*ptrObj, val);
}

void
simpleValueFromString (const std::string& str, std::string& val)
{
	val = str;
}

void
simpleValueFromString (const std::string& str, IndiRef& val)
{
	std::stringstream ss (str);
	ss.exceptions (stringstream::failbit | stringstream::badbit);
	try {
		ss >> val.num;
		ss >> val.gen;
	}catch (stringstream::failure& e) 
	{
		throw CObjBadValue ();
	}
}


Object*
xpdfObjFromString (const std::string& str)
{

	printDbg (debug::DBG_DBG,"xpdfObjFromString from " << str);
	
	//
	// Create parser. It can create complex types. Lexer knows just simple types.
	// Lexer SHOULD delete MemStream.
	//
	auto_ptr<Object> dct (new Object());
	//
	// xpdf MemStream DOES NOT free buf unless doDecrypt is called, but IT IS NOT
	// here, so we have to deallocate it !!
	// 
	size_t len = str.length ();
	char* pStr = new char [len + 1];
	strncpy (pStr, str.c_str(), len + 1);
					
	auto_ptr<Parser> parser	(new Parser (NULL, 
						       		      new Lexer (NULL, 
												  	 new MemStream (pStr, 0, len, dct.get())
												     )
										) 
							);
	//
	// Get xpdf obj from the stream
	//
	Object* obj = new Object;
	parser->getObj (obj);
	
	// delete string we don't need it anymore
	delete[] pStr;
	
	const string null ("null");

	//
	// If xpdf returned objNull and we did not give him null, an error occured
	//
	if ( (obj->isNull()) && !equal(str.begin(), str.end(), null.begin(), nocase_compare) )
	{
		obj->free ();
		delete obj;
		throw CObjBadValue ();
	}

	return obj;
}


//
//
//
bool
objHasParent (const IProperty& ip)
{
	CPdf* pdf = ip.getPdf ();
	assert (NULL != pdf);
	if (NULL == pdf)
		throw CObjInvalidOperation ();

	if ( &ip == pdf->getIndirectProperty(ip.getIndiRef()).get() )
		return true;
	else
		return false;
}




// =====================================================================================
} /* namespace utils */
// =====================================================================================
} /* namespace pdfobjects */
// =====================================================================================
