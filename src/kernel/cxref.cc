// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80
/*
 * $RCSfile$
 *
 * $Log$
 * Revision 1.5  2006/03/23 22:13:51  hockm0bm
 * printDbg added
 * exception handling
 * TODO removed
 * FIXME for revisions handling - throwing exeption
 *
 *
 */
#include "cxref.h"
#include "utils/debug.h"

using namespace pdfobjects;

CXref::~CXref()
{
using namespace debug;

	printDbg(DBG_DBG, "");
	/* FIXME: uncoment when cache is ready.
	if(cache)
		delete cache;
	*/
	
	// deallocates changed storage
	// goes through all elemenents, erases all of them and deallocates value
	// returned from remove object (iterators are not invalidated by remove
	// method)
	ObjectStorage< ::Ref, ObjectEntry*, RefComparator>::Iterator i;
	for(i=changedStorage.begin(); i!= changedStorage.end(); i++)
	{
		::Ref ref=i->first;
		ObjectEntry * entry=changedStorage.remove(ref);

		// check for entry
		if(!entry)
		{
			// this shouldn't happen
			printDbg(DBG_ERR, "ref=["<<ref.num<<", "<<ref.gen<<" doesn't have entry.");
			continue;
		}

		// check for entry object
		if(!entry->object)
		{
			// this shouldn't happen
			printDbg(DBG_ERR, "ref=["<<ref.num<<", "<<ref.gen<<" entry doesn't have an object");
			delete entry;
			continue;
		}

		// check if object is stored
		// if not, prints warning
		if(!entry->stored)
			printDbg(DBG_WARN, "ref=["<<ref.num<<", "<<ref.gen<<" entry is not stored");
	
		delete entry->object;
		delete entry;

	}

	
}

::Object * CXref::changeObject(::Ref ref, ::Object * instance)
{
using namespace debug;

	printDbg(DBG_DBG, "ref=["<< ref.num<<", "<<ref.gen<< "]");

	// discards from cache
	/* FIXME uncoment if cache is available
	if(cache)
		cache->discard(ref);
	*/

	// searches in changedStorage
	// this is returned so it can be used for some
	// history information - value keeper is responsible for
	// deallocation
	ObjectEntry * changedEntry = changedStorage.get(ref);
	::Object * changed=(changedEntry)?changedEntry->object:NULL;
	
	// insert new version to changedStorage
	// this is deep copy of given value
	::Object * object = instance->clone();

	// if changedEntry is NULL - object is change for the first time
	// we have to allocate entry, otherwise, just sets new object
	// value and reset stored to false
	if(!changedEntry)
	{
		changedEntry=new ObjectEntry();
		printDbg(DBG_DBG, "object is changed for the first time, creating changedEntry");
	}
	changedEntry->object=object;
	changedEntry->stored=false;
	changedStorage.put(ref, changedEntry);

	// object has been newly created, so we will set value in
	// the newStorage to true (so we know, that the value has
	// been set after initialization)
	if(newStorage.contains(ref))
	{
		newStorage.put(ref, true);
		printDbg(DBG_DBG, "newStorage updated");
	}
	
	// returns old version
	return changed;
}


void CXref::releaseObject(::Ref ref)
{
using namespace debug;

	printDbg(DBG_DBG, "ref=["<< ref.num<<", "<<ref.gen<<"]");
	// gets previous counter of reference
	// NOTE: if reference is not on releaseObject, count is 0
	// and so its ok
	int count=releasedStorage.get(ref);
	
	// puts reference and increased counter association
	releasedStorage.put(ref, ++count);
	printDbg(DBG_INFO,"refernce released "<<count<<" times");
}


::Object * CXref::changeTrailer(char * name, ::Object * value)
{
using namespace debug;

	printDbg(DBG_DBG, "name="<<name<<" value type="<<value->getType());
	
	Dict * trailer = trailerDict.getDict(); 

	::Object * prev = trailer->update(name, value);

	return prev;
}

::Ref CXref::reserveRef()
{
using namespace debug;

	int i=1;
	int num=-1, gen;

	printDbg(DBG_DBG, "");
	
	// goes through entries array in XRef class (xref entries)
	// and reuses first free entry with increased gen number.
	for(; i<size; i++)
		if(entries[i].type==xrefEntryFree)
		{
			printDbg(DBG_DBG, "reusing entry");
			num=i;
			gen=entries[i].gen+1;
			break;
		}

	if(num==-1)
	{
		// no entry is free, so we have to add new number
		// TODO integer limitations
		num=i+1;
		gen=0;
		printDbg(DBG_DBG, "using new entry");
	}
	//
	// registers newly created object to the newStorage
	// flag is set to false now and this is changed only if
	// initialized value is overwritten by change method
	// all objects with false flag should be ignored when
	// writing to a file
	::Ref objRef={num, gen};
	newStorage.put(objRef, false);
	printDbg(DBG_INFO, "ref=["<<num<<", "<<gen<<"]"<<" registered to newStorage");

	return objRef;
}

::Object * CXref::createObject(ObjType type, ::Ref * ref)
{
using namespace debug;

	printDbg(DBG_DBG, "type="<<type);
	// reserves new reference for object 
	::Ref objRef=reserveRef();
	
	// it's not possible to create indirect reference value or
	// any other internaly (by xpdf) used object types
	// FIXME: why is this not compileable
	/*
	if(type==objRef || type==objError || type==objEOF || type==objNone)
		return 0;
	*/

	::Object * obj=(::Object *)gmalloc(sizeof(::Object));
	
	// initialize according type
	switch(type)
	{
		case objBool:
			obj->initBool(false);
			break;
		case objInt:
			obj->initInt(0);
			break;
		case objReal:
			obj->initReal(0);
			break;
		case objString:
			obj->initString(NULL);
			break;
		case objName:
			obj->initName(NULL);
			break;
		case objArray:
			obj->initArray(this);
			break;
		case objDict:
			obj->initDict(this);
			break;
		case objStream:
			obj->initStream(NULL);
			break;
		case objCmd:
			obj->initCmd(NULL);
			break;
		case objNull:
			obj->initNull();
			break;
		default:
			// TODO assert

		break;
	}
	

	// sets reference parameter if not null
	if(ref)
		*ref=objRef;

	printDbg(DBG_INFO, "new object created and initialized");
	return obj;
}

bool CXref::knowsRef(::Ref ref)
{
	// checks newly created object only with true flag
	// not found returns 0, so it's ok
	if(newStorage.get(ref))
		return true;

	// has to be found in entries
	if(entries[ref.num].type==xrefEntryFree)
		return false;
	// object number is ok, so also gen must fit
	return entries[ref.num].gen==ref.gen;
}

bool CXref::typeSafe(::Object * obj1, ::Object * obj2)
{
using namespace debug;

	if(!obj1 || !obj2)
	{
		if(!obj1)
			printDbg(DBG_ERR, "obj1 is null");
		
		if(!obj2)
			printDbg(DBG_ERR, "obj2 is null");
		return false;
	}

	::Object dObj1=*obj1, dObj2=*obj2;	// object for dereferenced 
						// values - we assume, it's
						// not indirect
	// types for direct values.
	::ObjType type1=obj1->getType(), type2=obj2->getType();
	
	// checks indirect
	if(type1==objRef)
	{
		printDbg(DBG_DBG, "obj1 is reference - need to dereference");
		fetch(dObj1.getRef().num, dObj1.getRef().gen, &dObj1);
		type1=dObj1.getType();
		printDbg(DBG_DBG, "obj1 target type="<< type1);
		// has to free object
		dObj1.free();
	}
	if(type2==objRef)
	{
		printDbg(DBG_DBG, "obj2 is reference - need to dereference");
		fetch(dObj2.getRef().num, dObj2.getRef().gen, &dObj2);
		type2=dObj2.getType();
		printDbg(DBG_DBG, "obj2 target type="<< type2);
		// has to free object
		dObj2.free();
	}

	// no we have direct values' types
	bool ret=type1==type2;
	
	// if these types are not same, one more situation may occure:
	// dereferenced obj1 is objRef (it is indirect) and dereferenced
	// object is objNull (it is not present in pdf). In such situation, type2
	// can be everything
	if(!ret && (obj1->getType() == objRef && type1 == objNull))
	{
		::Ref ref=obj1->getRef();
		printDbg(DBG_WARN, "obj1 ref=["<<ref.num<<", "<<ref.gen<<"] is not present in pdf. type2="<<type2<<" is safe");
		return true;
	}

	return ret;
}

::Object * CXref::getTrailerEntry(char * name)
{
using namespace debug;

	printDbg(DBG_DBG, "name="<<name);
	Dict * trailer = trailerDict.getDict();

	// lookupNF doesn't create deep copy, so 
	// we have to get value and then make deep copy
	// calling clone method. To keep clean reference counting
	// obj has to be freed
	::Object obj;
	trailer->lookupNF(name, &obj);

	::Object * retValue=obj.clone();
	obj.free();

	return retValue;
}

::Object *CXref::getDocInfo(::Object *obj)
{
using namespace debug;

	printDbg(DBG_DBG, "");
	// gets object
	::Object docObj;
	XRef::getDocInfo(&docObj);

	// creates deep copy and frees object from getDocInfo
	// and initialize parameter from cloned value
	::Object * retObj=docObj.clone();
	docObj.free();
	*obj=*retObj;

	return retObj;
}

::Object *CXref::getDocInfoNF(::Object *obj)
{
using namespace debug;

	printDbg(DBG_DBG, "");
	// gets object
	::Object docObj;
	XRef::getDocInfoNF(&docObj);

	// creates deep copy and frees object (because getDocInfoNF 
	// uses copy method) from getDocInfo
	// and initialize parameter from cloned value
	::Object * retObj=docObj.clone();
	docObj.free();
	// shallow copy of the content (deep copied)
	*obj=*retObj;

	return retObj;
}

::Object * CXref::fetch(int num, int gen, ::Object *obj)
{
using namespace debug;

	printDbg(DBG_DBG, "num="<<num<<" gen"<<gen);
	
	::Ref ref={num, gen};
	
	// tries to use cache
	// TODO uncomment when cache is ready
	/*
	if(cache)
	{
		printDbg(DBG_INFO, "[num="<<num<<" gen"<<gen<<"] is cached");
	}
	*/
	

	ObjectEntry * entry=changedStorage.get(ref);
	if(entry)
	{
		printDbg(DBG_INFO, "[num="<<num<<" gen"<<gen<<"] is changed - using changedStorage");
		
		// object has been changed
		::Object * deepCopy=entry->object->clone();

		// shallow copy of content
		// content is deep copy of found object, so
		// this doesn't affect our ::Object in changedStorage
		*obj=*deepCopy;
		
		// dellocate deepCopy - constructor doesn't
		// destroy content which is copied (shallow)
		// to obj
		delete deepCopy; 
		return obj;
	}

	printDbg(DBG_INFO, "[num="<<num<<" gen"<<gen<<"] is not changed - using Xref");
	// delegates to original implementation
	obj=XRef::fetch(num, gen, obj);

	// if object is not null, caches object's deep copy
	/* FIXME uncoment, when cache is ready
	if(cache && obj->getType()!=objNull)
		cache->put(ref, obj->clone());
	*/   
	return obj;
}

int CXref::getNumObjects() 
{ 
using namespace debug;

	printDbg(DBG_DBG, "");

	size_t newSize=0;
	ObjectStorage< ::Ref, bool, RefComparator>::Iterator begin, i;

	for(i=newStorage.begin(); i!=newStorage.end(); i++)
		if(i->second)
			newSize++;

	printDbg(DBG_INFO, "original objects count="<<XRef::getNumObjects()<<" newly created="<<newSize);
	return XRef::getNumObjects() + newSize;
}
