/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006, 2007, 2008  PDFedit team: Michal Hocko,
 *                                              Miroslav Jahoda,
 *                                              Jozef Misutka,
 *                                              Martin Petricek
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
// xpdf
#include "kernel/xpdf.h"
//
#include "kernel/pdfspecification.h"
#include "kernel/factories.h"
#include "kernel/cobject.h"
#include "kernel/cpdf.h"
#include "kernel/cxref.h"


// =====================================================================================
namespace pdfobjects{
// =====================================================================================
namespace utils {
// =====================================================================================

using namespace std;
using namespace xpdf;
using namespace boost;


// =====================================================================================
namespace {
// =====================================================================================


	// helper function
	size_t 
	total_size_stream (size_t dict_size, size_t streambuf_size)
	{
		return dict_size + Specification::CSTREAM_HEADER.length() 
				+ streambuf_size + Specification::CSTREAM_FOOTER.length();
	}

	/**
	 * Read processors for simple types.
	 */
	template<typename Storage, typename Val>
	struct xpdfBoolReader
	{
		public:
			void operator() (Storage obj, Val val)
			{
				if (objBool != obj.getType())
					throw ElementBadTypeException ("Xpdf object is not bool.");
			 	val = (0 != obj.getBool());
			}
	};

	/** \copydoc xpdfBoolReader */
	template<typename Storage, typename Val>
	struct xpdfIntReader
	{
		public:
			void operator() (Storage obj, Val val)
			{
				if (objInt != obj.getType())
					throw ElementBadTypeException ("Xpdf object is not int.");
			 	val = obj.getInt ();
			}
	};

	/** \copydoc xpdfBoolReader */
	template<typename Storage, typename Val>
	struct xpdfRealReader
	{
		public:
			void operator() (Storage obj, Val val)
			{
				if (!obj.isNum())
					throw ElementBadTypeException ("Xpdf object is not real.");
			 	val = obj.getNum ();
			}
	};

	/** 
	 * Reader for xpdf string objects.
	 * This functor enables conversion  from Storage type (xpdf string object)
	 * to given Val typed container. Val has to implement clear and append
	 * methods.
	 */
	template<typename Storage, typename Val>
	struct xpdfStringReader
	{
		public:
			/** Convers given xpdf string object to string stored to given val
			 * representation.
			 * @param obj Xpdf object to convert (must by objString).
			 * @param val Value storage output.
			 *
			 * Copies all bytes (including 0 bytes) from given object
			 * representing string value.
			 */
			void operator() (Storage obj, Val val)
			{
				// clear val content and add all bytes
				// getCString is not suitable because string may contain 0
				// characters
				val.clear();
				size_t len = obj.getString()->getLength();
				GString * xpdfString=obj.getString();
				for(size_t i=0; i< len; ++i)
				{
					char c = xpdfString->getChar(static_cast<int>(i));
					val += c;
				}
				assert (len == val.length());
			}
	};

	/** \copydoc xpdfBoolReader */
	template<typename Storage, typename Val>
	struct xpdfNameReader
	{
		public:
			void operator() (Storage obj, Val val)
			{
				if (objName != obj.getType())
					throw ElementBadTypeException ("Xpdf object is not name.");
			 	val = obj.getName ();
			}
	};

	/** \copydoc xpdfBoolReader */
	template<typename Storage, typename Val>
	struct xpdfRefReader
	{
		public:
			void operator() (Storage obj, Val val)
			{
				if (objRef != obj.getType())
					throw ElementBadTypeException ("Xpdf object is not ref.");
				val.num = obj.getRefNum();
				val.gen = obj.getRefGen();
			}
	};
	

	/**
	 * WriteProcessors.
	 *
	 * We know that Storage is xpdf Object and value type depends on each writer type
	 */
	template<typename Storage, typename Val>
	struct xpdfBoolWriter
	{
		public:
			Object* operator() (Storage obj, Val val)
			{
				return obj->initBool (GBool(val));
			}
	};

	/** \copydoc xpdfBoolWriter */
	template<typename Storage, typename Val>
	struct xpdfIntWriter
	{
		public:
			Object* operator() (Storage obj, Val val)
			{
				return obj->initInt (val);
			}
	};

	/** \copydoc xpdfBoolWriter */
	template<typename Storage, typename Val>
	struct xpdfRealWriter
	{
		public:
			Object* operator() (Storage obj, Val val)
			{
				return obj->initReal (val);
			}
	};

	/** \copydoc xpdfBoolWriter */
	template<typename Storage, typename Val>
	struct xpdfStringWriter
	{
		public:
			Object* operator() (Storage obj, Val val)
			{
				const char * str = val.c_str();
				return obj->initString (new GString(str, static_cast<int>(val.length())));
			}
	};

	/** \copydoc xpdfBoolWriter */
	template<typename Storage, typename Val>
	struct xpdfNameWriter
	{
		public:
			Object* operator() (Storage obj, Val val)
			{
				return obj->initName (const_cast<char*>(val.c_str()));
			}
	};

	/** \copydoc xpdfBoolWriter */
	template<typename Storage, typename Val>
	struct xpdfNullWriter
	{
		public:
			Object* operator() (Storage obj, Val)
			{
				return obj->initNull ();
			}
	};

	/** \copydoc xpdfBoolWriter */
	template<typename Storage, typename Val>
	struct xpdfRefWriter
	{
		public:
			Object* operator() (Storage obj, Val val)
			{
				return obj->initRef (val.num, val.gen);
			}
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
	{
		typedef struct xpdfBoolReader<T,U> xpdfReadProcessor;
		typedef struct xpdfBoolWriter<T,U> xpdfWriteProcessor;
	};
	template<typename T, typename U> struct ProcessorTraitSimple<T,U,pInt>	 
	{
		typedef struct xpdfIntReader<T,U> xpdfReadProcessor;
		typedef struct xpdfIntWriter<T,U> xpdfWriteProcessor;
	};
	template<typename T, typename U> struct ProcessorTraitSimple<T,U,pReal>  
	{
		typedef struct xpdfRealReader<T,U> xpdfReadProcessor;
		typedef struct xpdfRealWriter<T,U> xpdfWriteProcessor;
	};
	template<typename T, typename U> struct ProcessorTraitSimple<T,U,pString>
	{
		typedef struct xpdfStringReader<T,U> xpdfReadProcessor;
		typedef struct xpdfStringWriter<T,U> xpdfWriteProcessor;
	};
	template<typename T, typename U> struct ProcessorTraitSimple<T,U,pName>  
	{
		typedef struct xpdfNameReader<T,U> xpdfReadProcessor;
		typedef struct xpdfNameWriter<T,U> xpdfWriteProcessor;
	};
	template<typename T, typename U> struct ProcessorTraitSimple<T,U,pNull>   
	{
		typedef struct xpdfNullWriter<T,U> xpdfWriteProcessor;
	};
	template<typename T, typename U> struct ProcessorTraitSimple<T,U,pRef>	 
	{
		typedef struct xpdfRefReader<T,U> xpdfReadProcessor;
		typedef struct xpdfRefWriter<T,U> xpdfWriteProcessor;
	};


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

		switch (obj.getType()) 
		{
		case objBool:
			simpleValueToString<pBool> (gTrue == obj.getBool(), str);
			break;

		case objInt:
			simpleValueToString<pInt> (obj.getInt(), str);
			break;

		case objReal:
			simpleValueToString<pReal> (obj.getReal(), str);
			break;

		case objString:
			simpleValueToString<pString> (string(obj.getString()->getCString()), str);
			break;

		case objName:
			simpleValueToString<pName> (string(obj.getName()), str);
			break;

		case objNull:
			simpleValueToString<pNull> (NullType (), str);
			break;

		case objRef:
			simpleValueToString<pRef> (Ref(obj.getRef()), str);
			break;

		case objCmd: {
			ostringstream oss;
			oss << obj.getCmd ();
			str = oss.str();
			}
			break;

		case objError: {
			ostringstream oss;
			oss << Specification::OBJERROR;
			str = oss.str();
			}
			break;
			
		default:
			assert (!"Bad object passed to simpleXpdfObjToString.");
			throw XpdfInvalidObject (); 
			break;
		
		} // switch
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
		xpdf::XpdfObject o;
		int i;

		switch (obj.getType()) 
		{
		
		case objArray:
			oss << Specification::CARRAY_PREFIX;
			for (i = 0; i < obj.arrayGetLength(); ++i) 
			{
				oss << Specification::CARRAY_MIDDLE;
				obj.arrayGetNF (i,o.get());
				string tmp;
				xpdfObjToString (*o,tmp);
				oss << tmp;
				o.reset ();
			}
			oss << Specification::CARRAY_SUFFIX;
			break;

		case objDict:
			oss << Specification::CDICT_PREFIX;
			for (i = 0; i <obj.dictGetLength(); ++i) 
			{
				oss << Specification::CDICT_MIDDLE << obj.dictGetKey(i) << Specification::CDICT_BETWEEN_NAMES;
				obj.dictGetValNF(i, o.get());
				string tmp;
				xpdfObjToString (*o,tmp);
				oss << tmp;
				o.reset ();
			}
			oss << Specification::CDICT_SUFFIX;
			break;

		case objStream:
			obj.streamReset ();
			{
				Dict* dict = obj.streamGetDict ();
				assert (NULL != dict);
				o->initDict (dict);
				std::string str;
				complexXpdfObjToString (*o, str);
				oss << str;
			}
			
			oss << Specification::CSTREAM_HEADER;
			obj.streamReset ();
			{
			int c = 0;
			while (EOF != (c = obj.streamGetChar())) 
				oss << static_cast<string::value_type> (c);
			}
			obj.streamClose ();
			oss << Specification::CSTREAM_FOOTER;
			break;
		
		default:
			assert (false);	
			break;
		}

		// convert oss to string
		str = oss.str ();
	}


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
			XpdfObject obj;

			int len = array.arrayGetLength ();
			for (int i = 0; i < len; ++i)
			{
				// Get Object at i-th position
				array.arrayGetNF (i, obj.get());
					
				shared_ptr<IProperty> cobj;
				// Create CObject from it
				if (isPdfValid(pdf))
				{
					hasValidRef (ip);
					cobj = shared_ptr<IProperty> (createObjFromXpdfObj (*pdf, *obj, ip.getIndiRef()));

				}else
				{
					cobj = shared_ptr<IProperty> (createObjFromXpdfObj (*obj));
				}

				if (cobj)
				{
					// Store it in the storage
					resultArray.push_back (cobj);
					// Free resources allocated by the object
					obj.reset ();
						
				}else
					throw CObjInvalidObject ();

			}	// for
		}	// void operator
	};

	/**
	 * This object parses xpdf object to CDict.
	 */
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
			xpdf::XpdfObject obj;

			int len = dict.dictGetLength ();
			for (int i = 0; i < len; ++i)
			{
				// Get Object at i-th position
				string key (dict.dictGetKey (i));
				obj->free ();
				dict.dictGetValNF (i,obj.get());

				shared_ptr<IProperty> cobj;
				// Create CObject from it
				if (isPdfValid (pdf))
					cobj = shared_ptr<IProperty> (createObjFromXpdfObj (*pdf, *obj, ip.getIndiRef()));
				else
					cobj = shared_ptr<IProperty> (createObjFromXpdfObj (*obj));

				if (cobj)
				{
					// Store it in the storage
					resultDict.push_back (make_pair(key,cobj));

				}else
					throw CObjInvalidObject ();
			}
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
		{typedef struct xpdfArrayReader<T,U>		xpdfReadProcessor;};
	template<typename T, typename U> struct ProcessorTraitComplex<T,U,pDict>   
		{typedef struct xpdfDictReader<T,U>		xpdfReadProcessor;};



// =====================================================================================
} // namespace
// =====================================================================================




// =====================================================================================
//  Xpdf 2 String / String 2 Xpdf functions
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
//
//
void
getStringFromXpdfStream (std::string& str, ::Object& obj)
{
	if (!obj.isStream())
	{
		assert (!"Object is not stream.");
		throw XpdfInvalidObject ();
	}

	// Clear string
	str.clear ();

	// Reset stream
	obj.streamReset ();
	// Save chars
	int c;
	while (EOF != (c = obj.streamGetChar())) 
		str += static_cast<std::string::value_type> (c);
	// Cleanup
	obj.streamClose ();
}


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
			return CArrayFactory::getInstance (pdf,ref,obj);

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
			return CArrayFactory::getInstance (obj);

		case objDict:
			return CDictFactory::getInstance  (obj);

		case objStream:
			return new CStream (obj);

		default:
			//assert (!"Bad type.");
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
	Object* obj = XPdfObjectFactory::getInstance ();
	
	typename ProcessorTraitSimple<Object*, T, Tp>::xpdfWriteProcessor wp;
	return wp (obj,val);
}

template Object* simpleValueToXpdfObj<pBool,const bool&> (const bool& val);
template Object* simpleValueToXpdfObj<pInt,const int&> (const int& val);
template Object* simpleValueToXpdfObj<pReal,const double&> (const double& val);
template Object* simpleValueToXpdfObj<pString,const string&> (const string& val);
template Object* simpleValueToXpdfObj<pName,const string&> (const string& val);
template Object* simpleValueToXpdfObj<pNull,const NullType&> (const NullType& val);
template Object* simpleValueToXpdfObj<pRef,const IndiRef&> (const IndiRef& val);

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
simpleValueFromXpdfObj<pNull,NullType&> (Object&, NullType&) 
{
	/*assert (!"operation not permitted...");*//*THIS IS FORBIDDEN IN THE CALLER*/
}


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

template void complexValueFromXpdfObj<pArray, CArray::Value&> 
		(IProperty& ip, 
		 Object& obj, 
		 CArray::Value& val);

template void complexValueFromXpdfObj<pDict, CDict::Value&>
		(IProperty& ip, 
		 Object& obj, 
		 CDict::Value& val);


//
//
//
void 
dictFromXpdfObj (CDict& resultDict, ::Object& dict)
{
	assert (objDict == dict.getType());
	if (objDict != dict.getType())
			throw ElementBadTypeException ("");
	assert (0 <= dict.dictGetLength ());
	utilsPrintDbg (debug::DBG_DBG, "" );
			
	xpdf::XpdfObject obj;

	int len = dict.dictGetLength ();
	for (int i = 0; i < len; ++i)
	{
		// Make string from key
		string key = dict.dictGetKey (i);
		// Make IProperty from value
		dict.dictGetValNF (i,obj.get());
		scoped_ptr<IProperty> cobj (createObjFromXpdfObj (*obj));
		if (cobj)
		{
			// Store it in the dictionary
			resultDict.addProperty (key, *cobj);

		}else
			throw CObjInvalidObject ();
	}
	assert ((size_t)len == resultDict.getPropertyCount());
}


// FIXME remove this is duplication from xpdf code. We don't
// need it here (beside nasty complex CObject translation to 
// the xpdf Object based on CObject->string->Object)

//
//
//
::Object*
xpdfObjFromString (const std::string& str, XRef* xref)
{
	//utilsPrintDbg (debug::DBG_DBG,"xpdfObjFromString from " << str);
	//utilsPrintDbg (debug::DBG_DBG,"xpdfObjFromString size " << str.size());
	
	//
	// Create parser. It can create complex types. Lexer knows just simple types.
	// Lexer SHOULD delete MemStream.
	//
	// dct is freed in ~BaseStream
	//
	::Object dct;
	
	// xpdf MemStream frees buf 
	size_t len = str.size ();
	char* pStr = (char *)gmalloc(static_cast<int>(len + 1));
	memcpy(pStr, str.c_str(), len);
	pStr[len] = '\0';
					
	scoped_ptr<Parser> parser(
			new Parser (xref, 
				new Lexer (xref,
					new MemStream (pStr, 0, static_cast<int>(len), &dct, gTrue)
					),
				gTrue
				)
			);
	//
	// Get xpdf obj from the stream
	//
	::Object* obj = XPdfObjectFactory::getInstance();
	parser->getObj (obj);

	//
	// If xpdf returned objNull and we did not give him null, an error occured
	//
	const static string null ("null");
	if ( (obj->isNull()) && !equal(str.begin(), str.end(), null.begin(), nocase_compare) )
	{
		obj->free ();
		xpdf::freeXpdfObject (obj);
		throw CObjBadValue ();
	}

	return obj;
}

//
//
//
::Object* 
xpdfStreamObjFromBuffer (const CStream::Buffer& buffer, const CDict& dict)
{
	STATIC_CHECK (1 == sizeof(CStream::Buffer::value_type), WANT_TO_READ_ONE_CHAR_BUT_GET_BUFFER_WITH_LARGER_STORAGE_THAN_CHAR);
	
	//
	// Copy buffer and use parser to make stream object
	//
	char* tmpbuf = static_cast<char*> (gmalloc (static_cast<int>(buffer.size() + Specification::CSTREAM_FOOTER.length())));
	size_t i = 0;
	for (CStream::Buffer::const_iterator it = buffer.begin(); it != buffer.end (); ++it)
		tmpbuf [i++] = static_cast<char> ((unsigned char)(*it));
	std::copy (Specification::CSTREAM_FOOTER.begin(), Specification::CSTREAM_FOOTER.end(), &(tmpbuf[i]));
	assert (i == buffer.size());
	//utilsPrintDbg (debug::DBG_DBG, tmpbuf);
	
	// Create stream
	::Object* objDict = dict._makeXpdfObject ();
	// Only undelying dictionary is used from objDict, so we can free objDict normally (this is 
	// due to the strange implementation of xpdf streams, no dict reference counting is used there
	::Stream* stream = new ::MemStream (tmpbuf, 
										static_cast<Guint>(0), 
										static_cast<Guint>(buffer.size()), 
										objDict, true);
	// Set filters
	stream = stream->addFilters (objDict);
	stream->reset ();
	::Object* obj = XPdfObjectFactory::getInstance ();
	obj->initStream (stream);
	
	// Free xpdf object that holds dictionary (not the dictionary itself)
	gfree (objDict);
	
	return obj;
}


size_t streamToCharBuffer (Object & streamObject, Ref ref, CharBuffer & outputBuf, bool asIndirect)
{
using namespace std;
using namespace debug;

	utilsPrintDbg(DBG_DBG, "");
	if(streamObject.getType()!=objStream)
	{
		utilsPrintDbg(DBG_ERR, "Given object is not a stream. Object type="<<streamObject.getType());
		return 0;
	}
	
	// gets stream dictionary string at first
	string dict;
	Dict *streamDict = streamObject.streamGetDict();

	// FIXME dirty workaround because we don't have dedicated function
	// for Dict -> String conversion
	// initDict increases streamDict's reference thus we need to
	// decrease it by free method.
	Object streamDictObj;
	streamDictObj.initDict(streamDict);
	xpdfObjToString(streamDictObj, dict);
	streamDictObj.free();

	// gets buffer len from stream dictionary Length field
	Object lenghtObj;
	streamDict->lookup("Length", &lenghtObj);
	if(!lenghtObj.isInt())
	{
		utilsPrintDbg(DBG_ERR, "Stream dictionary Length field is not int. type="<<lenghtObj.getType());
		lenghtObj.free();
		return 0;
	}
	// we don't need to call free for lenghtObj because it is 
	// int which doesn't allocate any memory for internal data
	if(0>lenghtObj.getInt())
	{
		utilsPrintDbg(DBG_ERR, "Stream dictionary Length field doesn't have correct value. value="<<lenghtObj.getInt());
		return 0;
	}
	size_t bufferLen=(size_t)lenghtObj.getInt();

	// indirect header is filled only if asIndirect flag is set
	// same way footer
	string header="";
	string footer="";
	if(asIndirect)
	{
		ostringstream indirectHeader;
		indirectHeader << ref << " " << Specification::INDIRECT_HEADER << "\n";
		header += indirectHeader.str();
		footer = Specification::INDIRECT_FOOTER;
	}

	// gets total length and allocates CharBuffer for output	
	size_t len = header.length() + total_size_stream (dict.length(), bufferLen) + footer.length(); 
	char* buf = char_buffer_new (len);
	outputBuf = CharBuffer (buf, char_buffer_delete()); 
	
	// get everything together into created buf - we can't use
	// str* functions here, because there may be \0 inside
	// dictionary string represenatation (string entries)
	size_t copied=0;
	memcpy(buf, header.c_str(), header.length());
	copied+=header.length();
	memcpy(buf+copied, dict.c_str(), dict.length());
	copied+=dict.length();
	memcpy(buf+copied, Specification::CSTREAM_HEADER.c_str(), Specification::CSTREAM_HEADER.length());
	copied+=Specification::CSTREAM_HEADER.length();
	// streamBuffer may contain '\0' so rest has to be copied by memcpy
	BaseStream * base=streamObject.getStream()->getBaseStream();
	base->reset();
	size_t i=0;
	for(; i<bufferLen; i++)
	{
		int ch=base->getChar();
		if(ch==EOF)
			break;
		// NOTE that we are reducing int -> char here, so multi-bytes
		// returned from a stream will be stripped to 1B
		if((unsigned char)ch != ch)
			utilsPrintDbg(DBG_DBG, "Too wide character("<<ch<<") in the stream at pos="<<i);
		buf[copied+i]=ch;
	}
	// checks number of written bytes and if any problem (more or less data),
	// clears buffer and returns with 0
	if(i!=bufferLen)
	{
		utilsPrintDbg(DBG_ERR, "Unexpected end of stream. "<<i<<" bytes read.");
		// TODO do we really need to clear whole buffer here? Is it enough to clear
		// the first byte?
		memset(buf, '\0', len);
		streamObject.getStream()->reset();
		return 0;
	}
	if(base->getChar()!=EOF)
	{
		utilsPrintDbg(DBG_ERR, "stream contains more data than claimed by dictionary Length field.");
		// TODO do we really need to clear whole buffer here? Is it enough to clear
		// the first byte?
		memset(buf, '\0', len);
		streamObject.getStream()->reset();
		return 0;
	}
	copied+=bufferLen;
	memcpy(buf + copied, Specification::CSTREAM_FOOTER.c_str(), Specification::CSTREAM_FOOTER.length());
	copied+=Specification::CSTREAM_FOOTER.length();
	memcpy(buf + copied, footer.c_str(), footer.length());
	copied+=footer.length();
	
	// just to be sure
	assert(copied==len);
	
	// restore stream object to the begining
	streamObject.getStream()->reset();
	return len;
}

// =====================================================================================
} /* namespace utils */
// =====================================================================================
} /* namespace pdfobjects */
// =====================================================================================
