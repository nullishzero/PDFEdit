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

// debug
#include "debug.h"

//
#include "exceptions.h"
#include "cobject.h"
//#include "cobjectI.h"


// =====================================================================================

namespace pdfobjects
{

//
// General object functions
//
namespace utils {

using namespace std;

//
//
//
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

//
//
//
void
simpleXpdfObjToString (Object& obj,string& str)
{
	printDbg (0,"simpleXpdfObjToString()");
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
complexXpdfObjToString (Object& /*obj*/, string& /*str*/)
{
/* 	
	printDbg (0,"complexXpdfObjToString(" << (unsigned int)obj << ")");
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
			xpdfObjToString (&o,tmp);
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
			xpdfObjToString (&o,tmp);
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
	str = oss.str ();
*/}


//
//
//
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
//
// We have to explicitely instantiate functions, because we have included them in header cobject.h
//
template void getAllDirectChildXpdfObjects<std::vector<Object*> > (Object& obj, std::vector<Object*>& /*store*/ );


namespace {

	template<typename Storage>
	void
	getAllChildXpdfObjects (Object& obj, Storage& /*store*/)
	{
		switch (obj.getType())
		{
			case objArray:
			{	//int size = obj.arrayGetLength ();
				//for (int i = 0; i < size; i++)
				{
					//store.push_back (obj.getArray()->elems[i]);
					//getAllChildXpdfObjects (obj.getArray()->elems[i]);
				}
			}	return;	
				break;
			
			case objDict:
			{	//int size = obj.dictGetLength ();
				//for (int i = 0; i < size; i++)
				{
					//store.push_back (obj.getDict()->entries[i].val);
					//getAllChildXpdfObjects (obj.getDict()->entries[i].val);
				}
			}	return;	
				break;

			case objStream:
				assert (!"I'm not implemented yet...");
				break;

			default:	// Null, Bool, Int, Real, String, Name
				break;
		}	
	}
}


//
//
//
template <typename Storage>
void getAllChildIPropertyObjects (CPdf& pdf, Object& o, Storage& store)
{
	typedef vector<Object*> ObjList;
	
	ObjList list;
	getAllChildXpdfObjects (o,list);

	ObjList::iterator it = list.begin ();
	for ( ; it != list.end (); it++)
	{
		IProperty* ip = pdf.getExistingProperty (*it);
		if (NULL != ip)
			store.push_back (ip);
	}
}
//
// We have to explicitely instantiate functions, because we have included them in header cobject.h
//
template void getAllChildIPropertyObjects<std::vector<IProperty*> > (CPdf& pdf, Object& o, std::vector<IProperty*>& store );




//
// Purposeful only for Dict and Stream
//
Object*
getXpdfObjectAtPos (Object& obj, const std::string& /*name*/)
{
	assert ((objDict == obj.getType()) || (objStream == obj.getType()) );

	/*Dict* dict = (objDict == obj.getType ()) ? obj.getDict () : obj.streamGetDict();
	Object* o = dict->find (name.c_str()).val;

	if (NULL == o)
		throw ObjInvalidPositionInComplex ();

	return o;
	*/
	return NULL;
}

Object*
getXpdfObjectAtPos (Object& obj, const unsigned int /*pos*/)
{
	assert (objArray == obj.getType());
	//assert (pos < obj.arrayGetLength());
	
//	if (pos > obj.arrayGetLength ())
//		throw ObjInvalidPositionInComplex ();

	//return obj.getArray()->elems[pos];
	
	return NULL;
}


//
// Does not free, just deletes the entry
//
void
removeXpdfObjectAtPos (Object& obj, const std::string& /*name*/)
{
	assert ((objDict == obj.getType()) || (objStream == obj.getType()) );

	// Get the dictionary
	//Dict* dict = (objDict == obj.getType()) ? obj.getDict () : obj.streamGetDict ();

	/*size_t len = dict->getLength() - 1;
	for (int i = 0; i =< len; i++)
	{
		//
		// If we find the item, move everything one back
		//
		if (name == dict->entries[i].key)
		{
			assert (utils::getXpdfObjectAtPos (*obj,name) == dict->entires[i].key);
			
			if (i < len )
			{
				// Free property name
				delete dict->entries[i].key;

				for (int j = i; j < len; j++)
				{
					// copy every entry to the previous one
					dict->entries[j].key = dict->entries[j+1].key;
					dict->entries[j].val = dict->entries[j+1].val;
				}
			}else
			{
				assert (i == len);

				// Free property name
				delete dict->entries[len].key;
			}

			// to be sure
			dict->entries[len].key = dict->entries[len].val = NULL;
			// decrement the size of the dictionary
			dict->length--;
			
			break;
		}
	}
	*/

}

void
removeXpdfObjectAtPos (Object& obj, const unsigned int /*pos*/)
{
	assert (objArray == obj.getType());
//	assert (pos < obj.arrayGetLength());
//	if ((pos < obj.arrayGetLength()) || (0 == obj.arrayGetLength()))
//		throw ObjInvalidPositionInComplex ();

/*	Array* a = obj.getArray();
	assert (NULL != a);

	size_t len = a->getLength () - 1;
	// copy every entry to the previous one
	for (int i = pos; i < len; i++)
	{
		a->entries[i] = a->entries[i+1];
	}

	// to be sure
	a->entries[len] = NULL;
	// decrement the size of the dictionary
	a->length--;
*/}


		
//
//
//
void
freeXpdfObject (Object* obj)
{
	assert (obj != NULL);
	if (NULL == obj)
		throw ObjInvalidObject ();
	printDbg (1,"objectFree()");
	std::string str;
	xpdfObjToString (*obj,str); 
	printDbg (1,"\t..." << str);
	
	switch (obj->getType()) 
	{
		case objString:
			//delete obj->string;
			//obj->string = NULL;
			break;
		
		case objName:
			//gfree(name);
			break;

		case objArray:
			//
			// A bit of a hack. That means do not free any nested
			// objects, we have already done this.
			//
			//obj->getArray()->length = 0;
			//delete obj->getArray();
			//obj->array = NULL;
			break;
		
		case objDict:
			//
			// Another xpdf hack. That means do not free any nested
			// objects, we have already done this.
			//
			//for (i = 0; i < obj->getDict()->length; ++i) 
			{
			//	gfree(obj->getDict()->entries[i].key);
			//	obj->getDict()->entries[i].key = NULL;
			}	
			//obj->getDict()->length = 0;
			//delete obj->getDict();
			//obj->dict = NULL;
			break;
		
		case objStream:
			//
			// really Everything??
			//
			//for (i = 0; i < obj->getStream()->getDict()->length; ++i) 
			{
			//	gfree(obj->getStream()getDict()->entries[i].key);
			//	obj->getStream()->getDict()->entries[i].key = NULL;
			}
			//obj->getStream()->getDict()->length = 0;
			//delete obj->getStream()->getDict();
			break;
		
		default:
			break;
		}

	// delete main obj
	delete obj;
}



//
// Delete processor, it needs to be here because of the CPdf usage
//
template<typename Check>
void
ObjectDeleteProcessor<Check>::operator() (Object* obj)
{
		IProperty* ip = pdf->getExistingProperty (obj);
		
		//
		// There is no IProperty associated with this obj so free it
		// If complex, then it will be freed recursively
		// 
		if (NULL == ip)
		{
			// make a check if any
			check (obj);	
			// free it, use objectFree instead of obj->free, because
			// we do not want to check ref count in Array and Dict
			freeXpdfObject (obj);
		
		}//else
		{
			ip->release ();
		}
}
//
// Explicit instantiating
//
template class ObjectDeleteProcessor<utils::CheckDirectObject>;
template class ObjectDeleteProcessor<utils::NoCheck>;


//
//
//
inline void
getSimpleValueFromString (const std::string& str, bool& val)
{
  if (str == "true")
	val = true;
  else if (str == "false")
	val = false;
  else
	throw ObjBadValueE ();
}

void
getSimpleValueFromString (const std::string& str, int& val)
{
	std::stringstream ss (str);
	ss >> val;
}

inline void
getSimpleValueFromString (const std::string& str, double& val)
{
	val = atof (str.c_str());
}

inline void
getSimpleValueFromString (const std::string& str, std::string& val)
{
	val = str;
}

inline void
getSimpleValueFromString (const std::string& str, IndiRef& val)
{
	std::stringstream ss (str);
	ss >> val.num;
	ss >> val.gen;
}



} /* namespace utils */
} /* namespace pdfobjects */
