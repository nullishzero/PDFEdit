// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80
/*
 * =====================================================================================
 *	  Filename:  cpdf.cc
 *	   Description: CPdf class implementation.
 *	   Created:  01/28/2006 03:48:14 AM CET
 *		Author:  jmisutka (), 
 *	   Changes: 2006/1/28 added mapping support
 *				2006/1/30 tested objToString () -- ok
 *				2006/2/8  after a long battle against g++ and ld + collect I made
 *						  the implementation of CPdf a .cc file
 * =====================================================================================
 */

#include "static.h"

#include "iproperty.h"
#include "cobject.h"
#include "cpdf.h"
#include "factories.h"
#include "utils/debug.h"


// =====================================================================================


using namespace pdfobjects;

typedef std::vector<boost::shared_ptr<IProperty> > ChildrenStorage;

namespace utils
{

/** Helper method for getting int property value from dictionary.
 * @param name Name of the property in the dictionary.
 * @param dict Dictionary where to search.
 *
 * Gets property according name. Checks property type and if it is realy
 * pInt, gets its int value which is returned.
 *
 * @throw ObjInvalidPositionInComplex if property is not found.
 * @throw ObjMalformed if property is found but doesn't contain integer value.
 * @return int value of the property.
 */
int getIntFromDict(std::string name, boost::shared_ptr<CDict> dict)
{
	using namespace boost;

	shared_ptr<IProperty> prop_ptr=dict->getPropertyValue(name);	
	if(prop_ptr->getType() != pInt)
	{
		// malformed dictionary
		// TODO handle
	}

	shared_ptr<CInt> int_ptr=IProperty::getSmartCObjectPtr<CInt>(prop_ptr);
	int value;
	int_ptr->getPropertyValue(value);

	return value;
}

/** Helper method for getting string property value from dictionary.
 * @param name Name of the property in the dictionary.
 * @param dict Dictionary where to search.
 *
 * Gets property according name. Checks property type and if it is realy
 * pString, gets its string value which is returned.
 *
 * @throw ObjInvalidPositionInComplex if property is not found.
 * @throw ObjMalformed if property is found but doesn't contain string value.
 * @return std::string value of the property.
 */
std::string getStringFromDict(std::string name, boost::shared_ptr<CDict> dict)
{
	using namespace boost;

	shared_ptr<IProperty> prop_ptr=dict->getPropertyValue(name);	
	if(prop_ptr->getType() != pString)
	{
		// malformed dictionary
		// TODO handle
	}

	shared_ptr<CString> str_ptr=IProperty::getSmartCObjectPtr<CString>(prop_ptr);
	std::string value;
	str_ptr->getPropertyValue(value);

	return value;
}
	
/** Helper method for getting name property value from dictionary.
 * @param name Name of the property in the dictionary.
 * @param dict Dictionary where to search.
 *
 * Gets property according name. Checks property type and if it is realy
 * pName, gets its string value which is returned.
 *
 * @throw ObjInvalidPositionInComplex if property is not found.
 * @throw ObjMalformed if property is found but doesn't contain name value.
 * @return std::string value of the property.
 */
std::string getNameFromDict(std::string name, boost::shared_ptr<CDict> dict)
{
	using namespace boost;

	shared_ptr<IProperty> prop_ptr=dict->getPropertyValue(name);	
	if(prop_ptr->getType() != pName)
	{
		// malformed dictionary
		// TODO handle
	}

	shared_ptr<CName> name_ptr=IProperty::getSmartCObjectPtr<CName>(prop_ptr);
	std::string value;
	name_ptr->getPropertyValue(value);

	return value;
}

// TODO also for rest of simple objects

/** Helper method to find page at certain position.
 * @param CPdf Pdf instance where to search.
 * @param pagesDict Reference to or Page or Pages dictionary representing 
 * page node (see Pdf standard notes).
 * @param startPos Starting position for searching (see note below).
 * @param pos Page position (starting from 1) to find.
 *
 * Method recursively goes through pages dictionary until given page 
 * position is found or no such position can be found. If position can't be
 * found under given page node, exception is thrown.
 * <br>
 * If given pagesDict is reference, uses CPdf::getIndirectProperty to get 
 * dictionary.
 * 
 * <p>
 * <b>Pdf standard notes</b>:
 * <br>
 * Pdf tree structure contains of two types of page dictionaries. 
 * <ul>
 * <li>Pages dictionary - which is just intermediate node in the tree and
 * contains children nodes. These may be direct pages or another Pages node.
 * It doesn't represent page itself. All children are stored in Kids array.
 * Dictionary also contains Count information which holds number of all
 * Page dictionaries under this node.
 * <li>Page dictionary - which represents direct page.
 * </ul>
 * This structure is rather complex but enables effective way to access 
 * arbitrary page in short time (some applications provide balanced tree form
 * to enable very effective access).
 * <p>
 * <b>Implementation notes</b>:<br>
 * When starting to search from root of all pages, pagesDict should be supplied
 * from pdf->getPropertyValue("/Pages"). This is indirect reference (according
 * standard), but can be used in this method. startPos should be 1 (first page
 * is 1).
 * <br>
 * If we have some intermediate <b>Pages</b> dictionary, startPos should be
 * lowest page number under this tree branch. This usage is recomended only if
 * caller exactly knows what he is doing, otherwise page position is wrong.
 * <p>
 * TODO malformed pdf handling - exception
 * TODO exception handling description (element not found, element malformed)
 *
 * @return Dereferenced page (wrapped in shared_ptr) dictionary at given 
 * position.
 */
boost::shared_ptr<IProperty> findPage(CPdf * pdf, boost::shared_ptr<IProperty> pagesDict, size_t startPos, size_t pos)
{
using namespace boost;
using namespace std;

	if(startPos > pos)
	{
		// impossible to find such page
		// TODO handle
	}

	// dictionary smart pointer holder
	// it is initialized according pagesDict parameter - if it is reference
	// it has to be dereferenced
	shared_ptr<CDict> dict_ptr;
	PropertyType type=pagesDict->getType();

	// checks if given parameter is reference and if so, dereference it
	// using getIndirectProperty method and casts to dict_ptr
	// otherwise test if given type is dictionary and casts to dict_ptr
	if(type==pRef)
	{
		IndiRef ref;
		IProperty::getSmartCObjectPtr<CRef>(pagesDict)->getPropertyValue(ref);
		shared_ptr<IProperty> indirect_ptr=pdf->getIndirectProperty(ref);
		if(indirect_ptr->getType() != pDict)
		{
			// malformed pdf
			// TODO handle
		}
		dict_ptr=IProperty::getSmartCObjectPtr<CDict>(indirect_ptr);
	}else
	{
		if(type!=pDict)
		{
			// maloformed pdf
			// TODO handle
		}
		dict_ptr=IProperty::getSmartCObjectPtr<CDict>(pagesDict);
	}

	// gets type property from dictionary to find out what to do
	string dict_type=getNameFromDict("/Type", dict_ptr);
	
	// if type is Page then we have page dictionary and so start_pos and pos 
	// must match otherwise it is not possible to find pos page
	if(dict_type=="/Page")
	{
		// everything ok 
		if(startPos == pos)
			return dict_ptr;
		
		// unable to find
		// TODO handle - not able to find page in this subtree
	}

	// if type is Pages then we are in intermediate node
	// in this case we have to start to dereference children from Kids array.
	if(dict_type=="/Pages")
	{

		shared_ptr<CDict> pages_ptr=IProperty::getSmartCObjectPtr<CDict>(pagesDict);
		int count=getIntFromDict("/Count", pages_ptr);

		// check if this subtree contains enought direct pages 
		if(count + startPos <= pos )
		{
			// no way to find given position under this subtree
			// TODO handle
		}

		// PAGE IS IN THIS SUBTREE, we have to find where
		// from pages's Kids array
		// min_pos holds minimum position for actual child (at the begining
		// startPos value and incremented by page number in node which can't
		// contain pos (normal page 1 and Pages their count).
		ChildrenStorage children;
		pages_ptr->_getAllChildObjects(children);
		ChildrenStorage::iterator i;
		size_t min_pos=startPos;
		for(i=children.begin(); i!=children.end(); i++)
		{
			// all members of Kids array have to be referencies according
			// PDF specification
			if((*i)->getType() != pRef)
			{
				// malformed pdf
				// TODO handle
			}

			// we have to dereference and indirect object has to be dictionary
			// child_ptr finally contains this dictionary
			shared_ptr<CRef> ref_ptr=IProperty::getSmartCObjectPtr<CRef>(*i);
			IndiRef ref;
			ref_ptr->getPropertyValue(ref);
			shared_ptr<IProperty> target_ptr=pdf->getIndirectProperty(ref);
			if(target_ptr->getType() != pDict)
			{
				// malformed pdf
				// TODO handle
			}
			shared_ptr<CDict> child_ptr=IProperty::getSmartCObjectPtr<CDict>(target_ptr);

			// we can have page or pages dictionary in child_ptr
			// Type field determine kind of node
			string dict_type=getNameFromDict("/Type", child_ptr);

			// Page dictionary is ok if min_pos equals pos, 
			// otherwise it is skipped - can't use recursion here because it's
			// not an error that this page is not correct one
			// min_pos is incremented
			if(dict_type=="/Page")
			{
				if(min_pos == pos)
					return child_ptr;
				min_pos++;
				continue;
			}

			// Pages dictionary is checked for its page count and if pos can
			// be found there, starts recursion
			// Otherwise increment min_pos with its count and continues
			if(dict_type=="/Pages")
			{
				int count=getIntFromDict("/Count", child_ptr);

				if(min_pos + count > pos )
					// pos IS in this subtree
					return findPage(pdf, child_ptr, startPos+min_pos, pos);

				// pos is not in this subtree
				// updates min_pos with its count and continues
				min_pos+=count;

				continue;
			}

			// malformed pdf
			// TODO handle
		}

		// this should never happen, because given pos was in this subtree
		// according Count Pages field
		// PDF malformed 
		// TODO handle
	}
	
	// should not happen - malformed pdf document
	// TODO handle

}

} // end of utils namespace

void CPdf::initRevisionSpecific(::Dict * /*trailer*/)
{
	// cleanup indirect mapping
	// cleanup all returned pages + signalization that they are invalid.
	// cleanup all returned outlines  -------------||----------------- 
}

CPdf::CPdf(BaseStream * stream, FILE * file, OpenMode openMode)
{
	// gets xref writer - if error occures, exception is thrown 
	xref=new XRefWriter(stream);
	pdfFile=file;
	mode=openMode;

	// initializes revision specific data for the newest revision
	initRevisionSpecific(xref->getTrailerDict()->getDict());
}

CPdf::~CPdf()
{
	// TODO implementation
}


//
// 
//
boost::shared_ptr<IProperty> CPdf::getIndirectProperty(IndiRef ref)
{
	//printDbg (0,"getExistingProperty(" << ref.num << "," << ref.gen << ")");

	// find the key, if it exists
	IndirectMapping::iterator i = indMap.find(ref);
	if(i!=indMap.end())
		// mapping exists, so returns value
		return i->second;

	// mapping doesn't exist yet, so tries to create one
	// fetches object according reference
	Object obj;
	xref->fetch(ref.num, ref.gen, &obj);
	
	// creates cobject from value according type - indirect
	// parent is set to object reference (it is its own indirect parent)
	// created object is wrapped to smart pointer and if not pNull also added to
	// the mapping
	IProperty * prop=NULL;// FIXME use createObjFromXpdfObj(*this, obj, ref);
	boost::shared_ptr<IProperty> prop_ptr(prop);
	if(prop_ptr->getType() != pNull)
		indMap.insert(IndirectMapping::value_type(ref, prop_ptr));

	return prop_ptr;
}


//
// TODO remove - on next commit
//
void
CPdf::delIndMapping (const IndiRef& ref)
{
	// if there is such mapping that is a problem
	//printDbg (0,"");
	
	indMap.erase (ref);
}

/** Adds all indirect object from given property.
 * @param pdf Pdf file where to put indirect objects.
 * @param ip Property to examine.
 *
 * Ckecks given property for type and if it is reference, uses 
 * CPdf::addIndirectProperty to add indirect object to the given pdf file.
 * Otherwise checks if ip is complex type and if so, checks all children 
 * recursively (calls this method on each). If called method returns with
 * non NULL reference, changes child to contain new reference pointing to
 * newly created object. All other simple values are just ignored.
 * <br>
 * Given ip may change its child value (if contains reference).
 * <br>
 * Returned reference has to be deallocated by caller.
 * 
 * @return Pointer to new reference, if new indirect property was created
 * for child (this means that child is reference) or NULL otherwise.
 */ 
IndiRef * addReferencies(CPdf * pdf, boost::shared_ptr<IProperty> ip)
{
	//printDbg(DBG_DBG,"");

	PropertyType type=ip->getType();
	ChildrenStorage childrenStorage;

	switch(type)
	{
		case pRef:
		{
			//printDbg(DBG_DBG,"Property is reference - adding new indirect object");
			IndiRef indiRef=pdf->addIndirectProperty(ip);
			//printDbg(DBG_DBG,"");
			return new IndiRef(indiRef);
		}	
		// complex types (pArray and pDict) collects their children to the 
		// container
		case pArray:
			IProperty::getSmartCObjectPtr<CArray>(ip)->_getAllChildObjects(childrenStorage);
			break;
		case pDict:
			IProperty::getSmartCObjectPtr<CDict>(ip)->_getAllChildObjects(childrenStorage);
			break;

		// all other simple values are ok, nothing should return
		default:
			return NULL;
	}

	// go throught all collected children and recursively calls this
	// method on each. If return value is not NULL, new object has been
	// created and sets new reference for child
	ChildrenStorage::iterator i;
	for(i=childrenStorage.begin(); i!=childrenStorage.end(); i++)
	{
		IndiRef * ref=addReferencies(pdf, *i);
		if(ref)
		{
			// new reference for this child
			boost::shared_ptr<CRef> ref_ptr=IProperty::getSmartCObjectPtr<CRef>(*i);
			ref_ptr->writeValue(*ref);
			//printDbg(DBG_DBG,"Reference changed to ["+ref->num+", "+ref->gen+"]");
			delete ref;
			continue;
		}
	}

	// also complex object is same - all referencies in this subtree are added
	// in this moment
	return NULL;
}

//
//
//
IndiRef CPdf::addIndirectProperty(boost::shared_ptr<IProperty> ip)
{
	//printDbg(DBG_DBG, *ip);

	// place for propertyValue
	// it is ip by default
	boost::shared_ptr<IProperty> propValue=ip;
	
	// if we given ip is reference, we have to distinguish whether
	// it comes from same file (and then nothing is to do and we
	// just simply return ip reference) 
	// or from different file, when we need to dereference and after 
	// something else can be done
	PropertyType type=ip->getType();
	if(type==pRef)
	{
		IndiRef ref;
		boost::shared_ptr<CRef> cref_ptr=
			IProperty::getSmartCObjectPtr<CRef>(ip);
		cref_ptr->getPropertyValue(ref);
		
		// just returns reference
		if(ip->getPdf()==this)
		{
			//printDbg(DBG_WARN, "Reference is from this file, nothing is added.");
			return ref;
		}

		// dereference real value
		propValue=ip->getPdf()->getIndirectProperty(ref); 
	}

	// All complex property values from different pdfs (not only referencies 
	// handled above) have to be searched for referencies inside, those are
	// added too
	if(propValue->getPdf() != this)
	{
		//printDbg(DBG_DBG, "Adding property from different file.");
		boost::shared_ptr<IProperty> clone;
		switch(propValue->getType())
		{
			// creates local clone for complex types - to keep 
			// original value untouched - and adds all referencies
			// in subtree
			// clone will contain correct referencies after
			// addReferencies function returns so changes value of
			// propValue
			case pDict:
			case pArray:
				clone=propValue->clone();
				addReferencies(this, clone);
				propValue=clone;
				break;
			default:
				// just to be gcc happy and prints no warning for
				// unhandled members from enum
				// we don't have to do anything here because
				// simple values can't contain referencies
				break;
			
		}

		// propValue contans real object that we want to add now
	}


	// gets xpdf Object from propValue (which contain definitive value to
	// be stored), registers new reference on xref		  
	::Object * obj=propValue->_makeXpdfObject();
	::Ref xpdfRef=xref->reserveRef();
	//printDbg(DBG_DBG, "New reference reseved ["+ref.num+", "+gen+"]");
	xref->changeObject(xpdfRef.num, xpdfRef.gen, obj);

	// xpdf object has to be deallocated
	utils::freeXpdfObject(obj);

	// creates return value from xpdf reference structure
	// and returns
	IndiRef reference={xpdfRef.num, xpdfRef.gen};
	//printDbg(DBG_INFO, "New indirect object inserted with reference ["+ref.num+", "+gen+"]");
	return reference;
}


CPdf * CPdf::getInstance(const char * filename, OpenMode mode)
{
	//printDbg(DBG_DBG, "");
	
	// openMode is read-only by default
	const char * openMode="r";
	// if mode is ReadWrite, set to read-write mode starting at the 
	// begining.
	if(mode == ReadWrite)
		openMode="r+";

	// opens file and creates (xpdf) FileStream
	FILE * file=fopen(filename, openMode);
	if(!file)
	{
		//printDbg(DBG_ERR, "Unable to open file (reason="+strerror(errno)+")");
		// TODO some output
		// exception
	}
	//printDbg(DBG_DBG,"File \""+filename+"\" open successfully in mode="+openMode);
	
	Object obj;
	// NOTE: I didn't find meaning obj parameter meaning for BaseStream
	// maybe it is only bad design of BaseStream (only FilterStream uses
	// this object
	obj.initNull();
	BaseStream * stream=new FileStream(file, 0, gFalse, 0, &obj);
	//printDbg(DBG_DBG,"File stream created");

	// stream is ready, creates CPdf instance
	CPdf * instance=new CPdf(stream, file, mode);

	//printDbg(DBG_INFO, "Instance created successfully openMode="+openMode);

	return instance;
}

int CPdf::close(bool saveFlag)
{
	//printDbg(DBG_DBG, "");
	// saves if necessary
	if(saveFlag)
		save(NULL);
	
	// deletes this instance
	// all clean-up is made in destructor
	delete this;

	//printDbg(DBG_INFO, "Instance deleted.")
	return 0;
}

