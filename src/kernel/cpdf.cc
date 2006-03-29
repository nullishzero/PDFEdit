// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80
/*
 * $RCSfile$
 *
 * $Log$
 * Revision 1.17  2006/03/29 06:12:34  hockm0bm
 * consolidatePageTree method added
 * starting to use getPageFromRef
 *
 * Revision 1.16  2006/03/27 22:28:42  hockm0bm
 * consolidatePageList method added to CPdf
 *
 * Utils namespace:
 * isDescendant method for subtree information
 * getNodePosition method for CDict -> position
 * searchTreeNode helper method
 *
 *
 */

#include <errno.h>
#include "static.h"

#include "iproperty.h"
#include "cobject.h"
#include "cobjecthelpers.h"
#include "cpdf.h"
#include "factories.h"
#include "utils/debug.h"


// =====================================================================================


using namespace boost;
using namespace std;
using namespace debug;


// TODO exceptions unification

namespace pdfobjects
{

typedef std::vector<boost::shared_ptr<IProperty> > ChildrenStorage;

namespace utils 
{
	
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
 *
 * @throw PageNotFoundException if given position couldn't be found.
 * @throw ElementBadTypeException if some of required element has bad type.
 * @throw MalformedFormatExeption if page node count number doesn't match the
 * reality (page count number is not reliable).
 * @return Dereferenced page (wrapped in shared_ptr) dictionary at given 
 * position.
 */
boost::shared_ptr<IProperty> findPage(CPdf & pdf, boost::shared_ptr<IProperty> pagesDict, size_t startPos, size_t pos)
{

	printDbg(DBG_DBG, "startPos=" << startPos << " pos=" << pos);
	if(startPos > pos)
	{
		printDbg(DBG_ERR, "startPos > pos");
		// impossible to find such page
		throw PageNotFoundException(pos);
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
		printDbg(DBG_DBG, "pagesDict is reference");
		try
		{
			dict_ptr=getDictFromRef(pagesDict);
		}catch(ElementBadTypeException & e)
		{
			// malformed pdf
			throw MalformedFormatExeption("Reference target is not dictionary");
		}
	}else
	{
		if(type!=pDict)
		{
			printDbg(DBG_ERR, "pagesDict is not dictionary type="<< type);
			// maloformed pdf
			throw ElementBadTypeException("pagesDict");
		}
		dict_ptr=IProperty::getSmartCObjectPtr<CDict>(pagesDict);
	}

	// gets type property from dictionary to find out what to do
	string dict_type=getNameFromDict("/Type", dict_ptr);
	printDbg(DBG_DBG, "dictionary type=" << dict_type);
	
	// if type is Page then we have page dictionary and so start_pos and pos 
	// must match otherwise it is not possible to find pos page
	if(dict_type=="/Page")
	{
		printDbg(DBG_DBG, "Page node is direct page");
		// everything ok 
		if(startPos == pos)
		{
			printDbg(DBG_INFO, "Page found");
			return dict_ptr;
		}
		
		// unable to find
		printDbg(DBG_ERR, "Page not found startPos="<<startPos);
		throw PageNotFoundException(pos);
	}

	// if type is Pages then we are in intermediate node
	// in this case we have to start to dereference children from Kids array.
	if(dict_type=="/Pages")
	{
		printDbg(DBG_DBG, "Page node is intermediate");
		shared_ptr<CDict> pages_ptr=IProperty::getSmartCObjectPtr<CDict>(pagesDict);
		int count=getIntFromDict("/Count", pages_ptr);

		printDbg(DBG_DBG, "Node has " << count << " pages");
		// check if this subtree contains enought direct pages 
		if(count + startPos <= pos )
		{
			printDbg(DBG_ERR, "page can't be found under this subtree startPos=" << startPos);
			// no way to find given position under this subtree
			throw PageNotFoundException(pos);
		}

		// PAGE IS IN THIS SUBTREE, we have to find where
		// from pages's Kids array
		// min_pos holds minimum position for actual child (at the begining
		// startPos value and incremented by page number in node which can't
		// contain pos (normal page 1 and Pages their count).
		ChildrenStorage children;
		pages_ptr->_getAllChildObjects(children);
		ChildrenStorage::iterator i;
		size_t min_pos=startPos, index=0;
		for(i=children.begin(); i!=children.end(); i++, index++)
		{
			printDbg(DBG_DBG, index << " kid checking");
			// all members of Kids array have to be referencies according
			// PDF specification
			if((*i)->getType() != pRef)
			{
				// malformed pdf
				printDbg(DBG_ERR, "Kid must be reference");
				throw ElementBadTypeException(""+index);
			}

			// we have to dereference and indirect object has to be dictionary
			// child_ptr finally contains this dictionary
			shared_ptr<CDict> child_ptr; 
			try
			{
				child_ptr=getDictFromRef(*i);
			}catch(ElementBadTypeException & e)
			{
				// target is not an dictionary
				throw MalformedFormatExeption("Kid target is not dictionary");
			}

			// we can have page or pages dictionary in child_ptr
			// Type field determine kind of node
			string dict_type=getNameFromDict("/Type", child_ptr);
			printDbg(DBG_DBG, "kid type="<<dict_type);

			// Page dictionary is ok if min_pos equals pos, 
			// otherwise it is skipped - can't use recursion here because it's
			// not an error that this page is not correct one
			// min_pos is incremented
			if(dict_type=="/Page")
			{
				if(min_pos == pos)
				{
					printDbg(DBG_INFO, "page found");
					return child_ptr;
				}
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
			printDbg(DBG_ERR, "kid dictionary doesn't have correct Type field");
			throw ElementBadTypeException(""+index);
		}

		// this should never happen, because given pos was in this subtree
		// according Count Pages field
		// PDF malformed 
		printDbg(DBG_ERR, "pages count field is not correct");
		throw MalformedFormatExeption("Page node count number is not correct");
	}
	
	// should not happen - malformed pdf document
	printDbg(DBG_ERR, "pagesDict dictionary doesn't have correct Type field");
	throw ElementBadTypeException("pagesDict");

}

/** Searches node in page tree structure.
 * @param pdf Pdf where to search.
 * @param superNode Node which to search.
 * @param node Node to be found.
 * @param startValue Position for first found node in this superNode.
 *
 * At first checks if node and superNode are same nodes (uses == operator to
 * compare). Then tries to get Type of the dictionary. In Page case returns with
 * startValue if given nodes are same or 0 (page not found).
 * If type is Pages (intermediate node) goes through Kids array and recursively
 * calls this method for each element until recursion returns with non 0 result.
 * This means the end of recursion. startValue is actualized for each Kid's
 * element (Page element increases 1, Pages element /Count).
 *
 * @throw ElementBadTypeException
 * @throw ElementNotFoundException
 * @return Position of the node or 0 if node couldn't be found under this
 * superNode.
 */
size_t searchTreeNode(CPdf & pdf, shared_ptr<CDict> superNode, shared_ptr<CDict> node, size_t startValue)
{
	// TODO exceptions correct messages
	
	
	// if nodes are same, startValue is returned.
	if(superNode==node)
		return startValue;

	// gets type of dictionary
	string superNodeType=getNameFromDict("/Type", superNode);
	
	// if type is Page we return with 0, because node is not this superNode and
	// there is nowhere to go
	if(superNodeType=="/Page")
		return 0;

	if(superNodeType!="/Pages")
		throw ElementBadTypeException("/Node");

	// if type is Pages, we go through Kids array and calls recursively for each
	// element until first one returns with non 0. Otherwise startValue is
	// updated by child node pages count.
	shared_ptr<IProperty> arrayProp_ptr=superNode->getPropertyValue("/Kids");
	if(arrayProp_ptr->getType()!=pArray)		// TODO can be reference?
		throw ElementBadTypeException("Kids");
	shared_ptr<CArray> array_ptr=IProperty::getSmartCObjectPtr<CArray>(arrayProp_ptr);
	vector<shared_ptr<IProperty> > kidsContainer;
	array_ptr->_getAllChildObjects(kidsContainer);
	vector<shared_ptr<IProperty> >::iterator i;
	for(i=kidsContainer.begin(); i!=kidsContainer.end(); i++)
	{
		// each element has to be reference
		PropertyType type=(*i)->getType();
		if(type!=pRef)
			throw ElementBadTypeException("Kids");

		// dereference indirect object - this has to be indirect object
		IndiRef ref;
		IProperty::getSmartCObjectPtr<CRef>(*i)->getPropertyValue(ref);
		shared_ptr<IProperty> element_ptr=pdf.getIndirectProperty(ref);
		if(element_ptr->getType()!=pDict)
			throw ElementBadTypeException("");
		shared_ptr<CDict> elementDict_ptr=IProperty::getSmartCObjectPtr<CDict>(element_ptr);
		string elementDictType=getNameFromDict("/Type", elementDict_ptr);
		if(elementDictType=="/Page")
		{
			if(searchTreeNode(pdf, elementDict_ptr, node, startValue))
				return startValue;

			// this was not correct one, increments startValue
			startValue++;
			continue;
		}
		if(elementDictType=="/Pages")
		{
			size_t ret=searchTreeNode(pdf, elementDict_ptr, node, startValue);
			if(ret)
				return ret;

			// we haven't found, so startValue is actualized with /Count value
			// of this node
			startValue+=getIntFromDict("/Count", elementDict_ptr);
			continue;
		}

		// dictionary type is uncorrect
		throw ElementBadTypeException("/Kids");
	}

	return 0;
}

/** Gets position of given node.
 * @param pdf Pdf where to examine.
 * @param node Node to find (CRef or CDict instances).
 *
 * Gets pdf's document catalog Pages field, which is the root of page tree.
 * Also gets dictionary value from node (if reference, dereferencing is done)
 * and starts searching using searchTreeNode method. If this returns real (non 0
 * position) is returned, it is returned by this method too. Otherwise throws
 * exception.
 *
 * @throw PageNotFoundException
 * @throw ElementBadTypeException
 * @throw MalformedFormatExeption
 * @return Node position.
 */
size_t getNodePosition(CPdf & pdf, shared_ptr<IProperty> node)
{
	// node must by from given pdf
	if(node->getPdf()!=&pdf)
		throw PageNotFoundException(0);

	// gets page tree root - it has to be Reference to Dictionary
	shared_ptr<IProperty> rootRef_ptr=pdf.getDictionary()->getPropertyValue("/Pages");
	if(rootRef_ptr->getType()!=pRef)
		// reference is required
		throw ElementBadTypeException("/Pages");
	IndiRef rootRef;
	IProperty::getSmartCObjectPtr<CRef>(rootRef_ptr)->getPropertyValue(rootRef);
	shared_ptr<IProperty> rootDictProp_ptr=pdf.getIndirectProperty(rootRef);

	// rootDictProp_ptr must be CDict instance
	if(rootDictProp_ptr->getType()!=pDict)
		throw MalformedFormatExeption("Page tree root doesn't refer to dictionary");
	shared_ptr<CDict> rootDict_ptr=IProperty::getSmartCObjectPtr<CDict>(rootDictProp_ptr);
	
	// gets dictionary from node parameter. It can be reference and
	// dereferencing has to be done or direct dictionary - otherwise error is
	// reported
	PropertyType nodeType=node->getType();
	if(nodeType!=pRef && nodeType!=pDict)
		throw ElementBadTypeException("node");
	shared_ptr<CDict> nodeDict_ptr;
	if(nodeType==pRef)
	{
		// dereferencing has to be done
		IndiRef ref;
		IProperty::getSmartCObjectPtr<CRef>(node)->getPropertyValue(ref);
		shared_ptr<IProperty> nodeProp_ptr=pdf.getIndirectProperty(ref);
		if(nodeProp_ptr->getType()!=pDict)
			throw ElementBadTypeException("node");

		nodeDict_ptr=IProperty::getSmartCObjectPtr<CDict>(nodeProp_ptr);
	}else
		nodeDict_ptr=IProperty::getSmartCObjectPtr<CDict>(node);
		
	size_t pos=searchTreeNode(pdf, rootDict_ptr, nodeDict_ptr, 1);
	if(pos)
		return pos;

	// node not found
	throw PageNotFoundException(0);
}

/** Checks if given child is descendant of node with given reference.
 * @param pdf Pdf where to resolv referencies.
 * @param parent Reference of the parent.
 * @param child Dictionary of page(s) node.
 *
 * Compares child's parent field - if not found, imediately returns false - 
 * with given reference. If referencies are same, returns true. 
 * Othwerwise resolves child's parent and starts recursion with it as
 * new child paremeter (it is transitive relation). 
 * <br>
 * NOTE: this method doesn't perform any checking of parameters.
 * <br>
 * TODO implement with iteration - not recursion
 *
 * @throw MalformedFormatExeption if child contains /Parent field which doesn't
 * point to the dictionary.
 * @return true If given child belongs to parent subtree, false otherwise.
 */
bool isDescendant(CPdf & pdf, IndiRef parent, shared_ptr<CDict> child)
{
using namespace utils;

	// tries to get child's direct parent node
	// if not found, we catch the exception and return false
	// this should happen in root node
	IndiRef directParent;
	try
	{
		directParent=getRefFromDict("/Parent", child);
	}catch(std::exception & e)
	{
		// child is not really parent child
		return false;
	}

	// we have direct parent reference
	// if it is same as given parent, we are done and return true
	// FIXME uncomment when IndiRef== is defined
	/*
	if(parent==directParent)
		return true;
	 */

	// parent may be somewhere higher in the page tree
	// Gets direct parent indirect object from given pdf and checks its type
	// if it is NOT dictionary - we have malformed pdf - exception is thrown
	// otherwise starts recursion with direct parent dictionary
	shared_ptr<IProperty> directParent_ptr=pdf.getIndirectProperty(directParent);
	if(directParent_ptr->getType()!=pDict)
		throw MalformedFormatExeption("Page node parent field doesn't point to dictionary");
	return isDescendant(pdf, parent, IProperty::getSmartCObjectPtr<CDict>(directParent_ptr));
}

} // end of utils namespace

void CPdf::initRevisionSpecific()
{

	printDbg(DBG_DBG, "");


	// Clean up part:
	// =============
	
	// cleans up indirect mapping
	if(indMap.size())
	{
		printDbg(DBG_INFO, "Cleaning up indirect mapping with "<<indMap.size()<<" elements");
		indMap.clear();
	}

	// cleans up and invalidates all returned pages
	if(pageList.size())
	{
		printDbg(DBG_INFO, "Cleaning up pages list with "<<pageList.size()<<" elements");
		PageList::iterator i;
		for(i=pageList.begin(); i!=pageList.end(); i++)
		{
			printDbg(DBG_DBG, "invalidating page at pos="<<i->first);
			// FIXME uncoment when method is reay
			//i->second->invalidate();
		}
		pageList.clear();
	}
	
	// cleanup all returned outlines  -------------||----------------- 
	
	// Initialization part:
	// ===================
	
	// initialize trailer dictionary from xpdf trailer dictionary object
	// no free should be called because trailer is returned directly from XRef
	Object * trailerObj=xref->getTrailerDict();
	trailer=boost::shared_ptr<CDict>(CDictFactory::getInstance(*trailerObj));
	
	// Intializes document catalog dictionary.
	// gets Root field from trailer, which should contain reference to catalog.
	// If no present or not reference, we have corrupted PDF file and exception
	// is thrown
	IndiRef rootRef=utils::getRefFromDict("/Root", trailer);
	shared_ptr<IProperty> prop_ptr=getIndirectProperty(rootRef);
	if(prop_ptr->getType()!=pDict)
	{
		printDbg(DBG_CRIT, "Trailer dictionary doesn't point to correct document catalog.");
		throw ElementBadTypeException("Root");
	}
	printDbg(DBG_INFO, "Document catalog successfully fetched");
	docCatalog=IProperty::getSmartCObjectPtr<CDict>(prop_ptr);
	
}

CPdf::CPdf(BaseStream * stream, FILE * file, OpenMode openMode)
{
	// gets xref writer - if error occures, exception is thrown 
	xref=new XRefWriter(stream);
	pdfFile=file;
	mode=openMode;

	// initializes revision specific data for the newest revision
	initRevisionSpecific();
}

CPdf::~CPdf()
{
	printDbg(DBG_DBG, "");

	// indirect mapping is cleaned up automaticaly
	
	// TODO implementation
	// discards all returned pages and outlines
}


//
// 
//
boost::shared_ptr<IProperty> CPdf::getIndirectProperty(IndiRef ref)
{
using namespace debug;

	printDbg (0,ref.num << "," << ref.gen );

	// find the key, if it exists
	IndirectMapping::iterator i = indMap.find(ref);
	if(i!=indMap.end())
	{
		// mapping exists, so returns value
		printDbg(DBG_DBG, "mapping exists");
		return i->second;
	}

	printDbg(DBG_DBG, "mapping doesn't exist")

	// mapping doesn't exist yet, so tries to create one
	// fetches object according reference
	Object obj;
	xref->fetch(ref.num, ref.gen, &obj);
	
	// creates cobject from value according type - indirect
	// parent is set to object reference (it is its own indirect parent)
	// created object is wrapped to smart pointer and if not pNull also added to
	// the mapping
	IProperty * prop=utils::createObjFromXpdfObj(*this, obj, ref);
	boost::shared_ptr<IProperty> prop_ptr(prop);
	if(prop_ptr->getType() != pNull)
	{
		indMap.insert(IndirectMapping::value_type(ref, prop_ptr));
		printDbg(DBG_DBG, "Mapping created");
	}

	return prop_ptr;
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
	printDbg(debug::DBG_DBG,"");

	PropertyType type=ip->getType();
	ChildrenStorage childrenStorage;

	switch(type)
	{
		case pRef:
		{
			printDbg(debug::DBG_DBG,"Property is reference - adding new indirect object");
			IndiRef indiRef=pdf->addIndirectProperty(ip);
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
			printDbg(debug::DBG_DBG,"Reference changed to [" << ref->num << ", " <<ref->gen << "]");
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
	printDbg(debug::DBG_DBG, "");

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
			printDbg(debug::DBG_WARN, "Reference is from this file, nothing is added.");
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
		printDbg(debug::DBG_DBG, "Adding property from different file.");
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
	printDbg(debug::DBG_DBG, "New reference reseved ["<<xpdfRef.num<<", "<<xpdfRef.gen<<"]");
	xref->changeObject(xpdfRef.num, xpdfRef.gen, obj);

	// xpdf object has to be deallocated
	utils::freeXpdfObject(obj);

	// creates return value from xpdf reference structure
	// and returns
	IndiRef reference={xpdfRef.num, xpdfRef.gen};
	printDbg(debug::DBG_INFO, "New indirect object inserted with reference ["<<xpdfRef.num<<", "<<xpdfRef.gen<<"]");
	return reference;
}


CPdf * CPdf::getInstance(const char * filename, OpenMode mode)
{
	printDbg(debug::DBG_DBG, "");
	
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
		printDbg(debug::DBG_ERR, "Unable to open file (reason="<<strerror(errno)<<")");
		// TODO some output
		// exception
	}
	printDbg(debug::DBG_DBG,"File \"" << filename << "\" open successfully in mode=" << openMode);
	
	Object obj;
	// NOTE: I didn't find meaning obj parameter meaning for BaseStream
	// maybe it is only bad design of BaseStream (only FilterStream uses
	// this object
	obj.initNull();
	BaseStream * stream=new FileStream(file, 0, gFalse, 0, &obj);
	printDbg(debug::DBG_DBG,"File stream created");

	// stream is ready, creates CPdf instance
	CPdf * instance=new CPdf(stream, file, mode);

	printDbg(debug::DBG_INFO, "Instance created successfully openMode=" << openMode);

	return instance;
}

int CPdf::close(bool saveFlag)
{
	printDbg(debug::DBG_DBG, "");
	// saves if necessary
	if(saveFlag)
		save(NULL);
	
	// deletes this instance
	// all clean-up is made in destructor
	delete this;

	printDbg(debug::DBG_INFO, "Instance deleted.")
	return 0;
}

boost::shared_ptr<CPage> CPdf::getPage(size_t pos)
{
using namespace utils;

	printDbg(DBG_DBG, "");

	if(pos>getPageCount())
	{
		printDbg(DBG_ERR, "Page out of range pos="<<pos);
		throw PageNotFoundException(pos);
	}

	// checks if page is available in pageList
	PageList::iterator i;
	if((i=pageList.find(pos))!=pageList.end())
	{
		printDbg(DBG_INFO, "Page at pos="<<pos<<" found in pageList");
		return i->second;
	}

	// page is not available in pageList, searching has to be done
	// gets Pages field from document catalog (no special checking has to be
	// done, because everything is done in getPageCount method).
	shared_ptr<CRef> rootPages_ptr=IProperty::getSmartCObjectPtr<CRef>(docCatalog->getPropertyValue("/Pages"));
	// find throws an exception if any problem found, otherwise pageDict_ptr
	// contians Page dictionary at specified position.
	shared_ptr<CDict> pageDict_ptr=IProperty::getSmartCObjectPtr<CDict>(findPage(*this, rootPages_ptr, 1, pos));

	// creates CPage instance from page dictionary and stores it to the pageList
	CPage * page=CPageFactory::getInstance(pageDict_ptr);
	shared_ptr<CPage> page_ptr(page);
	pageList.insert(PageList::value_type(pos, page_ptr));

	return page_ptr;
}

unsigned int CPdf::getPageCount()
{
using namespace utils;
	
	// gets Pages field from document catalog
	shared_ptr<IProperty> pages_ptr=docCatalog->getPropertyValue("/Pages");
	
	// gets type of the dictionary and find out if it is Page node or
	// intermediate node
	shared_ptr<CDict> dict_ptr=getDictFromRef(pages_ptr);
	string dictType=getNameFromDict("/Type", dict_ptr);
	if(dictType=="/Page")
	{
		// this document contains onlu one page
		return 1;
	}
	if(dictType=="/Pages")
	{
		// gets count field
		return getIntFromDict("/Count", dict_ptr);
	}

	printDbg(DBG_CRIT, "Pages dictionary has bad dictionary type type="<<dictType);
	throw MalformedFormatExeption("Pages dictionary is not Page or Pages dictionary");
}

boost::shared_ptr<CPage> CPdf::getNextPage(boost::shared_ptr<CPage> page)
{
	printDbg(DBG_DBG, "");

	size_t pos=getPagePosition(page);
	printDbg(DBG_DBG, "Page position is "<<pos);
	pos++;
	
	// checks if we are in boundary after incrementation
	if(pos>=getPageCount())
	{
		printDbg(DBG_ERR, "Page is out of range pos="<<pos);
		throw PageNotFoundException(pos);
	}

	// page in range, uses getPage
	return getPage(pos);

}

boost::shared_ptr<CPage> CPdf::getPrevPage(boost::shared_ptr<CPage> page)
{
	printDbg(DBG_DBG, "");

	size_t pos=getPagePosition(page);
	printDbg(DBG_DBG, "Page position is "<<pos);
	pos--;
	
	// checks if we are in boundary after incrementation
	if(pos>=getPageCount())
	{
		printDbg(DBG_ERR, "Page is out of range pos="<<pos);
		throw PageNotFoundException(pos);
	}

	// page in range, uses getPage
	return getPage(pos);
}

int CPdf::getPagePosition(boost::shared_ptr<CPage> page)
{
	printDbg(DBG_DBG, "");
		
	PageList::iterator i;
	for(i=pageList.begin(); i!=pageList.end(); i++)
	{
		// checks page
		if(i->second == page)
		{
			printDbg(DBG_INFO, "Page found pos="<<i->first);
			return i->first;
		}
	}

	// page not found, it hasn't been returned by this pdf
	throw PageNotFoundException();
}


void CPdf::consolidatePageList(shared_ptr<IProperty> oldValue, shared_ptr<IProperty> newValue)
{
using namespace utils;

	// correction for all pages affected by this subtree change
	int difference=0;

	// position of first page which has to be consolidated because of value
	// change
	size_t minPos=0;

	// handles original value - one before change
	// pNull means no previous value available (new sub tree has been added)
	if(oldValue->getType()!=pNull)
	{
		// gets dictionary type - it has to be page or pages node
		shared_ptr<CDict> oldDict_ptr=getDictFromRef(oldValue);
		string oldDictType=getNameFromDict("/Type", oldDict_ptr);

		// simple page is compared with all from pageList and if found, removes
		// it from list and invalidates it.
		// Difference is set to - 1, because one page is removed 
		if(oldDictType=="/Page")
		{
			difference = -1;

			PageList::iterator i;
			for(i=pageList.begin(); i!=pageList.end(); i++)
			{
				// checks page's dictionary with old one
				if(i->second->getDictionary() == oldDict_ptr)
				{
					// dictionary is ok
					// FIXME: uncoment when method is ready
					//i->second->invalidate();
					pageList.erase(i);
					break;
				}
			}
		}else
		{
			// Pages dictionary stands for intermediate node and so all CPages
			// from this sub tree are removed and invalidated
			// difference is set to -/Count value (those number of pages are
			// removed)
			if(oldDictType=="/Pages")
			{
				difference = -getIntFromDict("/Count", oldDict_ptr);

				// gets reference of oldValue - which is the root removed
				// subtree
				IndiRef ref;
				IProperty::getSmartCObjectPtr<CRef>(oldValue)->getPropertyValue(ref);
				
				PageList::iterator i;
				for(i=pageList.begin(); i!=pageList.end(); i++)
				{
					// checks page's dictionary whether it is in oldDict_ptr sub
					// tree and if so removes it from pageList
					if(isDescendant(*this, ref, i->second->getDictionary()))
					{
						// updates minPos with page position (if greater)
						// +1 is added to keep idea of minimal position of first
						// page which has to be consolidated
						if(i->first+1 > minPos)
							minPos=i->first+1;
						
						pageList.erase(i);
					}
				}
			}
		}
	}

	// oldValue subtree (if any) is consolidated now

	// number of added pages by newValue tree
	int pagesCount=0;
	
	// handles new value - one after change
	// if pNull - no new value is available (subtree has been removed)
	if(newValue->getType()!=pNull)
	{
		// gets dictionary type
		shared_ptr<CDict> newDict_ptr=getDictFromRef(newValue);
		string newDictType=getNameFromDict("/Type", newDict_ptr);

		// page type adds only one page
		if(newDictType=="/Page")
			pagesCount=1;

		// pages type adds /Count pages
		if(newDictType=="/Pages")
			pagesCount=getIntFromDict("/Count", newDict_ptr);

		// gets position of this node and adds number of pages under this
		// subtree to find out which pages has to be consolidated
		minPos = getNodePosition(*this, newValue) + pagesCount;
	}

	// corrects difference with added pages
	difference -= pagesCount;

	// no difference means no speacial handling for other pages
	// we have replaced old sub tree with new subtree with same number of pages
	if(difference==0)
		return;

	// minPos contains first position of page which has to be consolidated (if 
	// it is in the list). 
	PageList::iterator i;
	PageList readdContainer;
	for(i=pageList.begin(); i!=pageList.end(); i++)
	{
		// all pages >= minPos are removed and readded
		if(i->first >= minPos)
		{
			// collects all removed
			readdContainer.insert(PageList::value_type(i->first, i->second));	
			pageList.erase(i);
		}
	}
	
	// checks minPos==0 and if so, we have to handle situation special way,
	// because don't have any information about previous position of oldValue
	// subtree. In such case all pages which should be readded are searched in
	// page tree and actual position is used.
	if(!minPos)
	{
		for(i=readdContainer.begin(); i!=readdContainer.end(); i++)
		{
			// uses getNodePosition for each page's dictionary to find out
			// current position
			size_t pos=getNodePosition(*this, i->second->getDictionary());
			pageList.insert(PageList::value_type(pos, i->second));	
		}
		return;
	}
	
	// Information about page numbers which should be consolidated is available
	// so just adds difference for each in readdContainer
	// readds all removed with changed position (according difference)
	for(i=readdContainer.begin(); i!=readdContainer.end(); i++)
		pageList.insert(PageList::value_type(i->first+difference, i->second));	
}


void CPdf::consolidatePageTree(boost::shared_ptr<CDict> interNode)
{
using namespace utils;

	shared_ptr<IProperty> kidsArrayProp_ptr=interNode->getPropertyValue("/Kids");
	if(kidsArrayProp_ptr->getType()!=pArray)
	{
		// Kids field value must be aray
		throw ElementBadTypeException("/Kids");
	}

	// gets all children from array
	shared_ptr<CArray> kidsArray_ptr=IProperty::getSmartCObjectPtr<CArray>(kidsArrayProp_ptr);
	vector<shared_ptr<IProperty> > kidsContainer;
	kidsArray_ptr->_getAllChildObjects(kidsContainer);

	// iterate through all children, dereference all of them and checks
	// dictionary type. This used to get information about page count under this
	// node (Page is 1 and Pages is /Count)
	size_t count=0;
	vector<shared_ptr<IProperty> >::iterator i;
	for(i=kidsContainer.begin(); i!=kidsContainer.end(); i++)
	{
		if((*i)->getType()!=pRef)
		{
			// each element must be reference
			throw ElementBadTypeException("/Kids");
		}

		shared_ptr<CDict> kidDict_ptr=getDictFromRef(*i);
		string dictType=getNameFromDict("/Type", kidDict_ptr);
		if(dictType=="/Page")
			count++;
		if(dictType=="/Pages")
			count+=getIntFromDict("/Count", kidDict_ptr);
		
		// checks parentIProperty::getSmartCObjectPtr<CDict>(kidProp_ptr)
		try
		{
			IndiRef parentRef;
			parentRef=getRefFromDict("/Parent", kidDict_ptr);
			if(! (parentRef==interNode->getIndiRef()))
				kidDict_ptr->setPropertyValue("/Parent", *CRefFactory::getInstance(interNode->getIndiRef()));
			
		}catch(exception & e)
		{
			// no parent field found or it is not reference
			// TODO handle in non critical way - correct value
			throw ElementBadTypeException("/Parent");
		}
		// TODO handle bad dictionary type

	}

	// count is collected for this node, sets value and distribute it to all
	// parents
	CInt countProp(count);
	interNode->setPropertyValue("/Count", countProp);
	
	// distribute value also to fathers
	shared_ptr<IProperty> parentRef_ptr=interNode->getPropertyValue("/Parent");
	shared_ptr<CDict> parentDict_ptr=getDictFromRef(parentRef_ptr);

	// calls recursively
	consolidatePageTree(parentDict_ptr);
}

} // end of pdfobjects namespace
