// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80
/*
 * =====================================================================================
 *		  Filename:  cobject.cc
 *	   Description: CObject helper functions implementation.
 *		   Created:  08/02/2006 02:08:14 PM CET
 *			Author:  jmisutka (), 
 * =====================================================================================
 */

// static
#include "static.h"
// xpdf
#include "xpdf.h"
//
#include "factories.h"
#include "cobject.h"
#include "cxref.h"


// =====================================================================================
namespace pdfobjects{
// =====================================================================================

/**
 * Case insensitive comparator.
 */
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
using namespace boost;

//
// Forward declaration
//
void xpdfObjToString (Object& obj, string& str);

// =====================================================================================
namespace {
// =====================================================================================

	//
	// String constants used when converting objects to strings
	//
	// CObjectSimple
	/** Object Null string representation. */
	const string CNULL_NULL = "null";
		
	/** Object Bool string repersentation. */
	const string CBOOL_TRUE  =	"true";
	const string CBOOL_FALSE =	"false";

	/** Object Name string representation. */
	const string CNAME_PREFIX	= "/";

	/** Object String string representation. */
	const string CSTRING_PREFIX = "(";
	const string CSTRING_SUFFIX = ")";

	/** Object Ref string representation. */
	const string CREF_MIDDLE	= " ";
	const string CREF_SUFFIX	= " R";

	// CObjectComplex
	/** Object Arraystring representation. */
	const string CARRAY_PREFIX	= "[";
	const string CARRAY_MIDDLE	= " ";
	const string CARRAY_SUFFIX	= " ]";

	/** Object Dictionary string representation. */
	const string CDICT_PREFIX	= "<<";
	const string CDICT_MIDDLE	= "\n/";
	const string CDICT_BETWEEN_NAMES = " ";
	const string CDICT_SUFFIX	= "\n>>";

	/** Object Stream string representation. */
	//const string CSTREAM_STREAM = "<<stream>>";
	const string CSTREAM_HEADER = "\nstream\n";
	const string CSTREAM_FOOTER = "endstream";

	/** Indirect Object header and footer. */
	const string INDIRECT_HEADER = "obj ";
	const string INDIRECT_FOOTER = "\nendobj";

	/**
	 * Read processors for simple types.
	 */
	template<typename Storage, typename Val>
	struct xpdfBoolReader
	{public:
			void operator() (Storage obj, Val val)
			{if (objBool != obj.getType())
				throw ElementBadTypeException ("Xpdf object is not bool.");
			 val = (0 != obj.getBool());}
	};

	template<typename Storage, typename Val>
	struct xpdfIntReader
	{public:
			void operator() (Storage obj, Val val)
			{if (objInt != obj.getType())
				throw ElementBadTypeException ("Xpdf object is not int.");
			 val = obj.getInt ();}
	};

	template<typename Storage, typename Val>
	struct xpdfRealReader
	{public:
			void operator() (Storage obj, Val val)
			{if (!obj.isNum())
				throw ElementBadTypeException ("Xpdf object is not real.");
			 val = obj.getNum ();}
	};

	template<typename Storage, typename Val>
	struct xpdfStringReader
	{public:
			void operator() (Storage obj, Val val)
			{if (objString != obj.getType())
				throw ElementBadTypeException ("Xpdf object is not string.");
			 val = obj.getString ()->getCString();}
	};

	template<typename Storage, typename Val>
	struct xpdfNameReader
	{public:
			void operator() (Storage obj, Val val)
			{if (objName != obj.getType())
				throw ElementBadTypeException ("Xpdf object is not name.");
			 val = obj.getName ();}
	};


	template<typename Storage, typename Val>
	struct xpdfRefReader
	{public:
			void operator() (Storage obj, Val val)
			{if (objRef != obj.getType())
				throw ElementBadTypeException ("Xpdf object is not ref.");
			val.num = obj.getRefNum();
			val.gen = obj.getRefGen();}
	};
	
	/**
	 * WriteProcessors
	 *
	 * We know that Storage is xpdf Object and value type depends on each writer type
	 */
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
		 typedef struct utils::xpdfBoolReader<T,U>	xpdfReadProcessor;};
	template<typename T, typename U> struct ProcessorTraitSimple<T,U,pInt>	 
		{typedef struct utils::xpdfIntWriter<T,U>	xpdfWriteProcessor;
		 typedef struct utils::xpdfIntReader<T,U>	xpdfReadProcessor;};
	template<typename T, typename U> struct ProcessorTraitSimple<T,U,pReal>  
		{typedef struct utils::xpdfRealWriter<T,U>	xpdfWriteProcessor;
		 typedef struct utils::xpdfRealReader<T,U>	xpdfReadProcessor;};
	template<typename T, typename U> struct ProcessorTraitSimple<T,U,pString>
		{typedef struct utils::xpdfStringWriter<T,U>xpdfWriteProcessor;
		 typedef struct utils::xpdfStringReader<T,U> xpdfReadProcessor;};
	template<typename T, typename U> struct ProcessorTraitSimple<T,U,pName>  
		{typedef struct utils::xpdfNameWriter<T,U>	xpdfWriteProcessor;
		 typedef struct utils::xpdfNameReader<T,U>	xpdfReadProcessor;};
	template<typename T, typename U> struct ProcessorTraitSimple<T,U,pNull>   
		{typedef struct utils::xpdfNullWriter<T,U>	xpdfWriteProcessor;};
	template<typename T, typename U> struct ProcessorTraitSimple<T,U,pRef>	 
		{typedef struct utils::xpdfRefWriter<T,U>	xpdfWriteProcessor;
		 typedef struct utils::xpdfRefReader<T,U>	xpdfReadProcessor;};


	/**
	 * This object parses xpdf object to CArray.
	 */
	template<typename ObjectToParse, typename CObject>
	struct xpdfArrayReader
	{public:
		void operator() (IProperty& ip, const ObjectToParse array, CObject resultArray)
		{
			assert (objArray == array.getType());
			if (objArray != array.getType())
				throw ElementBadTypeException ("Array reader got xpdf object that is not array.");
			assert (0 <= array.arrayGetLength ());
			utilsPrintDbg (debug::DBG_DBG, "xpdfArrayReader\tobjType = " << array.getTypeName() );
			
			CPdf* pdf = ip.getPdf ();
			Object obj;

			int len = array.arrayGetLength ();
			for (int i = 0; i < len; ++i)
			{
					// Get Object at i-th position
					array.arrayGetNF (i, &obj);
					
					shared_ptr<IProperty> cobj;
					// Create CObject from it
					if (isInValidPdf(pdf))
					{
						hasValidRef (ip);
						cobj = shared_ptr<IProperty> (createObjFromXpdfObj (*pdf, obj, ip.getIndiRef()));

					}else
					{
						cobj = shared_ptr<IProperty> (createObjFromXpdfObj (obj));
					}

					if (cobj)
					{
						// Store it in the storage
						resultArray.push_back (cobj);
						// Free resources allocated by the object
						obj.free ();
						
					}else
						throw CObjInvalidObject ();

			}	// for
		}	// void operator
	};

	template<typename ObjectToParse, typename CObject>
	struct xpdfDictReader
	{public:
		void operator() (IProperty& ip, const ObjectToParse dict, CObject resultDict)
		{
			assert (objDict == dict.getType());
			if (objDict != dict.getType())
				throw ElementBadTypeException ("Dict reader got xpdf object that is not dict.");
			assert (0 <= dict.dictGetLength ());
			utilsPrintDbg (debug::DBG_DBG, "xpdfDictReader\tobjType = " << dict.getTypeName() );
			
			CPdf* pdf = ip.getPdf ();
			Object obj;

			int len = dict.dictGetLength ();
			for (int i = 0; i < len; ++i)
			{
					// Get Object at i-th position
					string key = dict.dictGetKey (i);
					dict.dictGetValNF (i,&obj);

					shared_ptr<IProperty> cobj;
					// Create CObject from it
					if (isInValidPdf (pdf))
						cobj = shared_ptr<IProperty> (createObjFromXpdfObj (*pdf, obj, ip.getIndiRef()));
					else
						cobj = shared_ptr<IProperty> (createObjFromXpdfObj (obj));

					if (cobj)
					{
						// Store it in the storage
						resultDict.push_back (make_pair(key,cobj));
						// Free resources allocated by the object
						obj.free ();

					}else
						throw CObjInvalidObject ();
			}
		}
	};

	template<typename ObjectToParse, typename CObject>
	struct xpdfStreamReader
	{public:
			void operator() (IProperty&, ObjectToParse /*obj*/, CObject /*val*/)
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
		{typedef struct utils::xpdfArrayReader<T,U>		xpdfReadProcessor;};
	template<typename T, typename U> struct ProcessorTraitComplex<T,U,pDict>   
		{typedef struct utils::xpdfDictReader<T,U>		xpdfReadProcessor;};


	/**
	 * Convert simple xpdf object to string.
	 *
	 * @param obj Object to parse.
	 * @param str Result string representation.
	 */
	void
	simpleXpdfObjToString (Object& obj,string& str)
	{
		utilsPrintDbg (debug::DBG_DBG," objType:" << obj.getTypeName() );

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

	/**
	 * Convert complex xpdf object to string.
	 *
	 * @param obj Object to parse.
	 * @param str Result string representation.
	 */
	void
	complexXpdfObjToString (Object& obj, string& str)
	{
		
		utilsPrintDbg (debug::DBG_DBG,"\tobjType = " << obj.getTypeName() );

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
			obj.streamReset ();
			assert (0 == obj.streamGetPos());
			{
				Dict* dict = obj.streamGetDict ();
				assert (NULL != dict);
				o.initDict (dict);
				std::string str;
				complexXpdfObjToString (o, str);
				oss << str;
			}
			
			oss << CSTREAM_HEADER;
			obj.streamReset ();
			char c;
			while (EOF != (c = obj.streamGetChar())) 
				oss << c;
			obj.streamClose ();
			oss << CSTREAM_FOOTER;
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


// =====================================================================================
//  Creation functions
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
				return CBoolFactory::getInstance (pdf, ref, obj);

			case objInt:
				return CIntFactory::getInstance (pdf, ref, obj);

			case objReal:
				return CRealFactory::getInstance (pdf, ref, obj);

			case objString:
				return CStringFactory::getInstance (pdf, ref, obj);

			case objName:
				return CNameFactory::getInstance (pdf, ref, obj);

			case objNull:
				return CNullFactory::getInstance ();

			case objRef:
				return CRefFactory::getInstance (pdf, ref, obj);

			case objArray:
				return new CArray (pdf,obj,ref);

			case objDict:
				return CDictFactory::getInstance (pdf, ref, obj);

			case objStream:
				return new CStream (pdf,obj,ref);

			default:
				assert (!"Bad type.");
				throw ElementBadTypeException ("createObjFromXpdfObj: Xpdf object has bad type.");
				break;
		}
}

IProperty*
createObjFromXpdfObj (Object& obj)
{

		switch (obj.getType ())
		{
			case objBool:
				return CBoolFactory::getInstance  (obj);

			case objInt:
				return CIntFactory::getInstance  (obj);

			case objReal:
				return CRealFactory::getInstance  (obj);

			case objString:
				return CStringFactory::getInstance  (obj);

			case objName:
				return CNameFactory::getInstance  (obj);

			case objNull:
				return CNullFactory::getInstance  ();

			case objRef:
				return CRefFactory::getInstance  (obj);

			case objArray:
				return new CArray (obj);

			case objDict:
				return CDictFactory::getInstance  (obj);

			case objStream:
				return new CStream (obj);

			default:
				utilsPrintDbg (debug::DBG_PANIC, "Bad type: " << obj.getTypeName () );
				assert (!"Bad type.");
				throw ElementBadTypeException ("createObjFromXpdfObj: Xpdf object has bad type.");
				break;
		}
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

template Object* simpleValueToXpdfObj<pBool,const bool&>	(const bool& val);
template Object* simpleValueToXpdfObj<pInt,const int&>		(const int& val);
template Object* simpleValueToXpdfObj<pReal,const double&>	(const double& val);
template Object* simpleValueToXpdfObj<pString,const string&>(const string& val);
template Object* simpleValueToXpdfObj<pName,const string&>	(const string&	val);
template Object* simpleValueToXpdfObj<pNull,const NullType&>(const NullType& val);
template Object* simpleValueToXpdfObj<pRef,const IndiRef&>	(const IndiRef& val);

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
simpleValueFromXpdfObj<pNull,NullType&> (Object&, NullType&) {/*assert (!"operation not permitted...");*//*THIS IS FORBIDDEN IN THE CALLER*/}

template void simpleValueFromXpdfObj<pBool, bool&>		(Object& obj,  bool& val);
template void simpleValueFromXpdfObj<pInt, int&>		(Object& obj,  int& val);
template void simpleValueFromXpdfObj<pReal, double&>	(Object& obj,  double& val);
template void simpleValueFromXpdfObj<pString, string&>	(Object& obj,  string& val);
template void simpleValueFromXpdfObj<pName, string&>	(Object& obj,  string&	val);
template void simpleValueFromXpdfObj<pNull, NullType&>	(Object& obj,  NullType& val);
template void simpleValueFromXpdfObj<pRef, IndiRef&>	(Object& obj,  IndiRef& val);

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
	static const string __true	("true");
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
	scoped_ptr<Object> ptrObj (xpdfObjFromString (str));
	
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


//
//
//
Object*
xpdfObjFromString (const std::string& str, XRef* xref)
{
	//utilsPrintDbg (debug::DBG_DBG,"xpdfObjFromString from " << str);
	utilsPrintDbg (debug::DBG_DBG,"xpdfObjFromString size " << str.size());
	
	//
	// Create parser. It can create complex types. Lexer knows just simple types.
	// Lexer SHOULD delete MemStream.
	//
	scoped_ptr<Object> dct (new Object());
	//
	// xpdf MemStream DOES NOT free buf unless doDecrypt is called, but IT IS NOT
	// here, so we have to deallocate it !!
	// 
	size_t len = str.length ();
	char* pStr = new char [len + 1];
	strncpy (pStr, str.c_str(), len + 1);
					
	scoped_ptr<Parser> parser	(new Parser (xref, 
										  new Lexer (xref, 
													 new MemStream (pStr, 0, len, dct.get())
													 )
											) 
								);
	//
	// Get xpdf obj from the stream
	//
	Object* obj = XPdfObjectFactory::getInstance();
	parser->getObj (obj);

	//\TODO remove it
	//{
	//std::string str;
	//utils::xpdfObjToString (*obj,str);
	//std::cout << "!!!!!!!!!!!!!!!!!!!!!!!" << str << std::endl;
	//}
	
	// delete string we don't need it anymore
	delete[] pStr;
	
	const string null ("null");

	//
	// If xpdf returned objNull and we did not give him null, an error occured
	//
	if ( (obj->isNull()) && !equal(str.begin(), str.end(), null.begin(), nocase_compare) )
	{
		obj->free ();
		gfree(obj);
		throw CObjBadValue ();
	}

	return obj;
}


// =====================================================================================
//  To/From string functions
// =====================================================================================

// =====================================================================================
namespace {
// =====================================================================================

//
/*
	Beginning with PDF 1.2, any character except null (character code 0) may be included
	in a name by writing its 2-digit hexadecimal code, preceded by the number
	sign character (#); see implementation notes 3 and 4 in Appendix H. This
	syntax is required to represent any of the delimiter or white-space characters or
	the number sign character itself; it is recommended but not required for characters
	whose codes are outside the range 33 (!) to 126 (~). The examples shown in
	Table 3.3 are valid literal names in PDF 1.2 and later.
*/
//
template<typename Iter>
string 
makeNamePdfValid (Iter it, Iter end)
{
	typedef string::value_type Item;
	string tmp;
	for (; it != end; ++it)
	{
		if ( '!' > (*it) || '~' < (*it))
		{ // Convert it to ascii
		
			Item a = (Item) (*it) >> sizeof (Item) * 4;
			Item b = (Item) (*it) & ((unsigned) 1 << sizeof(Item) * 4);
			tmp += string ("#") + a + b;
			
		}else
			tmp += *it;
	}
	
	return tmp;
}
//
/*
 Any characters may appear in a string except unbalanced parentheses and
 the backslash, which must be treated specially./
*/
//
template< typename Iter>
string 
makeStringPdfValid (Iter it, Iter end)
{
	typedef typename string::value_type Item;
	string tmp;
	for (; it != end; ++it)
	{
		if ( '(' == (*it) || ')' == (*it))
		{ // Prepend \ before ( or )
			if ('\\' != tmp[tmp.length() - 1])
				tmp += '\\';
		}
		
		tmp += *it;
	}
	
	return tmp;
}
	

// =====================================================================================
} // namespace
// =====================================================================================

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
// Special case for pString and pName
//
template<PropertyType Tp>
void
simpleValueToString (const std::string& val, std::string& str)
{
	STATIC_CHECK ((pString == Tp) || (pName == Tp), COBJECT_INCORRECT_USE_OF_simpleObjToString_FUNCTION);

	switch (Tp)
	{
			case pString:
				str = CSTRING_PREFIX + makeStringPdfValid (val.begin(), val.end()) + CSTRING_SUFFIX;
				break;

			case pName:
				str = CNAME_PREFIX + makeNamePdfValid (val.begin(), val.end());
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
		utilsPrintDbg (debug::DBG_DBG,"complexValueToString<pArray>()" );
		
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
	utilsPrintDbg (debug::DBG_DBG,"complexValueToString<pDict>()");

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
streamToString (const std::string& strDict, const std::string& buf, std::string& str)
{
	str = strDict;
	str += CSTREAM_HEADER;
	str += buf;
	str += CSTREAM_FOOTER;
}


//
//
//
void 
createIndirectObjectStringFromString  ( const IndiRef& rf, const std::string& val, std::string& output)
{
	ostringstream oss;

	oss << rf.num << " " << rf.gen << " " << INDIRECT_HEADER << "\n";
	oss << val;
	oss << INDIRECT_FOOTER;

	output = oss.str ();
}


// =====================================================================================
//  Other functions
// =====================================================================================

//
//
//
void
freeXpdfObject (Object* obj)
{
	assert (obj != NULL);
	if (NULL == obj)
		throw XpdfInvalidObject ();
	
	// delete all member variables
	obj->free ();
	// delete the object itself
	gfree(obj);
}




//
//
//
bool
objHasParent (const IProperty& ip, boost::shared_ptr<IProperty>& indiObj)
{
	assert (isInValidPdf (ip));
	if (!isInValidPdf (ip))
		throw CObjInvalidOperation ();

	CPdf* pdf = ip.getPdf ();
	if ( &ip == (indiObj=pdf->getIndirectProperty(ip.getIndiRef())).get() )
		return false;
	else
		return true;
}
bool
objHasParent (const IProperty& ip)
{boost::shared_ptr<IProperty> indi;return objHasParent (ip,indi);}



// =====================================================================================
} /* namespace utils */
// =====================================================================================
} /* namespace pdfobjects */
// =====================================================================================
