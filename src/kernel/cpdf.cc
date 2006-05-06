// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80
/*
 * $RCSfile$
 *
 * $Log$
 * Revision 1.44  2006/05/06 08:38:19  hockm0bm
 * * CPdf constructor sets mode of xref according its mode
 * * CPdf::consolidatePageList bug fixed
 * 	- readdContainer has to contain all pages with pos >= minPos (not only >)
 *
 * Revision 1.43  2006/05/05 11:55:55  petrm1am
 *
 * Commiting changes sent by Michal:
 *
 * cpdf.cc:
 * * consolidatePageList bug fixed
 *     - difference += pageCount not -=
 * * debug messages improved
 *
 * cxref.cc:
 * * reserveRef bug fix
 *     - totalCount replaced by xrefCount from XRef::getNumObjects
 *     - num is initialized to -1
 *     - kernelPrintDbg message wrong position (cannot use new entry)
 *
 * Revision 1.42  2006/05/01 13:53:07  hockm0bm
 * new style printDbg
 *
 * Revision 1.41  2006/04/28 17:15:50  hockm0bm
 * * helper functions exported by header file now
 *
 * Revision 1.40  2006/04/27 18:29:10  hockm0bm
 * * all methods which could be set to const are const now
 * * pageList is mutable to enable getPage (and all depended in getPage)
 *   to be const methods
 * * pageCount is mutable to enable getPageCount to be conts
 * * methods which should fail in read-only mode doesn't compare mode
 *   directly but use getMode - because it consideres also current revision
 * * changeIndirectProperty bug fixed - checking for mode is done now
 *
 * Revision 1.39  2006/04/27 05:55:39  hockm0bm
 * * changeRevision implemented
 * * documentation update
 *
 * Revision 1.38  2006/04/23 15:12:58  hockm0bm
 * changeIndirectProperty behaviour changed
 *         - indirect mapping removed only if given property is different
 *           instance than given one
 *         - throws CObjInvalidObject exception is thrown if prop is not from this
 *           pdf or mapping doesn't exist
 *
 * Revision 1.37  2006/04/23 13:14:02  hockm0bm
 * clone method implemented
 *
 * Revision 1.36  2006/04/22 20:12:50  hockm0bm
 * Last commit was accident
 * getInstance catches exceptions possibly thrown in CPdf and throws
 *         PdfOpenException
 *
 * Revision 1.35  2006/04/22 20:07:08  hockm0bm
 * bug fix
 *         - getPrevPage, getNextPage didn't return true if successful
 *
 * Revision 1.34  2006/04/22 17:22:14  hockm0bm
 * * getPageCount caches/uses value to/from pageCount field
 * * getNextPage, getPrevPage boundary checking corrected
 *
 * Revision 1.33  2006/04/21 20:40:02  hockm0bm
 * * changeIndirectProperty use freeXpdfObject with out problems
 * * PageTreeWatchDog::notify bug fix
 * 	- if newValue is CNull page tree consolidation wasn't done.
 * 	  Uses oldValue parent if new value is CNull
 * 	- additional checking for proper old and new values
 * 	- code reorganization and simplification
 * * caches page number in field pageCount
 * 	- invalidation is done in consolidatePageTree and initRevisionSpecific
 *
 * Revision 1.32  2006/04/20 22:34:37  hockm0bm
 * freeXpdfObject cloning seems to be problem (for array - TODO solve)
 *         - just to prevent SEGV (this solution means memmory leak)
 *
 * Revision 1.31  2006/04/20 18:04:36  hockm0bm
 * ambigues spell error corrected
 *
 * Revision 1.30  2006/04/19 18:46:12  hockm0bm
 * * getPropertyValue replaced by getValueFromSimple helper
 * * minor changes in searchTreeNode
 * 	- getPropertyId is used
 * 	- throws AmbiguesPageTreeException if not able to get node
 * 	  position because of Kids array ambiguity
 * * insertPage, removePage throws AmbiguesPageTreeException (same as above)
 *         - uses getPropertyId used
 *
 * Revision 1.29  2006/04/19 05:57:46  hockm0bm
 * * pageTreeWatchDog wrapped by shared_ptr
 * * print messages consolidated a bit
 * * exception handling and error prone in page tree functions
 *
 * Revision 1.28  2006/04/17 20:11:47  hockm0bm
 * * OpenMode reorganized
 *         - ReadOnly is first now
 *         - values are ordered according what everything can be done with file
 * * getInstance corrected
 *         - file is opened in append mode (r+) anytime mode is >=ReadWrite
 *           (also for advanced - this didn't work in previous version)
 *
 * Revision 1.27  2006/04/15 08:01:32  hockm0bm
 * * pdfFile field removed
 *         - we are using transparent StreamWriter now
 * * PageTreeWatchDog class implemented as Observer handling page tree changes.
 * * changeIndirectProperty method added
 *         - registers changed property to the XRefWriter
 *
 * Revision 1.26  2006/04/13 18:16:30  hockm0bm
 * insert/removePage, addIndirectProperty, save throws ReadOnlyDocumentException
 *
 * Revision 1.25  2006/04/12 17:55:21  hockm0bm
 * * save method changed
 * 		- fileName parameter removed
 * 		- newRevision parameter added
 * * new method clone
 * * getPage method tested (everything based on this method
 *   should be correct too)
 * * getNodePosition handle ambigues problem
 * 		- not soved yet - new exception should be created
 * * all methods using getNodePosition also handles
 *   problem with ambigues position of page dictionary
 * * registerPageTreeObserver new method
 * * constructor's BaseStream parameter changed to StreamWriter
 *   due to XRefWriter constructor change
 *
 * Revision 1.24  2006/04/01 20:31:15  hockm0bm
 * findPageDict
 *         - Kids array handling errors corrected
 *         - comments updated
 * testing start
 *
 * Revision 1.23  2006/04/01 17:47:29  hockm0bm
 * BUG fixed in findPageDict:
 * 189: shared_ptr<CDict> pages_ptr=IProperty::getSmartCObjectPtr<CDict>(pagesDict);
 *      int count=getIntFromDict("Count", pages_ptr);
 * Problem:
 *      pagesDict is parameter and may be dictionary or reference (in this case
 *      reference). Casting didn't fail so it was used!!!
 * Solution:
 *      dict_ptr is dereferenced dictionary for node.
 *
 * Revision 1.22  2006/04/01 16:45:17  hockm0bm
 * getInstance throws PdfOpenException when file open fails
 *
 * Revision 1.21  2006/03/31 22:36:52  hockm0bm
 * names changed from /Name -> Name
 *
 * Revision 1.20  2006/03/31 21:03:00  hockm0bm
 * removePage implemented
 *
 * Revision 1.19  2006/03/30 23:21:14  misuj1am
 *
 *
 *  -- getPropertyValue -> getProperty
 *
 * Revision 1.18  2006/03/30 21:29:33  hockm0bm
 * * findPage renamed to findPageDict
 *         - return value changed to shared_ptr<CDict>
 * * TODOs for exceptions unification
 * * code simplification for dictionary handling
 *         - using utils::getDictFromRef more often
 * * printDbg for more methods
 * * insertPage implemented
 *
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
	
boost::shared_ptr<CDict> findPageDict(const CPdf & pdf, boost::shared_ptr<IProperty> pagesDict, size_t startPos, size_t pos)
{
	// TODO error handling unification
	
	utilsPrintDbg(DBG_DBG, "startPos=" << startPos << " pos=" << pos);
	if(startPos > pos)
	{
		utilsPrintDbg(DBG_ERR, "startPos > pos");
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
		utilsPrintDbg(DBG_DBG, "pagesDict is reference");
		try
		{
			dict_ptr=getDictFromRef(pagesDict);
		}catch(ElementBadTypeException & e)
		{
			// malformed pdf
			utilsPrintDbg(DBG_ERR, "pagesDict doesn't refer to dictionary");
			throw ElementBadTypeException("pagesDict");
		}
	}else
	{
		if(type!=pDict)
		{
			utilsPrintDbg(DBG_ERR, "pagesDict is not dictionary type="<< type);
			// maloformed pdf
			throw ElementBadTypeException("pagesDict");
		}
		dict_ptr=IProperty::getSmartCObjectPtr<CDict>(pagesDict);
	}

	// gets type property from dictionary to find out what to do
	string dict_type=getNameFromDict("Type", dict_ptr);
	utilsPrintDbg(DBG_DBG, "dictionary type=" << dict_type);
	
	// if type is Page then we have page dictionary and so start_pos and pos 
	// must match otherwise it is not possible to find pos page
	if(dict_type=="Page")
	{
		utilsPrintDbg(DBG_DBG, "Page node is direct page");
		// everything ok 
		if(startPos == pos)
		{
			utilsPrintDbg(DBG_INFO, "Page found");
			return dict_ptr;
		}
		
		// unable to find
		utilsPrintDbg(DBG_ERR, "Page not found startPos="<<startPos);
		throw PageNotFoundException(pos);
	}

	// if type is Pages then we are in intermediate node
	// in this case we have to start to dereference children from Kids array.
	if(dict_type=="Pages")
	{
		utilsPrintDbg(DBG_DBG, "Page node is intermediate");
		int count=getIntFromDict("Count", dict_ptr);

		utilsPrintDbg(DBG_DBG, "Node has " << count << " pages");
		// check if this subtree contains enought direct pages 
		if(count + startPos <= pos )
		{
			utilsPrintDbg(DBG_ERR, "page can't be found under this subtree startPos=" << startPos);
			// no way to find given position under this subtree
			throw PageNotFoundException(pos);
		}

		// PAGE IS IN THIS SUBTREE, we have to find where
		
		// gets Kids array from pages dictionary and gets all its children
		shared_ptr<IProperty> kidsProp_ptr=dict_ptr->getProperty("Kids");
		if(kidsProp_ptr->getType()!=pArray)
		{
			utilsPrintDbg(DBG_ERR,"Kids field is not an array. type="<<kidsProp_ptr->getType());
			throw ElementBadTypeException("Kids");
		}
		shared_ptr<CArray> kids_ptr=IProperty::getSmartCObjectPtr<CArray>(kidsProp_ptr);
		ChildrenStorage children;
		kids_ptr->_getAllChildObjects(children);

		// min_pos holds minimum position for actual child (at the begining
		// startPos value and incremented by page number in node which can't
		// contain pos (normal page 1 and Pages their count).
		ChildrenStorage::iterator i;
		size_t min_pos=startPos, index=0;
		for(i=children.begin(); i!=children.end(); i++, index++)
		{
			shared_ptr<IProperty> child=*i;

			PropertyType childType=child->getType();
			utilsPrintDbg(DBG_DBG, index << " kid checking if it is reference");
			// all members of Kids array have to be referencies according
			// PDF specification
			if((*i)->getType() != pRef)
			{
				// malformed pdf
				utilsPrintDbg(DBG_ERR, "Kid["<<index<<"] is not reference. type="<<(*i)->getType());
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
				// element is not dictionary, we will just print warning
				utilsPrintDbg(DBG_WARN, "Target of Kids["<<index<<"] is not dictionary.");
				continue;
			}
			
			// we can have page or pages dictionary in child_ptr
			// Type field determine kind of node
			string dict_type=getNameFromDict("Type", child_ptr);
			utilsPrintDbg(DBG_DBG, "kid type="<<dict_type);

			// Page dictionary is ok if min_pos equals pos, 
			// otherwise it is skipped - can't use recursion here because it's
			// not an error that this page is not correct one
			// min_pos is incremented
			if(dict_type=="Page")
			{
				if(min_pos == pos)
				{
					utilsPrintDbg(DBG_INFO, "page found");
					return child_ptr;
				}
				min_pos++;
				continue;
			}

			// Pages dictionary is checked for its page count and if pos can
			// be found there, starts recursion
			// Otherwise increment min_pos with its count and continues
			if(dict_type=="Pages")
			{
				int count=getIntFromDict("Count", child_ptr);

				if(min_pos + count > pos )
					// pos IS in this subtree
					return findPageDict(pdf, child_ptr, min_pos, pos);

				// pos is not in this subtree
				// updates min_pos with its count and continues
				min_pos+=count;

				continue;
			}

			// malformed pdf
			// we will, just print warning
			utilsPrintDbg(DBG_WARN, "kid dictionary doesn't have correct Type field");
		}

		// this should never happen, because given pos was in this subtree
		// according Count Pages field
		// PDF malformed 
		utilsPrintDbg(DBG_ERR, "pages count field is not correct");
		throw MalformedFormatExeption("Page node count number is not correct");
	}
	
	// should not happen - malformed pdf document
	utilsPrintDbg(DBG_ERR, "pagesDict dictionary doesn't have correct Type field");
	throw ElementBadTypeException("pagesDict");

}

/** Searches node in page tree structure.
 * @param pdf Pdf where to search.
 * @param superNode Node which to search.
 * @param node Node to be found.
 * @param startValue Position for first found node in this superNode.
 *
 * At first checks if node and superNode are same nodes (uses == operator to
 * compare). Then tries to get Type of the superNode dictionary. In Page case 
 * returns with startValue if given nodes are same or 0 (page not found).
 * If type is Pages (intermediate node) goes through Kids array and recursively
 * calls this method for each element until recursion returns with non 0 result.
 * This means the end of recursion. startValue is actualized for each Kid's
 * element (Page element increases 1, Pages element Count).
 * <br>
 * If node is found as direct Kids member (this means that reference of target 
 * node is direct memeber of Kids array), then determines if the node position 
 * is unambiguous - checks whether reference to the node is unique in Kids array. 
 * If not throws exception. This means that searchTreeNode function is not able 
 * to definitively determine node's position.
 *
 * @throw ElementBadTypeException If required element of pdf object has bad
 * type.
 * @throw ElementNotFoundException If required element of pdf object is not
 * found.
 * @throw AmbiguousPageTreeException if page tree is ambiguous and node position
 * can't be determined.
 *
 * @return Position of the node or 0 if node couldn't be found under this
 * superNode.
 */
size_t searchTreeNode(CPdf & pdf, shared_ptr<CDict> superNode, shared_ptr<CDict> node, size_t startValue)
{
	utilsPrintDbg(DBG_DBG, "startPos="<<startValue);
	
	// if nodes are same, startValue is returned.
	if(superNode==node)
	{
		utilsPrintDbg(DBG_DBG, "Page found");
		return startValue;
	}

	// gets type of dictionary
	string superNodeType=getNameFromDict("Type", superNode);
	
	// if type is Page we return with 0, because node is not this superNode and
	// there is nowhere to go
	if(superNodeType=="Page")
		return 0;

	// if node is not also Pages - page tree is not well formated and we skip
	// this node
	if(superNodeType!="Pages")
	{
		utilsPrintDbg(DBG_WARN, "Given dictionary is not correct page tree node. type="<<superNodeType);
		return 0;
	}

	// if type is Pages, we go through Kids array and calls recursively for each
	// element until first one returns with non 0. Otherwise startValue is
	// updated by child node pages count.
	shared_ptr<IProperty> arrayProp_ptr=superNode->getProperty("Kids");
	if(arrayProp_ptr->getType()!=pArray)
	{
		utilsPrintDbg(DBG_ERR, "Supernode Kids field is not an array. type="<<arrayProp_ptr->getType());
		throw ElementBadTypeException("Kids");
	}
	shared_ptr<CArray> array_ptr=IProperty::getSmartCObjectPtr<CArray>(arrayProp_ptr);

	// Checks if node's reference is present in Kids array
	// All nodes has to be indirect objects so getIndiRef returns node's
	// reference
	// If getPropertyId returns more than one element - position is ambiguous and
	// exception is thrown 
	shared_ptr<CRef> nodeRef(CRefFactory::getInstance(node->getIndiRef()));
	vector<CArray::PropertyId> positions;
	getPropertyId<CArray, vector<CArray::PropertyId> >(array_ptr, nodeRef, positions);
	if(positions.size()>1)
	{
		utilsPrintDbg(DBG_WARN, "Position of node is ambiguous.");
		throw AmbiguousPageTreeException();
	}
	
	// goes through all Kids and checks if node is one of them or go deeper in
	// subtree in case of intermediate kid
	vector<shared_ptr<IProperty> > kidsContainer;
	array_ptr->_getAllChildObjects(kidsContainer);
	vector<shared_ptr<IProperty> >::iterator i;
	int index=0;
	for(i=kidsContainer.begin(); i!=kidsContainer.end(); i++, index++)
	{
		// each element has to be reference
		PropertyType type=(*i)->getType();
		if(type!=pRef)
		{
			// we will just print warning and skips this element
			utilsPrintDbg(DBG_WARN, "Kids["<<index<<"] is not an reference. type="<<type);
			continue;
		}

		// dereference indirect object - this has to be dictionary
		IndiRef ref=getValueFromSimple<CRef, pRef, IndiRef>(*i);
		shared_ptr<IProperty> element_ptr=pdf.getIndirectProperty(ref);
		if(element_ptr->getType()!=pDict)
		{
			// we will just print warning and skips this element
			utilsPrintDbg(DBG_WARN, "Kids["<<index<<"] doesn't refer to a dictionary. type="<<element_ptr->getType());
			continue;
		}
		shared_ptr<CDict> elementDict_ptr=IProperty::getSmartCObjectPtr<CDict>(element_ptr);

		// compares element_ptr (kid) with node, if they are same, returns
		// startValue
		if(element_ptr==node)
			return startValue;

		// node and element_ptr are not same, so node can't be this page
		string elementDictType=getNameFromDict("Type", elementDict_ptr);
		if(elementDictType=="Page")
		{
			// this was not correct one startValue is increased
			startValue++;
			continue;
		}

		// Pages dictionary means intermediarte node, so start recursion in this
		// subtree
		if(elementDictType=="Pages")
		{
			size_t ret=searchTreeNode(pdf, elementDict_ptr, node, startValue);
			if(ret)
				return ret;

			// we haven't found, so startValue is actualized with Count value
			// of this node
			startValue+=getIntFromDict("Count", elementDict_ptr);
			continue;
		}
		
		// if node is not Page or Pages it is ignored.
		utilsPrintDbg(DBG_WARN, "Kids element dictionary is neither Page nor Pages node.");
	}

	return 0;
}

size_t getNodePosition(CPdf & pdf, shared_ptr<IProperty> node)
{
	utilsPrintDbg(DBG_DBG, "");
	// node must by from given pdf
	if(node->getPdf()!=&pdf)
	{
		utilsPrintDbg(DBG_ERR, "Node is not from given pdf isntance.");
		throw PageNotFoundException(0);
	}

	// gets page tree root - it has to be Reference to Dictionary
	shared_ptr<IProperty> rootRef_ptr=pdf.getDictionary()->getProperty("Pages");
	shared_ptr<CDict> rootDict_ptr=getDictFromRef(rootRef_ptr);
	
	// gets dictionary from node parameter. It can be reference and
	// dereferencing has to be done or direct dictionary - otherwise error is
	// reported
	PropertyType nodeType=node->getType();
	if(nodeType!=pRef && nodeType!=pDict)
	{
		utilsPrintDbg(DBG_ERR, "Given node is not reference nor dictionary. type="<<nodeType);
		throw ElementBadTypeException("node");
	}
	shared_ptr<CDict> nodeDict_ptr;
	if(nodeType==pRef)
		nodeDict_ptr=getDictFromRef(node);
	else
		nodeDict_ptr=IProperty::getSmartCObjectPtr<CDict>(node);
		
	utilsPrintDbg(DBG_DBG, "Starts searching");
	size_t pos=searchTreeNode(pdf, rootDict_ptr, nodeDict_ptr, 1);
	utilsPrintDbg(DBG_DBG, "Searching finished. pos="<<pos);
	if(pos)
		return pos;

	// node not found
	throw PageNotFoundException(0);
}

bool isDescendant(CPdf & pdf, IndiRef parent, shared_ptr<CDict> child)
{
using namespace utils;

	// tries to get child's direct parent node
	// if not found, we catch the exception and return false
	// this should happen in root node
	IndiRef directParent;
	try
	{
		directParent=getRefFromDict("Parent", child);
	}catch(std::exception & e)
	{
		// child is not really parent child
		return false;
	}

	// we have direct parent reference
	// if it is same as given parent, we are done and return true
	if(parent==directParent)
		return true;

	// TODO solve problem when parent is the root of hierarchy and its Parent
	// field points to itself - this should NOT happen in normal pdf files
	
	// parent may be somewhere higher in the page tree
	// Gets direct parent indirect object from given pdf and checks its type
	// if it is NOT dictionary - we have malformed pdf - exception is thrown
	// otherwise starts recursion with direct parent dictionary
	shared_ptr<IProperty> directParent_ptr=pdf.getIndirectProperty(directParent);
	if(directParent_ptr->getType()!=pDict)
		throw MalformedFormatExeption("Page node parent field doesn't point to dictionary");
	return isDescendant(pdf, parent, IProperty::getSmartCObjectPtr<CDict>(directParent_ptr));
}

/** Registers page tree observer.
 * @param ref Reference to page tree node.
 * @param observer Pointer to observer to register.
 *
 * Registers given observer to given reference. Then gets indirect property from
 * reference and if it is Pages dictionary, Registers observer also on Kids
 * array and recursively to all its elements (which are referencies).
 */
void registerPageTreeObserver(boost::shared_ptr<CRef> ref, shared_ptr<const observer::IObserver<IProperty> > observer)
{
using namespace boost;
using namespace std;
using namespace pdfobjects::utils;

	IndiRef indiRef=getValueFromSimple<CRef, pRef, IndiRef>(ref);
	utilsPrintDbg(DBG_DBG, "ref=["<<indiRef.num<<", "<<indiRef.gen<<"]");

	// registers observer for page tree handling
	ref->registerObserver(observer);
	
	// dereferences and if it is Pages dictionary, calls recursively to all
	// children
	shared_ptr<CDict> dict_ptr=getDictFromRef(ref);
	string dictType=getNameFromDict("Type", dict_ptr);

	// if this is not pages dictionary, immediately returns
	if(dictType!="Pages")
		return;

	utilsPrintDbg(DBG_DBG, "Intermediate node. Registers to Kids array and all its elements");

	// gets Kids field from dictionary and all children from array and 
	// registers observer to array and to each member reference
	shared_ptr<IProperty> kidsProp_ptr=dict_ptr->getProperty("Kids");
	if(kidsProp_ptr->getType()!=pArray)
	{
		// Kids is not an array - this is not an hard error at this context
		utilsPrintDbg(DBG_WARN, "Kids field is not an array. type="<<kidsProp_ptr->getType());
		return;
	}
	shared_ptr<CArray> kids_ptr=IProperty::getSmartCObjectPtr<CArray>(kidsProp_ptr);
	kids_ptr->registerObserver(observer);
	vector<shared_ptr<IProperty> > container;
	kids_ptr->_getAllChildObjects(container);
	for(vector<shared_ptr<IProperty> >::iterator i=container.begin(); i!=container.end(); i++)
	{
		// all reference kids are used for recursion call
		shared_ptr<IProperty> elemProp_ptr=*i;
		if(elemProp_ptr->getType()==pRef)
			registerPageTreeObserver(IProperty::getSmartCObjectPtr<CRef>(elemProp_ptr), observer);
	}

	utilsPrintDbg(DBG_DBG, "All subnodes done for ref=["<<indiRef.num<<", "<<indiRef.gen<<"]");
}

/** Adds all indirect objects from given property.
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
 * After this method returns, all subproperties of given one are known to given
 * pdf instance.
 * <br>
 * Returned reference has to be deallocated by caller.
 * 
 * @return Pointer to new reference, if new indirect property was created
 * for reference (caller must deallocate result).
 */ 
IndiRef * addReferencies(CPdf * pdf, boost::shared_ptr<IProperty> ip)
{
	utilsPrintDbg(debug::DBG_DBG,"");

	PropertyType type=ip->getType();
	ChildrenStorage childrenStorage;

	switch(type)
	{
		case pRef:
		{
			utilsPrintDbg(debug::DBG_DBG,"Property is reference - adding new indirect object");
			// this may fail because of ReadOnlyDocumentException
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
			utilsPrintDbg(debug::DBG_DBG,"Reference changed to [" << ref->num << ", " <<ref->gen << "]");
			delete ref;
			continue;
		}
	}

	// also complex object is same - all referencies in this subtree are added
	// in this moment
	return NULL;
}

} // end of utils namespace

void CPdf::PageTreeWatchDog::notify(boost::shared_ptr<IProperty> newValue, boost::shared_ptr<const observer::IChangeContext<IProperty> > context) const throw()
{
using namespace observer;
using namespace boost;
using namespace debug;
using namespace utils;

	kernelPrintDbg(DBG_DBG, "");
	
	// if not basic context type, does notnig
	if(context->getType()!=IChangeContext<IProperty>::BasicChangeContextType)
		return;

	// gets basic context shared_ptr
	shared_ptr<const BasicChangeContext<IProperty> > basicContext=
		dynamic_pointer_cast<const BasicChangeContext<IProperty>, const IChangeContext<IProperty> >(context); 

	// gets original value
	const shared_ptr<IProperty> oldValue=basicContext->getOriginalValue();

	PropertyType oldType=oldValue->getType(),
				 newType=newValue->getType();

	// one of values can be CNull, but not both. If this happens, there is
	// nothing to do
	if(oldType==pNull && newType==pNull)
	{
		kernelPrintDbg(DBG_WARN, "Both newValue and oldValue are CNull");
		return;
	}
	
	// if both of values are not CRef, there is also nothing to consolidate,
	// because old mess has been replaced by new mess
	if(oldType!=pRef && newType!=pRef)
	{
		kernelPrintDbg(DBG_INFO, "Nothing to consolidate because newValue and oldValue are not CRef");
		return;
	}
		
	// if newValue is CRef instance, registers this observer on it and if it 
	// is intermediate node, also to all children. 
	// Uses registerPageTreeObserver method
	if(newType==pRef)
	{
		shared_ptr<CRef> newValueRef_ptr=IProperty::getSmartCObjectPtr<CRef>(newValue);
		try
		{
			registerPageTreeObserver(newValueRef_ptr, pdf->pageTreeWatchDog);
		}catch(CObjectException & e)
		{
			kernelPrintDbg(DBG_ERR, "registerPageTreeObserver failed with cause="<<e.what());
		}
	}


	// consolidates page tree from newValue's indirect parent. If newValue is
	// CNull, uses oldValue's. This is correct because they both have been in
	// same intermediate node and both are direct values (change is in Kids
	// array). 
	IndiRef parentRef=(newType!=pNull)
		?newValue->getIndiRef()
		:oldValue->getIndiRef();
	shared_ptr<IProperty> parentProp_ptr=pdf->getIndirectProperty(parentRef);
	if(parentProp_ptr->getType()!=pDict)
	{
		// target of the parent reference is not dictionary,
		// this should not happen - some one is doing something nasty
		kernelPrintDbg(DBG_ERR, "newValue's parent is not dictionary. THIS SHOUL NOT HAPPEN");
		return;
	}

	// starts consolidation from parent interemdiate node
	shared_ptr<CDict> parentDict_ptr=IProperty::getSmartCObjectPtr<CDict>(parentProp_ptr);
	try
	{
		// if consolidatePageTree hasn't kept page count numbers, total number
		// of pages must be invalidated
		if(!pdf->consolidatePageTree(parentDict_ptr))
			pdf->pageCount=0;
	}catch(CObjectException & e)
	{
		kernelPrintDbg(DBG_ERR, "consolidatePageTree failed with cause="<<e.what());
	}

	// consolidates pageList 
	try
	{
		pdf->consolidatePageList(oldValue, newValue);
	}catch(CObjectException &e)
	{
		kernelPrintDbg(DBG_ERR, "consolidatePageList failed with cause="<<e.what());
	}
	
	kernelPrintDbg(DBG_DBG, "observer handler finished");
}

void CPdf::initRevisionSpecific()
{
using namespace utils;

	kernelPrintDbg(DBG_DBG, "");

	// Clean up part:
	// =============
	
	// cleans up indirect mapping
	if(indMap.size())
	{
		kernelPrintDbg(DBG_INFO, "Cleaning up indirect mapping with "<<indMap.size()<<" elements");
		indMap.clear();
	}

	// cleans up and invalidates all returned pages
	if(pageList.size())
	{
		kernelPrintDbg(DBG_INFO, "Cleaning up pages list with "<<pageList.size()<<" elements");
		PageList::iterator i;
		for(i=pageList.begin(); i!=pageList.end(); i++)
		{
			kernelPrintDbg(DBG_DBG, "invalidating page at pos="<<i->first);
			// FIXME uncoment when method is reay
			//i->second->invalidate();
		}
		pageList.clear();
	}

	// invalidates pageCount
	pageCount=0;
	
	// cleanup all returned outlines  -------------||----------------- 
	
	// Initialization part:
	// ===================
	
	// initialize trailer dictionary from xpdf trailer dictionary object
	// no free should be called because trailer is returned directly from XRef
	Object * trailerObj=xref->getTrailerDict();
	kernelPrintDbg(DBG_DBG, "Creating trailer dictionary from type="<<trailerObj->getType());
	trailer=boost::shared_ptr<CDict>(CDictFactory::getInstance(*trailerObj));
	
	// Intializes document catalog dictionary.
	// gets Root field from trailer, which should contain reference to catalog.
	// If no present or not reference, we have corrupted PDF file and exception
	// is thrown
	kernelPrintDbg(DBG_DBG, "Getting Root of page tree");
	IndiRef rootRef=utils::getRefFromDict("Root", trailer);
	shared_ptr<IProperty> prop_ptr=getIndirectProperty(rootRef);
	if(prop_ptr->getType()!=pDict)
	{
		kernelPrintDbg(DBG_CRIT, "Trailer dictionary doesn't point to correct document catalog.");
		throw ElementBadTypeException("Root");
	}
	kernelPrintDbg(DBG_INFO, "Document catalog successfully fetched");
	docCatalog=IProperty::getSmartCObjectPtr<CDict>(prop_ptr);
	
	// calls registerPageTreeObserver to Document catalog Pages field which
	// means that observer is registered to whole page tree.
	// If Document catalog doesn't contain any Pages field, we don't have any
	// pages and so nothing is registered
	try
	{
		shared_ptr<IProperty> pageTreeProp_ptr=docCatalog->getProperty("Pages");
		if(pageTreeProp_ptr->getType()!=pRef)
		{
			kernelPrintDbg(DBG_ERR, "Pages field is not reference.");	
		}else
		{
			shared_ptr<CRef> pageTreeRoot_ptr=IProperty::getSmartCObjectPtr<CRef>(pageTreeProp_ptr);
			registerPageTreeObserver(pageTreeRoot_ptr, pageTreeWatchDog);
		}
	}catch(CObjectException & e)
	{
		kernelPrintDbg(DBG_WARN, "Document doesn't contain Page tree.");
	}
}

CPdf::CPdf(StreamWriter * stream, OpenMode openMode):pageTreeWatchDog(new PageTreeWatchDog(this))
{
	// gets xref writer - if error occures, exception is thrown 
	xref=new XRefWriter(stream, this);
	mode=openMode;

	// initializes revision specific data for the newest revision
	initRevisionSpecific();

	// sets mode accoring openMode
	// ReadOnly and ReadWrite implies xref paranoid mode (default one) 
	// whereas Advanced mode sets easy mode because we want to have full 
	// control over document
	if(mode==Advanced)
		xref->setMode(XRefWriter::easy);
}

CPdf::~CPdf()
{
	kernelPrintDbg(DBG_DBG, "");

	// indirect mapping is cleaned up automaticaly
	
	// discards all returned pages
	for(PageList::iterator i=pageList.begin(); i!=pageList.end(); i++)
	{
		kernelPrintDbg(DBG_DBG, "Invalidating page at pos="<<i->first);
		// FIXME uncoment when method is ready
		//i->second->invalidate();
	}

	// TODO handle outlines when ready
	
	
	// deallocates XRefWriter
	delete xref;
}


//
// 
// this method can't be const because createObjFromXpdfObj requires 
// CPdf * not const CPdf * given by this
boost::shared_ptr<IProperty> CPdf::getIndirectProperty(IndiRef ref)
{
using namespace debug;

	kernelPrintDbg (DBG_DBG,"ref=["<<ref.num << "," << ref.gen <<"]");

	// find the key, if it exists
	IndirectMapping::iterator i = indMap.find(ref);
	if(i!=indMap.end())
	{
		// mapping exists, so returns value
		kernelPrintDbg(DBG_DBG, "mapping exists");
		return i->second;
	}

	kernelPrintDbg(DBG_DBG, "mapping doesn't exist")

	// mapping doesn't exist yet, so tries to create one
	// fetches object according reference
	Object obj;
	xref->fetch(ref.num, ref.gen, &obj);
	
	boost::shared_ptr<IProperty> prop_ptr;

	// creates cobject from value according type - indirect
	// parent is set to object reference (it is its own indirect parent)
	// created object is wrapped to smart pointer and if not pNull also added to
	// the mapping
	if(obj.getType()!=objNull)
	{
		IProperty * prop=utils::createObjFromXpdfObj(*this, obj, ref);
		prop_ptr=shared_ptr<IProperty>(prop);
		indMap.insert(IndirectMapping::value_type(ref, prop_ptr));
		kernelPrintDbg(DBG_INFO, "Mapping created for ref=["<<ref.num<<", "<<ref.gen<<"]");
	}else
	{
		kernelPrintDbg(DBG_INFO, "ref["<<ref.num<<", "<<ref.gen<<"] not available or points to objNull");
		prop_ptr=shared_ptr<CNull>(CNullFactory::getInstance());
	}

	obj.free ();
	return prop_ptr;
}


//
//
//
IndiRef CPdf::addIndirectProperty(boost::shared_ptr<IProperty> ip)
{
using namespace utils;
using namespace debug;
using namespace boost;

	kernelPrintDbg(DBG_DBG, "");

	if(getMode()==ReadOnly)
	{
		kernelPrintDbg(DBG_ERR, "Document is in read-only mode now");
		throw ReadOnlyDocumentException("Document is in read-only mode.");
	}
	
	// place for propertyValue
	// it is ip by default
	shared_ptr<IProperty> propValue=ip;
	
	// if given ip is reference, we have to distinguish whether
	// it comes from same file (and then nothing is to do and we
	// just simply return ip reference) 
	// or from different file, when we need to dereference and after 
	// something else can be done
	PropertyType type=ip->getType();
	if(type==pRef)
	{
		IndiRef ref=getValueFromSimple<CRef, pRef, IndiRef>(ip);
		
		// just returns reference
		if(ip->getPdf()==this)
		{
			kernelPrintDbg(DBG_WARN, "Reference is from this file, nothing is added.");
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
		kernelPrintDbg(DBG_DBG, "Adding property from different file.");
		shared_ptr<IProperty> clone;
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
	// reserveRef may throw if we are in older revision
	::Object * obj=propValue->_makeXpdfObject();
	::Ref xpdfRef=xref->reserveRef();
	kernelPrintDbg(DBG_DBG, "New reference reseved ["<<xpdfRef.num<<", "<<xpdfRef.gen<<"]");
	xref->changeObject(xpdfRef.num, xpdfRef.gen, obj);

	// xpdf object has to be deallocated
	freeXpdfObject(obj);

	// creates return value from xpdf reference structure
	// and returns
	IndiRef reference={xpdfRef.num, xpdfRef.gen};
	kernelPrintDbg(DBG_INFO, "New indirect object inserted with reference ["<<xpdfRef.num<<", "<<xpdfRef.gen<<"]");
	change=true;
	return reference;
}

void CPdf::changeIndirectProperty(boost::shared_ptr<IProperty> prop)
{
	kernelPrintDbg(DBG_DBG, "");
	
	if(getMode()==ReadOnly)
	{
		kernelPrintDbg(DBG_ERR, "Document is in read-only mode now");
		throw ReadOnlyDocumentException("Document is in read-only mode.");
	}
	
	// checks property at first
	// it must be from same pdf
	if(prop->getPdf() != this)
	{
		kernelPrintDbg(DBG_ERR, "Given property is not from same pdf.");
		throw CObjInvalidObject();
	}
	// there must be mapping fro prop's indiref, but it doesn't have to be same
	// instance.
	IndiRef indiRef=prop->getIndiRef();
	if(indMap.find(indiRef)==indMap.end())
	{
		kernelPrintDbg(DBG_ERR, "Indirect mapping doesn't exist. prop seams to be fake.");
		throw CObjInvalidObject();
	}

	// gets xpdf Object instance and calls xref->change
	// changeObject may throw if we are in read only mode or if xrefwriter is
	// in paranoid mode and type check fails
	Object * propObject=prop->_makeXpdfObject();
	kernelPrintDbg(DBG_DBG, "Registering change to the XRefWriter");
	xref->changeObject(indiRef.num, indiRef.gen, propObject);
	utils::freeXpdfObject(propObject);

	// checks whether prop is same instance as one in mapping. If so, keeps
	// indirect mapping, because it has just changed some of its direct fields. 
	// Otherwise removes it, because new value is something totaly different. 
	// Mapping will be created in next getIndirectProperty call.
	if(prop==getIndirectProperty(indiRef))
	{
		kernelPrintDbg(DBG_INFO,  "Indirect mapping kept for ref=["<<indiRef.num<<", "<<indiRef.gen<<"]");
	}
	else
	{
		indMap.erase(indiRef);
		kernelPrintDbg(DBG_INFO, "Indirect mapping removed for ref=["<<indiRef.num<<", "<<indiRef.gen<<"]");
	}

	// sets change flag
	change=true;
}

CPdf * CPdf::getInstance(const char * filename, OpenMode mode)
{
using namespace std;

	kernelPrintDbg(debug::DBG_DBG, "");
	
	// openMode is read-only by default
	const char * openMode="r";
	
	// if mode is ReadWrite or higher, set to read-write mode starting at the 
	// begining.
	if(mode >= ReadWrite)
		openMode="r+";

	// opens file and creates (xpdf) FileStream
	FILE * file=fopen(filename, openMode);
	if(!file)
	{
		kernelPrintDbg(debug::DBG_ERR, "Unable to open file (reason="<<strerror(errno)<<")");
		throw PdfOpenException("Unable to open file.");
	}
	kernelPrintDbg(debug::DBG_DBG,"File \"" << filename << "\" open successfully in mode=" << openMode);
	
	// creates FileStream writer to enable changes to the File stream
	Object obj;
	obj.initNull();
	StreamWriter * stream=new FileStreamWriter(file, 0, gFalse, 0, &obj);
	kernelPrintDbg(debug::DBG_DBG,"File stream created");

	// stream is ready, creates CPdf instance
	try
	{
		CPdf * instance=new CPdf(stream, mode);
		kernelPrintDbg(debug::DBG_INFO, "Instance created successfully openMode=" << openMode);
		return instance;
	}catch(exception &e)
	{
		kernelPrintDbg(DBG_CRIT, "Pdf instance creation failed. cause="<<e.what());
		throw PdfOpenException("CPdf failed. reason=");
	}
}

int CPdf::close(bool saveFlag)
{
	kernelPrintDbg(debug::DBG_DBG, "");
	// saves if necessary
	if(saveFlag)
		save();
	
	// deletes this instance
	// all clean-up is made in destructor
	delete this;

	kernelPrintDbg(debug::DBG_INFO, "Instance deleted.")
	return 0;
}

boost::shared_ptr<CPage> CPdf::getPage(size_t pos)const
{
using namespace utils;

	kernelPrintDbg(DBG_DBG, "");

	if(pos < 1 || pos>getPageCount())
	{
		kernelPrintDbg(DBG_ERR, "Page out of range pos="<<pos);
		throw PageNotFoundException(pos);
	}

	// checks if page is available in pageList
	PageList::const_iterator i;
	if((i=pageList.find(pos))!=pageList.end())
	{
		kernelPrintDbg(DBG_INFO, "Page at pos="<<pos<<" found in pageList");
		return i->second;
	}

	// page is not available in pageList, searching has to be done
	// gets Pages field from document catalog (no special checking has to be
	// done, because everything is done in getPageCount method).
	shared_ptr<IProperty> rootPages_ptr=docCatalog->getProperty("Pages");
	// find throws an exception if any problem found, otherwise pageDict_ptr
	// contians Page dictionary at specified position.
	shared_ptr<CDict> pageDict_ptr=findPageDict(*this, rootPages_ptr, 1, pos);

	// creates CPage instance from page dictionary and stores it to the pageList
	CPage * page=CPageFactory::getInstance(pageDict_ptr);
	shared_ptr<CPage> page_ptr(page);
	pageList.insert(PageList::value_type(pos, page_ptr));
	kernelPrintDbg(DBG_DBG, "New page added to the pageList size="<<pageList.size())

	return page_ptr;
}

unsigned int CPdf::getPageCount()const
{
using namespace utils;
	
	kernelPrintDbg(DBG_DBG, "");
	
	// try to use cached value - if zero, we have to get it from Page tree root
	if(pageCount)
	{
		kernelPrintDbg(DBG_DBG, "Uses cached value");
		kernelPrintDbg(DBG_INFO, "Page Count="<<pageCount);
		return pageCount;
	}
	
	// gets Pages field from document catalog
	shared_ptr<IProperty> pages_ptr=docCatalog->getProperty("Pages");
	
	// gets type of the dictionary and find out if it is Page node or
	// intermediate node
	shared_ptr<CDict> dict_ptr=getDictFromRef(pages_ptr);
	string dictType=getNameFromDict("Type", dict_ptr);
	if(dictType=="Page")
	{
		// this document contains only one page
		// This is not standard situation
		kernelPrintDbg(DBG_INFO, "Page count=1 no intermediate node.");
		return pageCount=1;
	}
	if(dictType=="Pages")
	{
		// gets count field
		int count=getIntFromDict("Count", dict_ptr);
		kernelPrintDbg(DBG_INFO, "Page count="<<count);
		return pageCount=count;
	}

	// unable to ge page count 
	pageCount=0;
	kernelPrintDbg(DBG_CRIT, "Pages dictionary has bad dictionary type type="<<dictType);
	throw MalformedFormatExeption("Pages dictionary is not Page or Pages dictionary");
}

boost::shared_ptr<CPage> CPdf::getNextPage(boost::shared_ptr<CPage> page)const
{
	kernelPrintDbg(DBG_DBG, "");

	size_t pos=getPagePosition(page);
	kernelPrintDbg(DBG_DBG, "Page position is "<<pos);
	pos++;
	
	// checks if we are in boundary after incrementation
	if(pos==0 || pos>getPageCount())
	{
		kernelPrintDbg(DBG_ERR, "Page is out of range pos="<<pos);
		throw PageNotFoundException(pos);
	}

	// page in range, uses getPage
	return getPage(pos);

}

boost::shared_ptr<CPage> CPdf::getPrevPage(boost::shared_ptr<CPage> page)const
{
	kernelPrintDbg(DBG_DBG, "");

	size_t pos=getPagePosition(page);
	kernelPrintDbg(DBG_DBG, "Page position is "<<pos);
	pos--;
	
	// checks if we are in boundary after incrementation
	if(pos==0 || pos>getPageCount())
	{
		kernelPrintDbg(DBG_ERR, "Page is out of range pos="<<pos);
		throw PageNotFoundException(pos);
	}

	// page in range, uses getPage
	return getPage(pos);
}

size_t CPdf::getPagePosition(boost::shared_ptr<CPage> page)const
{
	kernelPrintDbg(DBG_DBG, "");
		
	// search in returned page list
	PageList::iterator i;
	for(i=pageList.begin(); i!=pageList.end(); i++)
	{
		// compares page instances
		// This is ok even if they manage same page dictionary
		if(i->second == page)
		{
			kernelPrintDbg(DBG_INFO, "Page found at pos="<<i->first);
			return i->first;
		}
	}

	// page not found, it hasn't been returned by this pdf
	throw PageNotFoundException();
}


void CPdf::consolidatePageList(shared_ptr<IProperty> oldValue, shared_ptr<IProperty> newValue)
{
using namespace utils;

	kernelPrintDbg(DBG_DBG, "");

	// correction for all pages affected by this subtree change
	int difference=0;

	// position of first page which should be considered during consolidation 
	// because of value change
	size_t minPos=0;

	// handles original value - one before change
	// pNull means no previous value available (new sub tree has been added)
	kernelPrintDbg(DBG_DBG, "oldValue type="<<oldValue->getType());
	if(oldValue->getType()!=pNull)
	{
		// FIXME remove
		IndiRef oldValueRef=getValueFromSimple<CRef, pRef, IndiRef>(oldValue);
		
		// gets dictionary type - it has to be page or pages node
		shared_ptr<CDict> oldDict_ptr=getDictFromRef(oldValue);
		string oldDictType=getNameFromDict("Type", oldDict_ptr);

		// simple page is compared with all from pageList and if found, removes
		// it from list and invalidates it.
		// Difference is set to - 1, because one page is removed 
		if(oldDictType=="Page")
		{
			kernelPrintDbg(DBG_DBG, "oldValue was simple page dictionary");
			difference = -1;

			PageList::iterator i;
			for(i=pageList.begin(); i!=pageList.end(); i++)
			{
				// checks page's dictionary with old one
				if(i->second->getDictionary() == oldDict_ptr)
				{
					// FIXME: uncoment when method is ready
					//i->second->invalidate();
					size_t pos=i->first;
					minPos=pos;
					pageList.erase(i);
					kernelPrintDbg(DBG_INFO, "CPage(pos="<<pos<<") associated with oldValue page dictionary removed. pageList.size="<<pageList.size());
					break;
				}
			}
		}else
		{
			// Pages dictionary stands for intermediate node and so all CPages
			// from this sub tree are removed and invalidated
			// difference is set to -Count value (those number of pages are
			// removed)
			if(oldDictType=="Pages")
			{
				kernelPrintDbg(DBG_DBG, "oldValue was intermediate node dictionary.")
				difference = -getIntFromDict("Count", oldDict_ptr);

				// gets reference of oldValue - which is the root removed
				// subtree
				IndiRef ref=getValueFromSimple<CRef, pRef, IndiRef>(oldValue);
				
				PageList::iterator i;
				for(i=pageList.begin(); i!=pageList.end(); i++)
				{
					// checks page's dictionary whether it is in oldDict_ptr sub
					// tree and if so removes it from pageList
					if(isDescendant(*this, ref, i->second->getDictionary()))
					{
						// updates minPos with page position (if greater)
						size_t pos=i->first;
						if(pos > minPos)
							minPos=pos;
						
						// FIXME: uncoment when method is ready
						//i->second->invalidate();
						pageList.erase(i);
						kernelPrintDbg(DBG_INFO, "CPage(pos="<<pos<<") associated with oldValue page dictionary removed. pageList.size="<<pageList.size());

					}
				}
			}
		}
	}

	// oldValue subtree (if any) is consolidated now
	kernelPrintDbg(DBG_DBG, "All page dictionaries from oldValue subtree removed");

	// number of added pages by newValue tree
	int pagesCount=0;
	
	// handles new value - one after change
	// if pNull - no new value is available (subtree has been removed)
	kernelPrintDbg(DBG_DBG, "newValue type="<<newValue->getType());
	if(newValue->getType()!=pNull)
	{
		// gets dictionary type
		shared_ptr<CDict> newDict_ptr=getDictFromRef(newValue);
		string newDictType=getNameFromDict("Type", newDict_ptr);

		// page type adds only one page
		if(newDictType=="Page")
			pagesCount=1;

		// pages type adds Count pages
		if(newDictType=="Pages")
			pagesCount=getIntFromDict("Count", newDict_ptr);

		// try to get position of newValue node.  No pages from this sbtree can
		// be in the pageList, so we can set minPos to its position.
		// If getNodePosition throws, then this node is ambiguous and so we have
		// no information
		try
		{
			minPos = getNodePosition(*this, newValue);
		}catch(exception &e)
		{
			// position can't be determined
			// no special handling is needed, minPos keeps its value
		}

		kernelPrintDbg(DBG_DBG, "newValue sub tree has "<<pagesCount<<" page dictionaries");
	}

	// corrects difference with added pages
	difference += pagesCount;

	// no difference means no speacial handling for other pages
	// we have replaced old sub tree with new subtree with same number of pages
	if(difference==0)
		return;

	kernelPrintDbg(DBG_INFO, "pageList consolidation from minPos="<<minPos);
	
	// all pages with position greater than minPos, has to be consolidated
	PageList::iterator i;
	PageList readdContainer;
	for(i=pageList.begin(); i!=pageList.end(); i++)
	{
		size_t pos=i->first;
		shared_ptr<CPage> page=i->second;

		// all pages >= minPos are removed and readded with correct position
		if(pos >= minPos)
		{
			// collects all removed
			readdContainer.insert(PageList::value_type(pos, page));	
			pageList.erase(i);
		}
	}
	
	// checks minPos==0 and if so, we have to handle situation special way,
	// because don't have any information about previous position of oldValue
	// subtree. In such case all pages which should be readded are searched in
	// page tree and actual position is used.
	if(!minPos)
	{
		kernelPrintDbg(DBG_DBG,"Reassigning all pages positition.");
		for(i=readdContainer.begin(); i!=readdContainer.end(); i++)
		{
			// uses getNodePosition for each page's dictionary to find out
			// current position. If getNodePosition throws an exception, it
			// means that it can't be determined. Such page is invalidated.
			try
			{
				size_t pos=getNodePosition(*this, i->second->getDictionary());
				kernelPrintDbg(DBG_DBG, "Original position="<<i->first<<" new="<<pos);
				pageList.insert(PageList::value_type(pos, i->second));	
			}catch(exception & e)
			{
				kernelPrintDbg(DBG_WARN, "page with original position="<<i->first<<" is ambiguous. Invalidating.");
				// page position is ambiguous and so it has to be invalidate
				//TODO uncoment when method is ready
				//i->second->invalidate();
			}
		}
		return;
	}
	
	kernelPrintDbg(DBG_DBG, "Moving pages position with difference="<<difference<<" from page pos="<<minPos);
	// Information about page numbers which should be consolidated is available
	// so just adds difference for each in readdContainer
	// readds all removed with changed position (according difference)
	for(i=readdContainer.begin(); i!=readdContainer.end(); i++)
	{
		kernelPrintDbg(DBG_DBG, "Original position="<<i->first<<" new="<<i->first+difference);
		pageList.insert(PageList::value_type(i->first+difference, i->second));	
	}
	kernelPrintDbg(DBG_INFO, "pageList consolidation done.")
}


bool CPdf::consolidatePageTree(boost::shared_ptr<CDict> interNode)
{
using namespace utils;

	IndiRef interNodeRef=interNode->getIndiRef();
	kernelPrintDbg(DBG_DBG, "interNode ref=["<<interNodeRef.num<<", "<<interNodeRef.gen<<"]");

	// gets pdf of the node
	CPdf * pdf=interNode->getPdf();
	if(!pdf)
	{
		kernelPrintDbg(DBG_ERR, "internode has no pdf");
		return true;
	}
	
	// checks if interNode is real Pages dictionary
	string dictType=getNameFromDict("Type", interNode);
	if(dictType!="Pages")
	{
		kernelPrintDbg(DBG_ERR, "interNode is not correct intermediate node. type="<<dictType);
		throw ElementBadTypeException("InterNode");
	}

	shared_ptr<IProperty> kidsArrayProp_ptr=interNode->getProperty("Kids");
	if(kidsArrayProp_ptr->getType()!=pArray)
	{
		kernelPrintDbg(DBG_CRIT, "interNode's Kids field is not an array. type="<<kidsArrayProp_ptr->getType());
		// Kids field value must be aray
		throw ElementBadTypeException("Kids");
	}

	// following code should not throw an exception

	// gets all children from array
	shared_ptr<CArray> kidsArray_ptr=IProperty::getSmartCObjectPtr<CArray>(kidsArrayProp_ptr);
	vector<shared_ptr<IProperty> > kidsContainer;
	kidsArray_ptr->_getAllChildObjects(kidsContainer);

	// iterate through all array elements, dereference all of them and checks
	// dictionary type. 
	// Collects leaf page nodes from them (for Pages dict use Count value and
	// for Page it is obvious)
	size_t count=0;
	vector<shared_ptr<IProperty> >::iterator i;
	size_t index=0;
	for(i=kidsContainer.begin(); i!=kidsContainer.end(); i++, index++)
	{
		// each element should be reference
		if((*i)->getType()!=pRef)
		{
			// element is not reference, so we print warning and skip it
			// We are in Observer context so CAN'T remove element
			kernelPrintDbg(DBG_WARN, "Kids["<<index<<"] element must be reference. type="<<(*i)->getType());
			continue;
		}

		// we have reference so get indirect object. Indirect should be
		// dictionary. If it is not, we will print warning and skip this
		// element. We are in Observer context so CAN'T remove element
		IndiRef elementRef=getValueFromSimple<CRef, pRef, IndiRef>(*i);
		shared_ptr<IProperty> kidProp_ptr=pdf->getIndirectProperty(elementRef);
		if(kidProp_ptr->getType()!=pDict)
		{
			kernelPrintDbg(DBG_WARN, "Target of Kids["<<index<<"] is not dictionary. type="<<kidProp_ptr->getType());
			continue;
		}

		shared_ptr<CDict> kidDict_ptr=IProperty::getSmartCObjectPtr<CDict>(kidProp_ptr);
			
		// indirect dictionary should be Page or Pages
		// page dictionary increments count by 1 and if Pages, adds Count
		// if some problem occures, skips this element
		try
		{
			string dictType=getNameFromDict("Type", kidDict_ptr);
			if(dictType=="Page")
				count++;
			else
				if(dictType=="Pages")
					count+=getIntFromDict("Count", kidDict_ptr);
				else
				{
					kernelPrintDbg(DBG_WARN, " interNode contains reference to dictionary with bad type="<<dictType);
					// bad dictionaty type is skipped
					continue;
				}
		}catch(CObjectException & e)
		{
			kernelPrintDbg(DBG_WARN, "Problem with dictionary. cause="<<e.what());
			continue;
		}

		// We have page or pages node at this moment. Checks Parent field
		// whether it is reference and points to interNode. All errors are
		// corrected.
		try
		{
			IndiRef parentRef;
			parentRef=getRefFromDict("Parent", kidDict_ptr);
			if(! (parentRef==interNode->getIndiRef()))
			{
				kernelPrintDbg(DBG_WARN, "Kids["<<index<<"] element dictionary doesn't have Parent with proper reference. Correcting to ref=["<<parentRef.num<<", "<<parentRef.gen<<"]");
				CRef cref(interNode->getIndiRef());
				kidDict_ptr->setProperty("Parent", cref);
			}
			
		}catch(ElementNotFoundException & e)
		{
			// Parent not found at all
			// field is added
			IndiRef parentRef=interNode->getIndiRef();
			kernelPrintDbg(DBG_WARN, "No Parent field found. Correcting to ref=["<<parentRef.num<<", "<<parentRef.gen<<"]");
			CRef cref(parentRef);
			kidDict_ptr->addProperty("Parent", cref);
		}catch(ElementBadTypeException & e)
		{
			// Parent is found but with bad type
			// Parent field is removed in first step and than added with correct
			// this is because of different type of field value which may lead
			// to exception if types are checked (in paranoid mode of
			// XRefWriter)
			IndiRef parentRef=interNode->getIndiRef();
			kernelPrintDbg(DBG_WARN, "Parent field found but with bad type. Correcting to ref=["<<parentRef.num<<", "<<parentRef.gen<<"]");
			kidDict_ptr->delProperty("Parent");
			CRef cref(parentRef);
			kidDict_ptr->addProperty("Parent", cref);
		}

	}

	// count is collected for this node, checks if it matches actual value and 
	// if not sets new value and distribute it to all parents. Parent can throw
	// but this node and its sub tree is ok now
	try
	{
		size_t currentCount=getIntFromDict("Count", interNode);
		if(currentCount == count)
		{
			// count field is ok - no parent consolidation has to be done
			kernelPrintDbg(DBG_DBG, "No need to for further consolidation Count field is ok");
			return true;
		}
	}catch(CObjectException &e)
	{
		// not found or bad type
		// no special handling - just continues as if no bad Count value was
		// present - This should never happen
		kernelPrintDbg(DBG_WARN, "Intermediate node without or with bad typed Count field.");
	}

	// Count field is not correct, sets new value and invalidates pagesCount
	kernelPrintDbg(DBG_WARN, "Count field is not correct. Reconstructing correct value.");
	CInt countProp(count);
	kernelPrintDbg(DBG_INFO, "new interNode Count="<<count);
	interNode->setProperty("Count", countProp);
	
	// distribute value also to fathers - if any
	try
	{
		// TODO be carefull top level parent may be parent of himself
		shared_ptr<IProperty> parentRef_ptr=interNode->getProperty("Parent");
		shared_ptr<CDict> parentDict_ptr=getDictFromRef(parentRef_ptr);

		// calls recursively
		kernelPrintDbg(DBG_DBG, "Distributing count value to parent");
		consolidatePageTree(parentDict_ptr);
	}
	catch(exception &e)
	{
		// no parent in intermediate node means root of page tree
		// also consolidatePageTree can throw, but we don't handle it from here.
		// It may be serious problem with demaged tree and so reconstruction has
		// to be done by hand
	}

	// consolidation had to be done
	kernelPrintDbg(DBG_INFO, "pageTree consolidation done for inter node ref=["<<interNodeRef.num<<", "<<interNodeRef.gen<<"]");
	return false;
}

boost::shared_ptr<CPage> CPdf::insertPage(boost::shared_ptr<CPage> page, size_t pos)
{
using namespace utils;

	kernelPrintDbg(DBG_DBG, "pos="<<pos);

	if(getMode()==ReadOnly)
	{
		kernelPrintDbg(DBG_ERR, "Document is in read-only mode now");
		throw ReadOnlyDocumentException("Document is in read-only mode.");
	}
		
	// zero position is corrected to 1
	if(pos==0)
		pos=1;

	// gets intermediate node which includes node at given position. To enable
	// also to insert after last page, following work around is done:
	// if page is greater than page count, append flag is set to true and so new
	// dictionary is stored after last position (instead of storePostion).
	size_t count=getPageCount();
	size_t storePostion=pos;
	bool append=false;
	if(pos>count)
	{
		// sets that we are appending and sets storePostion to last position
		append=true;
		storePostion=count;
		kernelPrintDbg(DBG_INFO, "inserting after (new last page) position="<<storePostion);
	}

	// gets intermediate node where to insert new page
	// in degenerated case, when there are no pages in the tree, we have to
	// handle it special way
	shared_ptr<CDict> interNode_ptr;
	shared_ptr<CRef> currRef;
	if(!count)
	{
		// no pages in the tree, new page should be added to the Root of page
		// tree internode
		shared_ptr<IProperty> rootProp_ptr=docCatalog->getProperty("Pages");
		interNode_ptr=getDictFromRef(IProperty::getSmartCObjectPtr<CRef>(rootProp_ptr));
	}else
	{
		// stores new page at position of existing page
		
		// searches for page at storePosition and gets its reference
		// page dictionary has to be an indirect object, so getIndiRef returns
		// dictionary reference
		shared_ptr<CDict> currentPage_ptr=findPageDict(*this, docCatalog->getProperty("Pages"), 1, storePostion);
		currRef=shared_ptr<CRef>(CRefFactory::getInstance(currentPage_ptr->getIndiRef()));
		
		// gets parent of found dictionary which maintains 
		shared_ptr<IProperty> parentRef_ptr=currentPage_ptr->getProperty("Parent");
		interNode_ptr=getDictFromRef(parentRef_ptr);
	}

	// gets Kids array where to insert new page dictionary
	shared_ptr<IProperty> kidsProp_ptr=interNode_ptr->getProperty("Kids");
	if(kidsProp_ptr->getType()!=pArray)
	{
		kernelPrintDbg(DBG_CRIT, "Pages Kids field is not an array type="<<kidsProp_ptr->getType());
		// Kids is not array - malformed intermediate node
		throw MalformedFormatExeption("Intermediate node Kids field is not an array.");
	}
	shared_ptr<CArray> kids_ptr=IProperty::getSmartCObjectPtr<CArray>(kidsProp_ptr);
	
	// gets index in Kids array where to store.
	// by default insert at 1st position (index is 0)
	size_t kidsIndex=0;
	if(count)
	{
		// gets index of searched node's reference in Kids array - if position 
		// can't be determined unambiguously (getPropertyId returns more positions), 
		// throws exception
		vector<CArray::PropertyId> positions;
		getPropertyId<CArray, vector<CArray::PropertyId> >(kids_ptr, currRef, positions);
		if(positions.size()>1)
		{
			kernelPrintDbg(DBG_ERR, "Page can't be created, because page tree is ambiguous for node at pos="<<storePostion);
			throw AmbiguousPageTreeException();
		}
		kidsIndex=positions[0]+append;
	}

	// Now it is safe to add indirect object, because there is nothing that can
	// fail
	
	// gets page's dictionary and adds it as new indirect property.
	// All page dictionaries must be indirect objects and addIndirectProperty
	// method also solves problems with deep copy and page from different file
	// transition
	IndiRef pageRef=addIndirectProperty(page->getDictionary());

	// adds newly created page dictionary to the kids array at kidsIndex
	// position. This triggers pageTreeWatchDog for consolidation and observer
	// is registered also on newly added reference
	CRef pageCRef(pageRef);
	kids_ptr->addProperty(kidsIndex, pageCRef);
	
	// page dictionary is stored in the tree, consolidation is also done at this
	// moment
	// CPage can be created and inserted to the pageList
	shared_ptr<CDict> newPageDict_ptr=IProperty::getSmartCObjectPtr<CDict>(getIndirectProperty(pageRef));
	shared_ptr<CPage> newPage_ptr(CPageFactory::getInstance(newPageDict_ptr));
	pageList.insert(PageList::value_type(storePostion, newPage_ptr));
	kernelPrintDbg(DBG_DBG, "New page added to the pageList size="<<pageList.size())
	return newPage_ptr;
}

void CPdf::removePage(size_t pos)
{
using namespace utils;

	kernelPrintDbg(DBG_DBG, "");

	if(getMode()==ReadOnly)
	{
		kernelPrintDbg(DBG_ERR, "Document is in read-only mode now");
		throw ReadOnlyDocumentException("Document is in read-only mode.");
	}

	// TODO question: Is it possible to have document with no pages?

	// checks position
	if(1>pos || pos>getPageCount())
		throw PageNotFoundException(pos);

	// Searches for page dictionary at given pos and gets its reference.
	shared_ptr<CDict> currentPage_ptr=findPageDict(*this, docCatalog->getProperty("Pages"), 1, pos);
	shared_ptr<CRef> currRef(CRefFactory::getInstance(currentPage_ptr->getIndiRef()));
	
	// Gets parent field from found page dictionary and gets its Kids array
	// TODO this may be problem - because there may be only one page in
	// document which could have no parent - same as in insertPage
	shared_ptr<IProperty> parentRef_ptr=currentPage_ptr->getProperty("Parent");
	shared_ptr<CDict> interNode_ptr=getDictFromRef(parentRef_ptr);
	shared_ptr<IProperty> kidsProp_ptr=interNode_ptr->getProperty("Kids");
	if(kidsProp_ptr->getType()!=pArray)
	{
		kernelPrintDbg(DBG_CRIT, "Pages Kids field is not an array type="<<kidsProp_ptr->getType());
		// Kids is not array - malformed intermediate node
		throw MalformedFormatExeption("Intermediate node Kids field is not an array.");
	}
	shared_ptr<CArray> kids_ptr=IProperty::getSmartCObjectPtr<CArray>(kidsProp_ptr);

	// gets index of searched node in Kids array and removes element from found
	// position - if position can't be determined unambiguously (getPropertyId
	// returns more positions), exception is thrown
	IndiRef tmpRef=currentPage_ptr->getIndiRef();
	vector<CArray::PropertyId> positions;
	getPropertyId<CArray, vector<CArray::PropertyId> >(kids_ptr, currRef, positions);
	if(positions.size()>1)
	{
		kernelPrintDbg(DBG_ERR, "Page can't be created, because page tree is ambiguous for node at pos="<<pos);
		throw AmbiguousPageTreeException();
	}
	
	// removing triggers pageTreeWatchDog consolidation
	size_t kidsIndex=positions[0];
	kids_ptr->delProperty(kidsIndex);
	
	// page dictionary is removed from the tree, consolidation is done also for
	// pageList at this moment
}

void CPdf::save(bool newRevision)
{
	kernelPrintDbg(DBG_DBG, "");

	if(getMode()==ReadOnly)
	{
		kernelPrintDbg(DBG_ERR, "Document is in read-only mode now");
		throw ReadOnlyDocumentException("Document is in read-only mode.");
	}
	
	// checks actual revision
	if(xref->getActualRevision())
	{
		kernelPrintDbg(DBG_ERR, "Document is not in latest revision");
		throw ReadOnlyDocumentException("Document is in read-only mode.");
	}
	
	// we are in the newest revision, so changes can be saved
	// delegates all work to the XRefWriter and set change to 
	// mark, that no changes were stored
	xref->saveChanges(newRevision);
	change=false;
}

void CPdf::clone(FILE * file)const
{
using namespace debug;

	kernelPrintDbg(DBG_DBG, "");

	if(!file)
	{
		kernelPrintDbg(DBG_ERR, "output file is NULL");
		return;
	}
	
	// delagates to XRefWriter
	xref->cloneRevision(file);
}

void CPdf::changeRevision(revision_t revisionNum)
{
	kernelPrintDbg(DBG_DBG, "");
	
	// set revision xref->changeRevision
	xref->changeRevision(revisionNum);
	
	// prepares internal structures for new revision
	initRevisionSpecific();
}

} // end of pdfobjects namespace
