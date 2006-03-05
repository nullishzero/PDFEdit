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

#include <ostream>
#include <iostream>
#include <sstream>
#include <string>

//xpdf
#include <Object.h>
#include <Parser.h>
#include <Lexer.h>
#include <Stream.h>

// debug
#include "debug.h"

//
#include "exceptions.h"
#include "cobject.h"


// =====================================================================================

namespace pdfobjects
{

//
// General object functions
//
namespace utils {

using namespace std;

namespace {

		/**
		 * Creates CObject from xpdf object.
		 */
		IProperty*
		createObjFromXpdfObj (CPdf& pdf, Object& obj,const IndiRef& ref, bool direct)
		{

				switch (obj.getType ())
				{
					case objBool:
						return new CBool (pdf,obj,ref,direct);

					case objInt:
						return new CInt (pdf,obj,ref,direct);

					case objReal:
						return new CReal (pdf,obj,ref,direct);

					case objString:
						return new CString (pdf,obj,ref,direct);

					case objName:
						return new CName (pdf,obj,ref,direct);

					case objNull:
						return new CNull (pdf,obj,ref,direct);

					case objRef:
						return new CRef (pdf,obj,ref,direct);

					case objArray:
						return new CArray (pdf,obj,ref,direct);

					case objDict:
						return new CDict (pdf,obj,ref,direct);

					case objStream:
						return new CStream (pdf,obj,ref,direct);

					default:
						assert (!"Bad type.");
						break;
				}

		}

		//
		// ReadProcessors for simple types
		//
		template<typename Storage, typename Val>
		struct BoolReader
		{public:
				void operator() (Storage obj, Val val)
					{val = (0 != obj.getBool());}
		};

		template<typename Storage, typename Val>
		struct IntReader
		{public:
				void operator() (Storage obj, Val val)
					{val = obj.getInt ();}
		};

		template<typename Storage, typename Val>
		struct RealReader
		{public:
				void operator() (Storage obj, Val val)
					{val = obj.getNum ();}
		};

		template<typename Storage, typename Val>
		struct StringReader
		{public:
				void operator() (Storage obj, Val val)
					{val = obj.getString ()->getCString();}
		};

		template<typename Storage, typename Val>
		struct NameReader
		{public:
				void operator() (Storage obj, Val val)
					{val = obj.getName ();}
		};


		template<typename Storage, typename Val>
		struct RefReader
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
		struct BoolWriter
		{public:
				Object* operator() (Storage obj, Val val)
					{return obj->initBool (GBool(val));}
		};

		template<typename Storage, typename Val>
		struct IntWriter
		{public:
				Object* operator() (Storage obj, Val val)
					{return obj->initInt (val);}
		};

		template<typename Storage, typename Val>
		struct RealWriter
		{public:
				Object* operator() (Storage obj, Val val)
					{return obj->initReal (val);}
		};

		template<typename Storage, typename Val>
		struct StringWriter
		{public:
				Object* operator() (Storage /*obj*/, Val /*val*/)
				//	{return obj->initString (new GString(val.c_str()));}
					{return NULL;}
		};

		template<typename Storage, typename Val>
		struct NameWriter
		{public:
				Object* operator() (Storage /*obj*/, Val /*val*/)
					//{return obj->initName (const_cast<char*>(val.c_str()));}
					{return NULL;}
		};

		template<typename Storage, typename Val>
		struct NullWriter
		{public:
				Object* operator() (Storage obj, Val)
					{return obj->initNull ();}
		};

		template<typename Storage, typename Val>
		struct RefWriter
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
			{public: typedef struct utils::BoolWriter<T,U>	WriteProcessor;
			public: typedef struct utils::BoolReader<T,U> 	ReadProcessor;};
		template<typename T, typename U> struct ProcessorTraitSimple<T,U,pInt>   
			{public: typedef struct utils::IntWriter<T,U> 	WriteProcessor;
			public: typedef struct utils::IntReader<T,U> 	ReadProcessor;};
		template<typename T, typename U> struct ProcessorTraitSimple<T,U,pReal>  
			{public: typedef struct utils::RealWriter<T,U> 	WriteProcessor;
			public: typedef struct utils::RealReader<T,U> 	ReadProcessor;};
		template<typename T, typename U> struct ProcessorTraitSimple<T,U,pString>
			{public: typedef struct utils::StringWriter<T,U> WriteProcessor;
			public: typedef struct utils::StringReader<T,U> ReadProcessor;};
		template<typename T, typename U> struct ProcessorTraitSimple<T,U,pName>  
			{public: typedef struct utils::NameWriter<T,U> 	WriteProcessor;
			public: typedef struct utils::NameReader<T,U> 	ReadProcessor;};
		template<typename T, typename U> struct ProcessorTraitSimple<T,U,pNull>   
			{public: typedef struct utils::NullWriter<T,U> 	WriteProcessor;};
		template<typename T, typename U> struct ProcessorTraitSimple<T,U,pRef>   
			{public: typedef struct utils::RefWriter<T,U> 	WriteProcessor;
			public: typedef struct utils::RefReader<T,U> 	ReadProcessor;};


		//
		//
		//
		template<typename Value, typename Storage>
		struct ArrayReader
		{public:
				void operator() (IProperty& ip, const Value /*array*/, Storage val)
				{
					assert (NULL != ip.getPdf ());
					//assert (objArray == array.getType());
					//assert (0 <= array.arrayGetLength ());
					//printDbg (0,"ArrayReader\tobjType = " << array.getTypeName() );
					
					Object obj;

					int len = 10;//array.arrayGetLength ();
					for (int i = 0; i < len; i++)
					{
							// Get Object at i-th position
					//		array.arrayGetNF (i, &obj);
							// Create CObject from it
							IProperty* cobj = createObjFromXpdfObj (*(ip.getPdf ()), obj, ip.getIndiRef(), true);
							assert (cobj);
							if (NULL == cobj)
									throw ObjInvalidObject ();

							// Store it in the storage
							val.push_back (cobj);
					}
				}
		};

		template<typename Value, typename Storage>
		struct DictReader
		{public:
				void operator() (IProperty&, Value /*obj*/, Storage /*val*/)
				{
				}
		};

		template<typename Value, typename Storage>
		struct StreamReader
		{public:
				void operator() (IProperty&, Value /*obj*/, Storage /*val*/)
				{
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
			{public: typedef struct utils::ArrayReader<T,U> 	ReadProcessor;};
		template<typename T, typename U> struct ProcessorTraitComplex<T,U,pDict>   
			{public: typedef struct utils::DictReader<T,U> 		ReadProcessor;};
		template<typename T, typename U> struct ProcessorTraitComplex<T,U,pStream>   
			{public: typedef struct utils::StreamReader<T,U> 	ReadProcessor;};

		//
		// Forward declaration
		//
		void xpdfObjToString (Object& obj, string& str);

		/**
		 *
		 */
		void
		simpleXpdfObjToString (Object& obj,string& str)
		{
			printDbg (0,"simpleXpdfObjToString(" << (unsigned int)&obj << ")");
			printDbg (0,"\tobjType = " << obj.getTypeName() );

			ostringstream oss;

			switch (obj.getType()) 
			{
			case objBool:
				oss << ((obj.getBool()) ? "true" : "false");
				break;

			case objInt:
				oss << obj.getInt();
				break;

			case objReal:
				oss << obj.getReal ();
				break;

			case objString:
				oss << "("  << obj.getString()->getCString() << ")";
				break;

			case objName:
				oss << "/" << obj.getName();
				break;

			case objNull:
				oss << "null";
				break;

			case objRef:
				oss << obj.getRefNum() << " " << obj.getRefGen() << " R";
				break;

			default:
				assert (!"Bad object passed to simpleXpdfObjToString.");
				throw ObjBadTypeE(); 
				break;
			}

			// convert oss to string
			str = oss.str ();
		}

		//
		//
		//
		void
		complexXpdfObjToString (Object& /*obj*/, string&/* str*/)
		{
		 	
			/*printDbg (0,"complexXpdfObjToString(" << (unsigned int)&obj << ")");
			printDbg (0,"\tobjType = " << obj.getTypeName() );

			ostringstream oss;
			Object o;
			int i;

			switch (obj.getType()) 
			{
			
			case objArray:
				oss << "[";
				for (i = 0; i < obj.arrayGetLength(); ++i) 
				{
					if (i > 0)
						oss << " ";
					obj.arrayGetNF (i,&o);
					string tmp;
					xpdfObjToString (o,tmp);
					oss << tmp;
				o.free();
				}
				oss << "]";
				break;

			case objDict:
				oss << "<<";
				for (i = 0; i <obj.dictGetLength(); ++i) 
				{
					oss << " /" << obj.dictGetKey(i) << " ";
					obj.dictGetValNF(i, &o);
					string tmp;
					xpdfObjToString (o,tmp);
					oss << tmp;
					o.free();
				}
				oss << " >>";
				break;

			case objStream:
				oss << "<stream>";
				break;

			default:
				assert (false);	
				break;
			}

			// convert oss to string
			str = oss.str ();*/
		}


		/**
		 *
		 */
		inline void 
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


		/**
		 * Get all objects that are "in" an object with recursion
		 * up to level 1. That means just direct descendats.
		 *
		 * REMARK: Object can't be const because of the superb xpdf implementation.
		 *
		 * @param o		xpdf Object.
		 * @param store storage that implements push_back() function.
		 */
		template <typename Storage>
		void getAllDirectChildXpdfObjects (Object& obj, Storage& /*store*/) 
		{
			switch (obj.getType())
			{
				case objArray:
				{	//int size = obj.arrayGetLength ();
					//for (int i = 0; i < size; i++)
					{
						//store.push_back (obj.getArray()->elems[i]);
					}
				}	return;	
					break;
				
				case objDict:
				{	//int size = obj.dictGetLength ();
					//for (int i = 0; i < size; i++)
					{
						//store.push_back (obj.getDict()->entries[i].val);
					}
				}	return;	
					break;

				case objStream:
					assert (!"I'm not implemented yet...");
					break;

				case objRef:
					assert (!"I'm not implemented yet...");
					break;

				default:	// Null, Bool, Int, Real, String, Name
					assert (!"Bad object passed to getAllDirectChildXpdfObjects.");
					throw ObjBadTypeE ();
					break;
			}	
		}

} // anonymous namespace




//
// Convert simple value from CObjectSimple to string
//
template <>
void
simpleValueToString<pBool> (bool val, string& str)
{
	printDbg (0,"simpleValueToString()");

	str = ((val) ? "true" : "false");
}
template void simpleValueToString<pBool>	(bool val, string& str);
//
//
//
template <>
void
simpleValueToString<pInt> (int val, string& str)
{
	printDbg (0,"simpleValueToString()");

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
	printDbg (0,"simpleValueToString()");

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
	printDbg (0,"simpleValueToString()");

	switch (Tp)
	{
			case pString:
				str = "(" + val + ")";
				break;

			case pName:
				str = "/" + val;
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
	printDbg (0,"simpleValueToString()");

	str = "null";
}
template void simpleValueToString<pNull> (const NullType&, string& str);
//
// Special case for pRef
//
template<>
void
simpleValueToString<pRef> (const IndiRef& ref, string& str)
{
	printDbg (0,"simpleValueToString()");

	ostringstream oss;
	oss << ref.num << " " << ref.gen << " R";
	// convert oss to string
	str = oss.str ();
}
template void simpleValueToString<pRef> (const IndiRef&, string& str);



//
// Convert complex value from CObjectComplex to string
//
template<>
void
complexValueToString<pArray> (const vector<IProperty*>& val, string& str)
{
		printDbg (0,"complexValueToString<pArray>()" );
		
		ostringstream oss;

		// start tag
		str = "[ ";
		
		//
		// Loop through all items and get each string and append it to the result
		//
		vector<IProperty*>::const_iterator it = val.begin();
		for (; it != val.end(); it++) 
		{
			string tmp;
			(*it)->getStringRepresentation (tmp);
			str += tmp;
			str += " ";
		}
		
		// end tag
		str += "]";
}
template void complexValueToString<pArray> (const vector<IProperty*>& val, string& str);
//
//
//
template<>
void
complexValueToString<pDict> (const list<pair<string,IProperty*> >& val, string& str)
{
	printDbg (0,"complexValueToString<pDict>()");

	// start tag
	str = "<<";

	//
	// Loop through all items and get each items name + string representation
	// and append it to the result
	//
	list<pair<string,IProperty*> >::const_iterator it = val.begin ();
	for (; it != val.end(); it++) 
	{
		str +=" /" + (*it).first + " ";
		string tmp;
		(*it).second->getStringRepresentation (tmp);
		str += tmp;
	}

	// end tag
	str +=" >>";
}
template void complexValueToString<pDict> (const list<pair<string,IProperty*> >& val, string& str);
//
//
//
template<>
void
complexValueToString<pStream> (const std::list<std::pair<std::string,IProperty*> >& /*val*/, string& str)
{
	printDbg (0,"complexValueToString<pStream>()");
	str = "<stream>";
}
template void complexValueToString<pStream> (const list<pair<string,IProperty*> >& val, string& str);


//
//
//
void
freeXpdfObject (Object* obj)
{
	assert (obj != NULL);
	if (NULL == obj)
		throw ObjInvalidObject ();
	printDbg (1,"freeXpdfObject()");
	std::string str;
	xpdfObjToString (*obj,str); 
	printDbg (1,"\t..." << str);
	
	// delete all member variables
	obj->free ();
	// delete the object itself
	delete obj;
}



//
//
//
inline void
simpleValueFromString (const std::string& str, bool& val)
{
  if (str == "true")
	val = true;
  else if (str == "false")
	val = false;
  else
	throw ObjBadValueE ();
}

inline void
simpleValueFromString (const std::string& str, int& val)
{
	std::stringstream ss (str);
	ss.exceptions (stringstream::eofbit| stringstream::failbit | stringstream::badbit);
	try {
		ss >> val;
	}catch (stringstream::failure& e) 
	{
		throw ObjBadValueE ();
  	}					
}

inline void
simpleValueFromString (const std::string& str, double& val)
{
	double d = 42.11232;	// anything except 0.0
	if (0.0 != (d=atof (str.c_str())))
		val = d;
	else
		throw ObjBadValueE ();
}

inline void
simpleValueFromString (const std::string& str, std::string& val)
{
	val = str;
}

inline void
simpleValueFromString (const std::string& str, IndiRef& val)
{
	std::stringstream ss (str);
	ss.exceptions (stringstream::eofbit| stringstream::failbit | stringstream::badbit);
	try {
		ss >> val.num;
		ss >> val.gen;
	}catch (stringstream::failure& e) 
	{
		throw ObjBadValueE ();
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
	
	typename ProcessorTraitSimple<Object*, T, Tp>::WriteProcessor wp;
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
	typename ProcessorTraitSimple<Object&, T, Tp>::ReadProcessor rp;
	rp (obj,val);
}

//
// Special case for pNull
//
template <> 
inline void
simpleValueFromXpdfObj<pNull,NullType&> (Object&, NullType&) {/*THIS IS HANDLED IN CALLER*/}

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
	typename ProcessorTraitComplex<Object&, T, Tp>::ReadProcessor rp;
	rp (ip, obj, val);
}

template void complexValueFromXpdfObj<pArray, std::vector<IProperty*>&> (IProperty& ip, Object& obj, std::vector<IProperty*>& val);
template void complexValueFromXpdfObj<pDict,  std::list<std::pair<std::string,IProperty*> >&> 
	(IProperty& ip, Object& obj, std::list<std::pair<std::string,IProperty*> >& val);
template void complexValueFromXpdfObj<pStream,std::list<std::pair<std::string,IProperty*> >&> 
	(IProperty& ip, Object& obj, std::list<std::pair<std::string,IProperty*> >& val);




//
//
//
namespace
{
		bool nocase_compare (char c1, char c2)
		{
			return toupper(c1) == toupper(c2);
		}
}


Object*
xpdfObjFromString (const std::string& str)
{
	//
	// Create parser. It can create complex types. Lexer knows just simple types.
	// Lexer SHOULD delete MemStream
	//
	auto_ptr<Parser> parser	(new Parser (NULL, 
						       		      new Lexer (NULL, 
												  	 new MemStream (strdup(str.c_str()), 0, str.length(), NULL)
												     )
										  ) 
							);
	//
	// Get xpdf obj from the stream
	//
	Object* obj = new Object;
	parser->getObj (obj);
	
	string null ("null");
	
	if ( equal(str.begin (), str.end (), null.begin (), nocase_compare) 
					&& (obj->isNull()))
	{
		throw ObjBadValueE ();
	}

	return obj;
}

	
} /* namespace utils */
} /* namespace pdfobjects */
