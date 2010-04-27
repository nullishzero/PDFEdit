/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006-2009  PDFedit team: Michal Hocko,
 *                                        Jozef Misutka,
 *                                        Martin Petricek
 *                   Former team members: Miroslav Jahoda
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
#include "kernel/static.h"

#include "kernel/cobject.h"
#include "kernel/cobjecthelpers.h"
#include "kernel/cpdf.h"
#include "kernel/cpage.h"
#include "kernel/factories.h"
#include "utils/debug.h"
#include "kernel/cpageattributes.h"
#include "kernel/pdfedit-core-dev.h"
#include "kernel/streamwriter.h"

using namespace boost;
using namespace std;
using namespace debug;

typedef std::vector<boost::shared_ptr<pdfobjects::IProperty> > ChildrenStorage;

namespace pdfobjects
{

// initializes global list of alive pdf instances
CPdf::CPdfListContainer CPdf::allPdfs = CPdf::CPdfListContainer();

namespace utils 
{

/** Operator for output stream with PageTreeNodeType enumeration type.
 * @param stream Stream, where to print.
 * @param nodeType type to print.
 *
 * Prints human readable from of page tree node enumeration value.
 *
 * @return reference to given stream.
 */
ostream & operator<<(ostream & stream, PageTreeNodeType nodeType)
{
	switch(nodeType)
	{
		case InterNode:
			stream << "InterNode";
			break;
		case LeafNode:
			stream << "LeafNode";
			break;
		case RootNode:
			stream << "RootNode";
			break;
		case UnknownNode:
			stream << "UnknownNode";
			break;
		case ErrorNode:
			stream << "ErrorNode";
			break;
	}

	return stream;
}

shared_ptr<CDict> getPageTreeRoot(const boost::shared_ptr<CPdf> &pdf)
{
	shared_ptr<CDict> result;
	
	try
	{
		shared_ptr<IProperty> pagesProp=pdf->getDictionary()->getProperty("Pages");
		if(!isRef(pagesProp))
			// returns null dictionary
			return result;

		return getCObjectFromRef<CDict>(pagesProp);
	}catch(CObjectException & )
	{
	}

	// returns null dictionary
	return result;
}

PageTreeNodeType getNodeType(const boost::shared_ptr<IProperty> &nodeProp)throw()
{
	PageTreeNodeType nodeType=UnknownNode;

	// checks nodeProp - must be dictionary or reference to dictionary
	shared_ptr<CDict> nodeDict;
	if(isDict(nodeProp))
		nodeDict=IProperty::getSmartCObjectPtr<CDict>(nodeProp);
	else
		if(isRef(nodeProp))
		{
			try
			{
				nodeDict=getCObjectFromRef<CDict>(nodeProp);
			}catch(ElementBadTypeException &)
			{
				// target is not a dictionary
				return ErrorNode;
			}
		}
		else
			// property is not dictionary nor reference
			return ErrorNode;
	
	// checks root node at first
	shared_ptr<CPdf> pdf=nodeProp->getPdf().lock();
	assert(pdf);
	shared_ptr<CDict> rootDict=getPageTreeRoot(pdf);
	if(rootDict==nodeDict)
		// root dictionary found and it is same as internode
		return RootNode;
	
	// given node is not root of page tree, chcecks Type field
	if(nodeDict->containsProperty("Type"))
	{
		shared_ptr<IProperty> nodeType=nodeDict->getProperty("Type");
		try
		{
			if(isRef(nodeType))
				nodeType=getCObjectFromRef<CName>(nodeType);
			CName::Value typeName=getValueFromSimple<CName>(nodeType);
			if(typeName=="Page")
				return LeafNode;
			if(typeName=="Pages")
				return InterNode;
		}catch(CObjectException &)
		{
			// bad typed field
		}
		
		return UnknownNode;
	}

	// type field not found, so tries to determine dictionary type according
	// existing fields.
	// Internode should contain at least Kids array field
	if(nodeDict->containsProperty("Kids"))
	{
		shared_ptr<IProperty> kidsProp=nodeDict->getProperty("Kids");
		if(isArray(kidsProp))
			return InterNode;
		if(isRef(kidsProp))
		{
			// Kids property is reference, so checks whether it refer to array,
			// if yes, returns InterNode
			try
			{
				getCObjectFromRef<CArray>(kidsProp);
				return InterNode;
			}catch(...)
			{
				// target is not an array, so it's not intermediate node or it
				// is demaged node
			}
		}
	}

	return nodeType;
}

template<typename Container>
void getKidsFromInterNode(const boost::shared_ptr<CDict> & interNodeDict, Container & container)throw()
{
	container.clear();

	// tries to get Kids array
	if(interNodeDict->containsProperty("Kids"))
	{
		shared_ptr<IProperty> kidsProp=interNodeDict->getProperty("Kids");
		shared_ptr<CArray> kidsArray;
		if(isRef(kidsProp))
		{
			try
			{
				kidsArray=getCObjectFromRef<CArray>(kidsProp);
			}catch(CObjectException &)
			{
				// target is not an array
				return;
			}
		}else
		{
			if(!isArray(kidsProp))
				// not an array
				return;
			kidsArray=IProperty::getSmartCObjectPtr<CArray>(kidsProp);
		}
		
		// fills given container with all children
		kidsArray->_getAllChildObjects(container);
	}

}

namespace {
	
/** Updates value for given key in cache.
 * @param key Key value for cache entry.
 * @param value Value for given key for cache entry.
 * @param cache Cache mapping structure.
 *
 * Updates current mapping or create new with [key, value] pair.
 * <br>
 * Given template parameter must by associateve container compliant. 
 */
template <typename CacheType>
void updateCache(
		const typename CacheType::key_type & key, 
		const typename CacheType::mapped_type & value, 
		CacheType & cache)
{
using namespace debug;
	
	utilsPrintDbg(DBG_DBG, "cache key="<<key<<" value="<<value);
	typename CacheType::iterator pos=cache.find(key);
	if(pos!=cache.end())
	{
		// value is present in mapping
		//  NOTE it means that value has not been discarded before update
		utilsPrintDbg(DBG_WARN, pos->first<<" already cached with value="<<pos->second<<". Rewriting to value="<<value);
		pos->second=value;
		return;
	}

	// adds mapping for this reference
	utilsPrintDbg(DBG_DBG, "new cache entry: key="<<key<<" value="<<value);
	cache.insert(typename CacheType::value_type(key, value));
}

/** Gets value associated with given key.
 * @param key Key value for cache entry.
 * @param value Reference where to store value associated to given key.
 * @param cache Cache mapping structure.
 *
 * Tries to find mapping with given key and if found, sets given value parameter
 * with found one and returns with true.
 * <br>
 * Given template parameter must by associateve container compliant. 
 *
 * @return true if mapping was found, false otherwise.
 */
template<class CacheType>
bool getCachedValue(const typename CacheType::key_type & key, typename CacheType::mapped_type & value, CacheType & cache)
{
using namespace debug;

	utilsPrintDbg(DBG_DBG, "key="<<key);
	typename CacheType::iterator pos=cache.find(key);
	if(pos!=cache.end())
	{
		utilsPrintDbg(DBG_DBG, "cache entry found. key="<<pos->first<<" value="<<pos->second);
		value=pos->second;
		return true;
	}
	utilsPrintDbg(DBG_DBG, "no cache entry found for "<<key);

	return false;
}

/** Discards cache entry with given key.
 * @param key Key value for cache entry.
 * @param cache Cache mapping structure.
 *
 * Tries to find entry with given key and if found, removes it from cache.
 * <br>
 * Given template parameter must by associateve container compliant. 
 */
template<typename CacheType>
void discardCachedEntry(const typename CacheType::key_type & key, CacheType & cache)
{
	utilsPrintDbg(DBG_DBG, "key="<<key);
	typename CacheType::iterator pos=cache.find(key);
	if(pos!=cache.end())
	{
		utilsPrintDbg(DBG_DBG, "cache entry found. key="<<pos->first<<" value="<<pos->second<<". Discarding");
		cache.erase(pos);
	}else
		utilsPrintDbg(DBG_DBG, "no cache entry for "<<key);
}

/** Discards whole given cache.
 * @param cache Cache to clear.
 *
 * Discards all entries in cache.
 * <br>
 * Given CacheType must provide clear method.
 *
 */
template<typename CacheType>
void clearCache(CacheType & cache)
{
	cache.clear();
}

/** Discards cached entries for intermediate node and its subtree.
 * @param ref Reference of intermediate node for mapping.
 * @param pdf Pdf for intermediate node.
 * @param cache Cache mapping structure.
 * @param withSubTree Flag to control also sub tree discarding (false by
 * default).
 *
 * Removes mapping with ref key (if exists). If given withSubTree flag is set to
 * true, also dereferences intermediate node dictionary (uses 
 * pdf.getIndirectProperty) and discards (recursivelly) all intermediate nodes 
 * under given one. This should be used if intermediate node is removed from
 * tree (all subnodes are removed too and so should be discarded).
 */
void discardKidsCountCache(
		IndiRef & ref, 
		boost::shared_ptr<CPdf> pdf, 
		PageTreeNodeCountCache & cache, 
		bool withSubTree=false)
{
using namespace debug;

	discardCachedEntry(ref, cache);

	if(!withSubTree)
		return;

	shared_ptr<IProperty> nodeProp=pdf->getIndirectProperty(ref);	
	if(getNodeType(nodeProp)>=InterNode)
	{
		ChildrenStorage childs;
		assert(isDict(nodeProp));
		shared_ptr<CDict> nodeDict=IProperty::getSmartCObjectPtr<CDict>(nodeProp);
		getKidsFromInterNode(nodeDict, childs);
		utilsPrintDbg(DBG_DBG, "discarding all nodes in "<<ref<<" subtree");
		for(ChildrenStorage::iterator i=childs.begin(); i!=childs.end(); ++i)
		{
			shared_ptr<IProperty> child=*i;
			if(!isRef(child))
				// skip array mess
				continue;
			IndiRef childRef=getValueFromSimple<CRef>(child);
			discardKidsCountCache(childRef, pdf, cache, true);
		}
		utilsPrintDbg(DBG_DBG, "all nodes in "<<ref<<" subtree discarded");
	}
}


} // end of anonymous namespace for PageTreeNodeCountCache manipulation

size_t getKidsCount(const boost::shared_ptr<IProperty> & interNodeProp, PageTreeNodeCountCache * cache)throw()
{	
	// leaf node adds one direct page in page tree
	if(getNodeType(interNodeProp)==LeafNode)
		return 1;
	
	// gets dictionary from given property. If reference, gets target object. If
	// it is not a dictionary, returns with 0
	shared_ptr<CDict> interNodeDict;
	if(isRef(interNodeProp))
	{
		try
		{
			interNodeDict=getCObjectFromRef<CDict>(interNodeProp);
		}catch(CObjectException & )
		{
			// target is not a dictionary
			return 0;
		}
	}else
	{
		if(!isDict(interNodeProp))
			return 0;
		interNodeDict=IProperty::getSmartCObjectPtr<CDict>(interNodeProp);
	}
	
	// tries cache at first
	size_t count;
	if(cache)
	{
		IndiRef ref=interNodeDict->getIndiRef();
		if(getCachedValue(ref, count, *cache))
			return count;
	}


	// we assume inter node, so collects all children and calculates total
	// direct page count (calls recursively - just for referencies because those
	// should be stored in array, everything different is just mess)
	count=0;
	ChildrenStorage children;
	getKidsFromInterNode(interNodeDict, children);
	for(ChildrenStorage::const_iterator i=children.begin(); i!=children.end(); ++i)
	{
		shared_ptr<IProperty> childProp=*i;
		if(isRef(childProp))
			count+=getKidsCount(childProp, cache);
	}

	// creates cache entry for this ref
	if(cache)
	{
		IndiRef ref=interNodeDict->getIndiRef();
		updateCache(ref, count, *cache);
	}

	return count;
}

boost::shared_ptr<CDict> findPageDict(
		const boost::shared_ptr<CPdf> &pdf, 
		const boost::shared_ptr<IProperty> &pagesDict, 
		size_t startPos, size_t pos, 
		PageTreeNodeCountCache * cache)
{
	utilsPrintDbg(DBG_DBG, "startPos=" << startPos << " pos=" << pos);
	if(startPos > pos)
	{
		utilsPrintDbg(DBG_ERR, "startPos("<<startPos<<") > pos("<<pos<<")");
		// impossible to find such page
		throw PageNotFoundException(pos);
	}

	// dictionary smart pointer holder
	// it is initialized according pagesDict parameter - if it is reference
	// it has to be dereferenced
	shared_ptr<CDict> dict_ptr;

	// checks if given parameter is reference and if so, dereference it
	// using getIndirectProperty method and casts to dict_ptr
	// otherwise test if given type is dictionary and casts to dict_ptr
	if(isRef(pagesDict))
	{
		utilsPrintDbg(DBG_DBG, "pagesDict is reference");
		try
		{
			dict_ptr=getCObjectFromRef<CDict>(pagesDict);
		}catch(ElementBadTypeException & )
		{
			// malformed pdf
			utilsPrintDbg(DBG_ERR, "pagesDict "<< pagesDict <<" doesn't refer to dictionary");
			throw ElementBadTypeException("pagesDict");
		}
	}else
	{
		if(!isDict(pagesDict))
		{
			utilsPrintDbg(DBG_ERR, "pagesDict is not dictionary type="<< pagesDict->getType());
			// maloformed pdf
			throw ElementBadTypeException("pagesDict");
		}
		dict_ptr=IProperty::getSmartCObjectPtr<CDict>(pagesDict);
	}

	// target dictionary must be defined now
	assert(dict_ptr.get());

	// gets dictionary node type
	PageTreeNodeType nodeType=getNodeType(dict_ptr);
	
	// if type is Page then we have page dictionary and so start_pos and pos 
	// must match otherwise it is not possible to find page at given position
	if(nodeType==LeafNode)
	{
		utilsPrintDbg(DBG_DBG, "Page node is direct page");
		// everything ok 
		if(startPos == pos)
		{
			utilsPrintDbg(DBG_DBG, "Page found");
			return dict_ptr;
		}
		
		// unable to find
		utilsPrintDbg(DBG_ERR, "Page not found startPos="<<startPos);
		throw PageNotFoundException(pos);
	}

	// internode or root node
	if(nodeType>=InterNode)
	{
		utilsPrintDbg(DBG_DBG, "Page node is intermediate");

		// calculates direct page count under this inter node rather than use
		// Count field (which may be malformed)
		int count=getKidsCount(dict_ptr, cache);

		utilsPrintDbg(DBG_DBG, "InterNode has " << count << " pages");

		// check if this subtree contains enought direct pages 
		if(count + startPos <= pos )
		{
			utilsPrintDbg(DBG_ERR, "page can't be found under this subtree startPos=" << startPos);
			// no way to find given position under this subtree
			throw PageNotFoundException(pos);
		}

		// PAGE IS IN THIS SUBTREE, we have to find where
		
		// gets Kids array from pages dictionary and gets all its children
		ChildrenStorage children;
		getKidsFromInterNode(dict_ptr, children);

		// min_pos holds minimum position for actual child (at the begining
		// startPos value and incremented by page number in node which can't
		// contain pos - normal page 1 and Pages their count).
		size_t min_pos=startPos, index=0;
		for(ChildrenStorage::iterator i=children.begin(); i!=children.end(); ++i, ++index)
		{
			shared_ptr<IProperty> child=*i;

			if(!isRef(child))
			{
				utilsPrintDbg(DBG_WARN, "Kid["<<index<<"] is not reference. type="<<child->getType()<<". Ignoring");
				continue;
			}
			
			// all members of Kids array have to be either intermediate nodes or
			// leaf nodes - all other are ignored
			PageTreeNodeType nodeType=getNodeType(child);
			if(nodeType!=InterNode && nodeType!=RootNode && nodeType!=LeafNode)
			{
				utilsPrintDbg(DBG_WARN, "Kid["<<index<<"] is not valid page tree node. nodeType="<<nodeType<<". Ignoring");
				continue;
			}

			// gets child dictionary (everything is checked, so no exception can
			// be thrown here)
			shared_ptr<CDict> child_ptr=getCObjectFromRef<CDict>(child); 
			
			utilsPrintDbg(DBG_DBG, "kid["<<index<<"] node type="<<nodeType);

			// Page dictionary (leaf node) is ok if min_pos equals pos, 
			// otherwise it is skipped - can't use recursion here because it's
			// not an error that this page is not correct one (recursion would
			// throw an exception because it assumes that page must be found in
			// searched subtree)
			// min_pos is incremented if this page is not searched one
			if(nodeType==LeafNode)
			{
				if(min_pos == pos)
				{
					utilsPrintDbg(DBG_DBG, "page at pos="<<pos<<" found. Node reference "<<child_ptr->getIndiRef());
					return child_ptr;
				}
				++min_pos;
				continue;
			}

			// Pages dictionary is checked for its page count and if pos can
			// be found there, starts recursion - value is calculated rather
			// than used from Count field (which may be malformed).
			// Otherwise increment min_pos with its count and continues
			if(nodeType==InterNode)
			{
				int count=getKidsCount(child_ptr,cache);

				if(min_pos + count > pos )
					// pos IS in this subtree
					return findPageDict(pdf, child_ptr, min_pos, pos, cache);

				// pos is not in this subtree
				// updates min_pos with its count and continues
				min_pos+=count;

				continue;
			}
		}
		assert(!"Never can get here. Possibly bug!!!");
	}
	
	// should not happen - malformed pdf document
	utilsPrintDbg(DBG_ERR, "pagesDict dictionary is not valid page tree node. Nodetype="<<nodeType);
	throw ElementBadTypeException("pagesDict");
}

/** Searches node in page tree structure.
 * @param pdf Pdf where to search.
 * @param superNode Page tree node where to search (may be intermediate or leaf).
 * @param node Node to search for.
 * @param startValue Position of the superNode.
 * @param cache Cache for reference to page count mapping.
 *
 * At first checks if node and superNode are same nodes (uses == operator to
 * compare) and if so, returns startPos. Otherwise tries to get node type 
 * (uses getNodeType helper function). 
 * In Page case (LeafNode) returns with startValue if given nodes are same or 
 * 0 (page not found). This the end of recursion.
 * If it is intermediate node, goes through Kids array and recursively calls 
 * this method for each element until recursion returns with non 0 result.
 * This means the end of recursion. startValue is actualized for each Kid's
 * element with 0 recursion return value (Leaf node element increases by 1, 
 * intermediate node element by getKidsCount value).
 * <br>
 * If node is found as direct Kids member (this means that reference of target 
 * node is direct member of Kids array), then determines if the node position 
 * is unambiguous - checks whether reference to the node is unique in Kids array. 
 * If not throws exception. This means that searchTreeNode function is not able 
 * to definitively determine node's position.
 * <br>
 * Function tries to find node position also in page tree structure which 
 * doesn't follow pdf specification. All wierd page tree elements are ignored 
 * and just those which may stand for intermediate or leaf nodes are considered.
 * Also doesn't use Count and Parent fields information during searching.
 * <br>
 * Uses getKidsCount function internally to get intermediate leaf nodes count. 
 * getKidsCount method requieres also cache parameter which stores already known 
 * nodes to their counts mapping. This function just delegates given cache 
 * parameter to getPageCount and doesn't care for it much more. Note that if 
 * parameter is NULL, cache is not used.
 *
 * @throw AmbiguousPageTreeException if page tree is ambiguous and node position
 * can't be determined.
 *
 * @return Position of the node or 0 if node couldn't be found under this
 * superNode.
 */
size_t searchTreeNode(
		shared_ptr<const CPdf> pdf, 
		shared_ptr<CDict> superNode, 
		shared_ptr<CDict> node, 
		size_t startValue, 
		PageTreeNodeCountCache * cache)
{
	size_t position=0;

	utilsPrintDbg(DBG_DBG, "startPos="<<startValue);
	
	// if nodes are same, startValue is returned.
	if(superNode==node)
	{
		utilsPrintDbg(DBG_DBG, "Page found");
		return startValue;
	}

	// gets super node type
	PageTreeNodeType superNodeType=getNodeType(superNode);
	
	// if type is Page we return with 0, because node is not this superNode and
	// there is nowhere to go
	if(superNodeType==LeafNode)
		return 0;

	// if node is not also intermediate node (or root node) - page tree is not 
	// well formated and we skip this node
	if(superNodeType<InterNode)
	{
		utilsPrintDbg(DBG_WARN, "Given dictionary is not correct page tree node. type="<<superNodeType);
		return 0;
	}

	// given node is intermediate and so searches recursivelly all its kids
	// until first returns successfully
	ChildrenStorage children;
	ChildrenStorage::iterator i;
	size_t index=0;
	getKidsFromInterNode(superNode, children);
	for(i=children.begin(); i!=children.end(); ++i, ++index)
	{
		shared_ptr<IProperty> child=*i;
		
		// each element has to be reference
		if(!isRef(child))
		{
			// we will just print warning and skips this element
			utilsPrintDbg(DBG_WARN, "Kids["<<index<<"] is not an reference. type="<<child->getType()<<". Ignoring");
			continue;
		}

		// ignores also all non leaf and non intermediates nodes
		PageTreeNodeType nodeType=getNodeType(child);
		if(nodeType!=LeafNode && nodeType!=InterNode)
		{
			utilsPrintDbg(DBG_WARN, "Kids["<<index<<"] is not valid page tree element. type="<<nodeType<<". Ignoring");
			continue;
		}

		// dereference target dictionary - never throws, because we have checked
		// node type
		shared_ptr<CDict> elementDict_ptr=getCObjectFromRef<CDict>(child);
	
		// compares elementDict_ptr (kid) with node, if they are same, returns
		// startValue
		if(elementDict_ptr==node)
		{
			position=startValue;
			break;
		}

		// recursively checks subnode if it is intermediate - if found 
		// re-return position
		if((nodeType!=LeafNode) && (position=searchTreeNode(pdf, elementDict_ptr, node, startValue, cache)))
			break;

		// node is not under elementDict_ptr node so updates startValue
		// according skipped page count (page is 1 and intermediate node the
		// number of direct pages under it)
		startValue+=getKidsCount(elementDict_ptr, cache);
	}

	// checks for reference to same node in children from (i, children.end())
	if(i!=children.end())
	{
		// i points to last checked element, so starts checking from next
		// position
		++i;++index;
		IndiRef nodeRef=node->getIndiRef();
		for(;i!=children.end(); ++i, ++index)
		{
			shared_ptr<IProperty> child=*i;
			if(isRef(child) && getValueFromSimple<CRef>(child)==nodeRef)
			{
				utilsPrintDbg(DBG_WARN, "Internode "<<superNode->getIndiRef()<<" is ambiguous. Kids["<<index<<"] duplicates reference to node.");
				throw AmbiguousPageTreeException();
			}
		}
	}
	
	return position;
}

size_t getNodePosition(const shared_ptr<CPdf> &pdf, const shared_ptr<IProperty> &node, PageTreeNodeCountCache * cache)
{
	utilsPrintDbg(DBG_DBG, "");
	// node must be from given pdf
	if(node->getPdf().lock()!=pdf)
	{
		utilsPrintDbg(DBG_ERR, "Node is not from given pdf isntance.");
		throw PageNotFoundException(0);
	}
	
	// gets page tree root - if not found, then PageNotFoundException is thrown
	shared_ptr<CDict> rootDict_ptr=getPageTreeRoot(pdf);
	if(!rootDict_ptr.get())
		throw PageNotFoundException(0);
	

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
	if(isRef(node))
		nodeDict_ptr=getCObjectFromRef<CDict>(node);
	else
		nodeDict_ptr=IProperty::getSmartCObjectPtr<CDict>(node);
		
	utilsPrintDbg(DBG_DBG, "Starts searching");
	size_t pos=searchTreeNode(pdf, rootDict_ptr, nodeDict_ptr, 1, cache);
	utilsPrintDbg(DBG_DBG, "Searching finished. pos="<<pos);
	if(pos)
		return pos;

	// node not found
	throw PageNotFoundException(0);
}

bool isNodeDescendant(const boost::shared_ptr<CPdf>& pdf, const IndiRef &parent, const shared_ptr<CDict> &child)
{
using namespace utils;

	if(!child->containsProperty("Parent"))
	{
		// child has no parent, so can't be descendant
		return false;
	}
	
	// gets parent property
	shared_ptr<IProperty> parentProp=child->getProperty("Parent");
	if(!isRef(parentProp))
	{
		// parent is incorect
		return false;
	}

	// compares Parent property value with given parent reference. If they are
	// same, we are done and node child node is real parent child - same is true
	// for all nodes in recursion (parent is transitive relation)
	IndiRef parentRef=getValueFromSimple<CRef>(parentProp);
	if(parent==parentRef)
		return true;

	// referencies are not same, so gets parent dictionary and checks its parent
	try
	{
		shared_ptr<CDict> parentDict=getCObjectFromRef<CDict>(parentProp);
		return isNodeDescendant(pdf, parent, parentDict);
	}catch(CObjectException & )
	{
		// parent indirect object is not valid
		return false;
	}
}

bool isEncrypted(const boost::shared_ptr<CPdf> &pdf)
{
 	return (gTrue == pdf->getCXref()->isEncrypted());
}

} // end of utils namespace

void CPdf::registerPageTreeObservers(boost::shared_ptr<IProperty> & prop)
{
using namespace boost;
using namespace std;
using namespace pdfobjects::utils;

	if(!isDict(prop)&&!isRef(prop))
		return;

	// gets dictionary from given property
	shared_ptr<CDict> dict_ptr;
	if(isRef(prop))
	{
		try
		{
			dict_ptr=getCObjectFromRef<CDict>(prop);
		}catch(CObjectException &)
		{
			// prop is not dictionary
			return;
		}
	}else
		dict_ptr=IProperty::getSmartCObjectPtr<CDict>(prop);
	
	// registers observer for Kids property change notification
	REGISTER_SHAREDPTR_OBSERVER(dict_ptr, pageTreeNodeObserver);
	
	// gets Kids field from dictionary and all children from array and 
	// registers PageTreeKidsObserver observer to array and to each member 
	// reference. If Kids property is reference, registers PageTreeNodeObserver
	// to it to enable notification also reference value notification
	if(!dict_ptr->containsProperty("Kids"))
		return;

	shared_ptr<IProperty> kidsProp_ptr=dict_ptr->getProperty("Kids");
	shared_ptr<CArray> kids_ptr;
	if(isRef(kidsProp_ptr))
	{
		// Kids property is reference - this is not offten but may occure and
		// reference replacement should invalidate whole target array so
		// observer is needed also to reference
		utilsPrintDbg(DBG_DBG, "Kids array is reference. Registering obsever.");
		REGISTER_SHAREDPTR_OBSERVER(kidsProp_ptr, pageTreeNodeObserver);
		
		// gets target object
		try
		{
			kids_ptr=getCObjectFromRef<CArray>(kidsProp_ptr);
			// creates mapping for this indirect kids array to be able to get
			// intermediate node later for array members
			// Uses dereferenced array's indiref to get indirect reference to
			// array.
			// kidsProp_ptr is direct internode member, so getIndiRef is ok here
			// as value.
			updateCache(kids_ptr->getIndiRef(), kidsProp_ptr->getIndiRef(), pageTreeKidsParentCache);
		}catch(CObjectException &)
		{
			// target is not an array, keeps kids_ptr uninitialized and do the
			// handling later
		}
	}else
		if(isArray(kidsProp_ptr))
			kids_ptr=IProperty::getSmartCObjectPtr<CArray>(kidsProp_ptr);
	if(!kids_ptr.get())
	{
		// kids_ptr is not initialized, what means that Kids array is not typed
		// correctly (it is not array or reference to array).
		utilsPrintDbg(DBG_WARN, "Node's Kids property is not an array or reference to array.");
		return;
	}

	// registers PageTreeKidsObserver observer to array and all its reference elements
	// all other may be ignored because they doesn't follow specification and so
	// can't point to intermediate node (their value change is not important and
	// their existence in Kids array is handled by obsever on array)
	utilsPrintDbg(DBG_DBG, "Kids array found. Registering obsever.");
	REGISTER_SHAREDPTR_OBSERVER(kids_ptr, pageTreeKidsObserver);
	ChildrenStorage container;
	kids_ptr->_getAllChildObjects(container);
	for(ChildrenStorage::iterator i=container.begin(); i!=container.end(); ++i)
	{
		shared_ptr<IProperty> elemProp_ptr=*i;
		if(isRef(elemProp_ptr))
		{
			REGISTER_SHAREDPTR_OBSERVER(elemProp_ptr, pageTreeKidsObserver);
			registerPageTreeObservers(elemProp_ptr);
		}
	}

	utilsPrintDbg(DBG_DBG, "All subnodes done for "<<dict_ptr->getIndiRef());
}

void CPdf::unregisterPageTreeObservers(boost::shared_ptr<IProperty> & prop, bool cleanup)
{
using namespace boost;
using namespace std;
using namespace pdfobjects::utils;

	if(!isDict(prop)&&!isRef(prop))
		return;

	// gets dictionary from given property
	shared_ptr<CDict> dict_ptr;
	if(isRef(prop))
	{
		try
		{
			dict_ptr=getCObjectFromRef<CDict>(prop);
		}catch(CObjectException &)
		{
			// prop is not dictionary
			return;
		}
	}else
		dict_ptr=IProperty::getSmartCObjectPtr<CDict>(prop);
	
	// If we are not doing cleanup, we don't want to unregister observers 
	// from node which may be still in the page tree (this situation may 
	// occure if this position of node is ambiguous). In such situation, 
	// keeps observers
	if(!cleanup)
	{
		bool unregister=true;
		try
		{
			unregister=!getNodePosition(_this.lock(), dict_ptr, &nodeCountCache);
		}catch(AmbiguousPageTreeException &)
		{
			// node is still in the tree and even more it is still ambiguous
			unregister=false;	
		}catch(...)
		{
			// all other exceptions means that node is not found for some reason
			unregister=true;
		}
		if(!unregister)
		{
			kernelPrintDbg(DBG_WARN, "Keeps observers for "
					<<dict_ptr->getIndiRef()
					<<" because node is still in the tree.");
			return;
		}
	}
	
	// unregisters observer from Kids property change notification
	UNREGISTER_SHAREDPTR_OBSERVER(dict_ptr, pageTreeNodeObserver);
	
	// gets Kids field from dictionary and all children from array and 
	// unregisters PageTreeKidsObserver observer from array and from each member 
	// reference. If Kids property is reference, unregisters PageTreeNodeObserver
	// from it too
	if(!dict_ptr->containsProperty("Kids"))
		return;

	shared_ptr<IProperty> kidsProp_ptr=dict_ptr->getProperty("Kids");
	shared_ptr<CArray> kids_ptr;
	if(isRef(kidsProp_ptr))
	{
		// Kids property is reference - this is not offten but may occure and
		// reference replacement should invalidate whole target array so
		// observer is needed also to reference
		utilsPrintDbg(DBG_DBG, "Kids array is reference. Unregistering obsever.");
		UNREGISTER_SHAREDPTR_OBSERVER(kidsProp_ptr, pageTreeNodeObserver);
		
		// gets target object
		try
		{
			kids_ptr=getCObjectFromRef<CArray>(kidsProp_ptr);
			// creates mapping for this indirect kids array to be able to get
			// intermediate node later for array members
			// Uses dereferenced array's indiref to get indirect reference to
			// array.
			// kidsProp_ptr is direct internode member, so getIndiRef is ok here
			// as value.
			discardCachedEntry(kids_ptr->getIndiRef(), pageTreeKidsParentCache);
		}catch(CObjectException &)
		{
			// target is not an array, keeps kids_ptr uninitialized and do the
			// handling later
		}
	}else
		if(isArray(kidsProp_ptr))
			kids_ptr=IProperty::getSmartCObjectPtr<CArray>(kidsProp_ptr);
	if(!kids_ptr.get())
	{
		// kids_ptr is not initialized, what means that Kids array is not typed
		// correctly (it is not array or reference to array).
		utilsPrintDbg(DBG_WARN, "Node's Kids property is not an array or reference to array.");
		return;
	}

	// registers PageTreeKidsObserver observer to array and all its reference elements
	// all other may be ignored because they doesn't follow specification and so
	// can't point to intermediate node (their value change is not important and
	// their existence in Kids array is handled by obsever on array)
	utilsPrintDbg(DBG_DBG, "Kids array found. Unregistering obsever.");
	UNREGISTER_SHAREDPTR_OBSERVER(kids_ptr, pageTreeKidsObserver);
	ChildrenStorage container;
	kids_ptr->_getAllChildObjects(container);
	for(ChildrenStorage::iterator i=container.begin(); i!=container.end(); ++i)
	{
		shared_ptr<IProperty> elemProp_ptr=*i;
		if(isRef(elemProp_ptr))
		{
			UNREGISTER_SHAREDPTR_OBSERVER(elemProp_ptr, pageTreeKidsObserver);
			unregisterPageTreeObservers(elemProp_ptr, cleanup);
		}
	}

	utilsPrintDbg(DBG_DBG, "All subnodes done for "<<dict_ptr->getIndiRef());
}

void CPdf::PageTreeRootObserver::notify(
		boost::shared_ptr<IProperty> newValue, 
		boost::shared_ptr<const observer::IChangeContext<IProperty> > context) const throw()
{
using namespace boost;
using namespace debug;
using namespace observer;
using namespace utils;

	assert(isActive());
	shared_ptr<IProperty> oldValue;
	if(!context)
	{
		kernelPrintDbg(DBG_WARN, "No context available. Ignoring calling.");
		return;
	}
	kernelPrintDbg(DBG_DBG, "context type="<<context->getType());
	// initializes oldValue from context
	switch(context->getType())
	{
		case BasicChangeContextType:
			{
				// Pages reference value has changed
				shared_ptr<const BasicChangeContext<IProperty> > basicContext=
					dynamic_pointer_cast<const BasicChangeContext<IProperty>, const IChangeContext<IProperty> >(context); 
				oldValue=basicContext->getOriginalValue();

				// both oldValue and newValue has to be referencies
				assert(isRef(oldValue));
				assert(isRef(newValue));
			}
			break;
		case ComplexChangeContextType:
			{
				// document catalog dictionary has changed. Checks valueId and
				// proceede just if Pages property has changed
				shared_ptr<const CDict::CDictComplexObserverContext > complexContex=
					dynamic_pointer_cast<const CDict::CDictComplexObserverContext, const IChangeContext<IProperty> >(context); 
				if(!complexContex)
				{
					kernelPrintDbg(DBG_WARN, "ComplexChangeContext contains unsupported property id.");
					return;
				}
				if(complexContex->getValueId()!="Pages")
					// different property from document catalog has changed
					return;

				oldValue=complexContex->getOriginalValue();

				// unregister observer from reference
				if(isRef(oldValue))
				{
					kernelPrintDbg(DBG_DBG, "unregistering obsever from old Pages property "<<oldValue);
					try
					{
						UNREGISTER_SHAREDPTR_OBSERVER(oldValue, pdf->pageTreeRootObserver);
					}catch(ObserverException & e)
					{
						kernelPrintDbg(DBG_ERR, "oldValue "<<oldValue<<" observer unregistration failed.");
					}
				}

				// registers 
				if(isRef(newValue))
				{
					kernelPrintDbg(DBG_DBG, "registering observer to new Pages property "<<newValue);
					REGISTER_SHAREDPTR_OBSERVER(newValue, pdf->pageTreeRootObserver);
				}
			}
			break;
		default:
			kernelPrintDbg(DBG_WARN, "Unsupported context type");
	}

	assert(oldValue.get());
	assert(newValue.get());

	// oldValue's target needs obsevers unregistration - it is not accessible
	// anymore
	if(isRef(oldValue))
	{
		try
		{
			shared_ptr<IProperty> oldValueDict=getCObjectFromRef<CDict>(oldValue);
			kernelPrintDbg(DBG_DBG, "unregistering observers from old page tree.");
			pdf->unregisterPageTreeObservers(oldValueDict);
		}catch(CObjectException & e)
		{
			IndiRef ref=getValueFromSimple<CRef>(oldValue);
			kernelPrintDbg(DBG_WARN, "oldValue's "<<ref<<" is not dictionary.");
		}catch(ObserverException & e)
		{
			kernelPrintDbg(DBG_ERR, "oldValue's "<<oldValue
					<<" target unregisterPageTreeObservers failed.");
		}
	}

	// invalidates pageCount
	pdf->pageCount=0;
	
	// removes and invalidates whole pageList
	kernelPrintDbg(DBG_DBG, "Invalidating pageList with "<<pdf->pageList.size()<<" elements");
	for(PageList::iterator i=pdf->pageList.begin(); i!=pdf->pageList.end(); ++i)
	{
		shared_ptr<CPage> page=i->second;
		page->invalidate();
	}
	pdf->pageList.clear();

	// clears nodeCountCache
	kernelPrintDbg(DBG_DBG, "Discarding nodeCountCache with "<<pdf->nodeCountCache.size()<<" entries");
	utils::clearCache(pdf->nodeCountCache);
	
	// registers new page tree root
	// checks newValue property type and if it is not reference to dictionary it
	// is skipped - because it is not valid page tree root
	if(!isRef(newValue))
	{
		kernelPrintDbg(DBG_WARN, "Pages property is not reference. type="<<newValue->getType());
		return;
	}
	IndiRef newValueRef=utils::getValueFromSimple<CRef>(newValue);
	shared_ptr<IProperty> newValueProp=pdf->getIndirectProperty(newValueRef);
	if(!isDict(newValueProp))
	{
		kernelPrintDbg(DBG_WARN, "Pages property doesn't refer to dictionary. type="<<newValueProp->getType());
		return;
	}

	// we have new page tree root dictionary and so observers have to be
	// registered
	kernelPrintDbg(DBG_DBG, "Registering obsevers to new page tree with root "<<newValueRef);
	pdf->registerPageTreeObservers(newValueProp);

	kernelPrintDbg(DBG_DBG, "PageTreeRootObserver finished");
}

void CPdf::PageTreeNodeObserver::notify(
		boost::shared_ptr<IProperty> newValue, 
		boost::shared_ptr<const observer::IChangeContext<IProperty> > context) const throw()
{
using namespace debug;
using namespace boost;
using namespace observer;

	assert(isActive());
	if(!context)
	{
		kernelPrintDbg(DBG_WARN, "No context available. Ignoring calling.");
		return;
	}
	shared_ptr<IProperty> oldValue;
	ChildrenStorage oldValues, newValues;
	kernelPrintDbg(DBG_DBG, "context type="<<context->getType());
	switch(context->getType())
	{
		case BasicChangeContextType:
			{
				// this means that node contains Kids array with reference type
				// and this reference has changed its value - both oldValue and
				// newValue has to be referencies
				shared_ptr<const BasicChangeContext<IProperty> > basicContext=
					dynamic_pointer_cast<const BasicChangeContext<IProperty>, const IChangeContext<IProperty> >(context); 
				oldValue=basicContext->getOriginalValue();
				assert(isRef(oldValue));
				assert(isRef(newValue));
			}
			break;
		case ComplexChangeContextType:
			{
				// this means that inter node dictionary has changed
				// if changed property is not Kids, immediatelly returns
				shared_ptr<const CDict::CDictComplexObserverContext > complexContex=
					dynamic_pointer_cast<const CDict::CDictComplexObserverContext, const IChangeContext<IProperty> >(context); 
				if(!complexContex)
				{
					kernelPrintDbg(DBG_WARN, "ComplexChangeContext contains unsupported property id.");
					return;
				}
				if(complexContex->getValueId()!="Kids")
					return;
				oldValue=complexContex->getOriginalValue();
				
				// unregisters observer from oldValue
				if(isRef(oldValue))
				{
					try
					{
						UNREGISTER_SHAREDPTR_OBSERVER(oldValue, pdf->pageTreeNodeObserver);
					}catch(ObserverException & e)
					{
						kernelPrintDbg(DBG_ERR, "unregisterObserver has failed for oldValue "<<oldValue);
					}
				}

				// newValue as reference means that we have new Kids field as
				// reference and so this observer has to be registered on it -
				// this enables notifying when reference value is changed
				// NOTE that observer to array is registered later
				if(isRef(newValue))
					REGISTER_SHAREDPTR_OBSERVER(newValue, pdf->pageTreeNodeObserver);
			}
			break;
		default:
			kernelPrintDbg(DBG_WARN, "unsuported context type");
	}

	// oldValue is set from context now - tries to get all array members, if it
	// is reference gets target indirect object. If can't get array, doesn't
	// fill anything
	try
	{
		// collects all children from array
		shared_ptr<CArray> kidsArray;
		if(isRef(oldValue))
			kidsArray=utils::getCObjectFromRef<CArray>(oldValue);
		else
			if(isArray(oldValue))
				kidsArray=IProperty::getSmartCObjectPtr<CArray>(oldValue);
		if(kidsArray.get())
		{
			kidsArray->_getAllChildObjects(oldValues);

			// unregisters observer from kids oldValue array
			UNREGISTER_SHAREDPTR_OBSERVER(kidsArray, pdf->pageTreeKidsObserver);
		}
		kernelPrintDbg(DBG_DBG, "oldValues collected. size="<<oldValues.size());
	}catch (CObjectException & e)
	{
		IndiRef ref=utils::getValueFromSimple<CRef>(oldValue);
		kernelPrintDbg(DBG_WARN, "oldValue "<<ref<<" doesn't refer to array.");
	}catch (ObserverException & e)
	{
		kernelPrintDbg(DBG_ERR, "oldValue's kidsArray doesn't have registered pageTreeKidsObserver");
	}

	// collects newValue's array elemenent - if newValue is reference, gets
	// target array 
	try
	{
		// collects all children from array
		shared_ptr<CArray> kidsArray;
		if(isRef(newValue))
			kidsArray=utils::getCObjectFromRef<CArray>(newValue);
		else
			if(isArray(newValue))
				kidsArray=IProperty::getSmartCObjectPtr<CArray>(newValue);
		if(kidsArray.get())
		{
			kidsArray->_getAllChildObjects(newValues);

			// registers pageTreeKidsObserver observer to the array
			REGISTER_SHAREDPTR_OBSERVER(kidsArray, pdf->pageTreeKidsObserver);
		}
		kernelPrintDbg(DBG_DBG, "newValues collected. size="<<newValues.size());
	}catch (CObjectException & e)
	{
		IndiRef ref=utils::getValueFromSimple<CRef>(oldValue);
		kernelPrintDbg(DBG_WARN, "newValue "<<ref<<" doesn't refer to array.");
	}

	// consolidates page tree under indirect parent of oldValue or newValue.
	// This is ok, because at least one of oldValue or newValue must be non
	// pNull and they must be direct members of node dictionary
	IndiRef interNodeRef=(!isNull(oldValue))?oldValue->getIndiRef():newValue->getIndiRef();
	try
	{
		shared_ptr<IProperty> interNodeProp=pdf->getIndirectProperty(interNodeRef);
		if(isDict(interNodeProp))
		{
			shared_ptr<CDict> interNode=IProperty::getSmartCObjectPtr<CDict>(interNodeProp);
			// if consolidatePageTree hasn't kept page count numbers, total number
			// of pages must be invalidated
			if(!pdf->consolidatePageTree(interNode, true))
				pdf->pageCount=0;
		}
	}catch(...)
	{
		// This should not happen, but we can live without it.
		// It is almost for sure, that there is some bug inside
		// consolidatePageTree if this happens
		kernelPrintDbg(DBG_CRIT, "consolidatePageTree has failed. Should not happen. Possibly bug.");
	}

	// removes all pages from removed array
	shared_ptr<IProperty> null(CNullFactory::getInstance());
	kernelPrintDbg(DBG_DBG, "Consolidating page list by removing oldValues.");
	size_t index=0;
	for(ChildrenStorage::iterator i=oldValues.begin(); i!=oldValues.end(); ++i, ++index)
	{
		shared_ptr<IProperty> child=*i;
		// consider just referencies, other elements are just mess in array
		// unregisters observers and consolidates pageList like this node has
		// been removed 
		if(isRef(child))
		{
			try
			{
				pdf->unregisterPageTreeObservers(child);
			}catch(ObserverException & e)
			{
				kernelPrintDbg(DBG_ERR, "kids["<<index<<"] unregisterPageTreeObservers has failed");
			}
			pdf->consolidatePageList(child, null);
		}
	}
	kernelPrintDbg(DBG_DBG, "Consolidating page list by adding newValues.");
	for(ChildrenStorage::iterator i=newValues.begin(); i!=newValues.end(); ++i)
	{
		shared_ptr<IProperty> child=*i;
		// consider just referencies, other elements are just mess in array
		// registers observers and consolidates pageList like this node has
		// been added 
		if(isRef(child))
		{
			pdf->consolidatePageList(null, child);
			pdf->registerPageTreeObservers(child);
		}
	}
}

void CPdf::PageTreeKidsObserver::notify(
		boost::shared_ptr<IProperty> newValue, 
		boost::shared_ptr<const observer::IChangeContext<IProperty> > context) const throw()
{
using namespace observer;
using namespace boost;
using namespace debug;
using namespace utils;

	assert(isActive());
	if(!context)
	{
		kernelPrintDbg(DBG_WARN, "No context available. Ignoring calling.");
		return;
	}
	ChangeContextType contextType=context->getType();
	kernelPrintDbg(DBG_DBG, "contextType="<<contextType);
	// gets original value from given context. It has to at least
	// BasicChangeContext
	shared_ptr<IProperty> oldValue;
	switch(contextType)
	{
		// This context means that just simple value has been changed and so
		// reference has changed its value - both oldValue and newValue has to
		// be pRef
		case BasicChangeContextType:
			{
				// this means that reference property has changed its value
				shared_ptr<const BasicChangeContext<IProperty> > basicContext=
					dynamic_pointer_cast<const BasicChangeContext<IProperty>, const IChangeContext<IProperty> >(context); 
				oldValue=basicContext->getOriginalValue();

				// both oldValue and newValue must be referencies
				assert(isRef(oldValue));
				assert(isRef(newValue));
			}
			break;
		case ComplexChangeContextType:
			{
				// this means that array content has changed
				shared_ptr<const CArray::CArrayComplexObserverContext > complexContex=
					dynamic_pointer_cast<const CArray::CArrayComplexObserverContext, const IChangeContext<IProperty> >(context); 
				if(!complexContex)
				{
					kernelPrintDbg(DBG_WARN, "ComplexChangeContext contains unsupported property id.");
					return;
				}
				// change value identificator is not important in this moment
				oldValue=complexContex->getOriginalValue();

				// unregisters obsever from reference element
				if(isRef(oldValue))
				{
					try
					{
						UNREGISTER_SHAREDPTR_OBSERVER(oldValue, pdf->pageTreeKidsObserver);
					}catch(ObserverException & e)
					{
						kernelPrintDbg(DBG_WARN, "oldValue observer unregistration failed");
					}
				}
				
				// newValue doesn't have observer registered yet and if it is
				// reference, registers observers to it and its whole subtree
				if(isRef(newValue))
				{
					REGISTER_SHAREDPTR_OBSERVER(newValue, pdf->pageTreeKidsObserver);
				}
			}
			break;
		default:
			// context type is not supported, so does nothing
			kernelPrintDbg(DBG_WARN, "Unsupported context type="<<contextType);
			return;
	}
	

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
		kernelPrintDbg(DBG_DBG, "Nothing to consolidate because newValue and oldValue are not CRef");
		return;
	}

	// if oldValue is reference to dictionary (node), this node needs observers
	// unregistration
	if(isRef(oldValue))
	{
		try
		{
			shared_ptr<IProperty> oldValueDict=getCObjectFromRef<CDict>(oldValue);
			pdf->unregisterPageTreeObservers(oldValueDict);
		}catch(CObjectException & e)
		{
			IndiRef ref=getValueFromSimple<CRef>(oldValue);
			kernelPrintDbg(DBG_WARN, "oldValue "<<ref<<" doesn't refer to dictionary.");
		}catch(ObserverException & e)
		{
			kernelPrintDbg(DBG_ERR, "oldValue "<<oldValue
					<<" unregisterPageTreeObservers has failed.");
		}
	}
		
	// consolidates page tree from newValue's indirect parent. If newValue is
	// CNull, uses oldValue's. Indirect reference can't be used directly,
	// although reference must be direct value (and so its indirect parent
	// reference was returned), because it is stored in Kids array which may be
	// indirect object and so reference to Kids array instead of intermediate
	// node would be returned. To prevent this problem, tries to use
	// pageTreeKidsParentCache and if indirect reference is associade with some
	// node, uses returned value.
	IndiRef ref=(newType!=pNull)
		?newValue->getIndiRef()
		:oldValue->getIndiRef();
	IndiRef parentRef=ref;
	if(getCachedValue(ref, parentRef, pdf->pageTreeKidsParentCache))
		kernelPrintDbg(DBG_DBG, "Uses pageTreeKidsParentCache with mapping from"<<ref<<" to "<<parentRef);
	shared_ptr<IProperty> parentProp_ptr=pdf->getIndirectProperty(parentRef);
	if(parentProp_ptr->getType()!=pDict)
	{
		// target of the parent reference is not dictionary,
		// this should not happen - some one is doing something nasty
		kernelPrintDbg(DBG_CRIT, "newValue's parent is not dictionary. THIS SHOUL NOT HAPPEN");
		return;
	}

	// starts consolidation from parent intermediate node
	shared_ptr<CDict> parentDict_ptr=IProperty::getSmartCObjectPtr<CDict>(parentProp_ptr);
	try
	{
		// if consolidatePageTree hasn't kept page count numbers, total number
		// of pages must be invalidated
		kernelPrintDbg(DBG_DBG, "consolidating page tree.");
		if(!pdf->consolidatePageTree(parentDict_ptr, true))
			pdf->pageCount=0;
	}catch(CObjectException & e)
	{
		kernelPrintDbg(DBG_ERR, "consolidatePageTree failed with cause="<<e.what());
	}

	// consolidates pageList 
	try
	{
		kernelPrintDbg(DBG_DBG, "consolidating page list.");
		pdf->consolidatePageList(oldValue, newValue);
	}catch(CObjectException &e)
	{
		kernelPrintDbg(DBG_ERR, "consolidatePageList failed with cause="<<e.what());
	}

	// kidsCount cache couldn't have beem discarded until now because it is used
	// in consolidatePageList
	if(isRef(oldValue))
	{
		IndiRef oldRef=getValueFromSimple<CRef>(oldValue);
		kernelPrintDbg(DBG_DBG, "discarding leaf count cache for "<<oldRef<<" subtree");
		discardKidsCountCache(oldRef, pdf->_this.lock(), pdf->nodeCountCache, true);
	}

	// if newValue is reference, registers observers to newValue dereferenced 
	// dictionary node sub tree
	if(isRef(newValue))
	{
		try
		{
			shared_ptr<IProperty> newValueDict=getCObjectFromRef<CDict>(newValue);
			pdf->registerPageTreeObservers(newValueDict);
		}catch(CObjectException & e)
		{
			IndiRef ref=getValueFromSimple<CRef>(newValue);
			kernelPrintDbg(DBG_WARN, "newValue "<<ref<<" doesn't refer to dictionary.");
		}
	}
	
	kernelPrintDbg(DBG_DBG, "observer handler finished");
}
void CPdf::unregisterPageObservers()
{
using namespace utils;
using namespace observer;

	if(!docCatalog.get())
		return;

	kernelPrintDbg(DBG_DBG, "Unregistering all observers for page tree");
	try
	{
		UNREGISTER_SHAREDPTR_OBSERVER(docCatalog, pageTreeRootObserver);
	}catch(ObserverException &e)
	{
		kernelPrintDbg(DBG_WARN, "document catalog observer unregistration failed.");
	}
	if(docCatalog->containsProperty("Pages"))
	{
		shared_ptr<IProperty> pagesProp=docCatalog->getProperty("Pages");
		if(isRef(pagesProp))
		{
			UNREGISTER_SHAREDPTR_OBSERVER(pagesProp, pageTreeRootObserver);
			shared_ptr<IProperty> pageTreeRoot=getPageTreeRoot(_this.lock());
			if(pageTreeRoot.get())
			{
				try
				{
					unregisterPageTreeObservers(pageTreeRoot, true);
				}catch(ObserverException &e)
				{
					kernelPrintDbg(DBG_WARN, "page tree root unregisterPageTreeObservers failed.");
				}
			}
		}

	}
}

void CPdf::initRevisionSpecific()
{
	kernelPrintDbg(debug::DBG_DBG, "");

	// if we need credentials
	// for encrypted document, we have to wait until setCredentials is called,
	// which in turn calls this method. All other public methods which could
	// require encrypted content has to check check_need_credentials to ensute that
	// everything is prepared to be used
	if(needsCredentials())
	{
		kernelPrintDbg(debug::DBG_WARN, "Credentials for document are required. Waiting for setCredentials method");
		return;
	}

	// Clean up part:
	// =============
	
	// unregisters all page tree observers before docCatalog invalidation
	// only if docCatalog is already initialized (not in first call from
	// constructor
	unregisterPageObservers();

	// cleans up and invalidates all returned pages
	if(pageList.size())
	{
		kernelPrintDbg(debug::DBG_DBG, "Cleaning up pages list with "<<pageList.size()<<" elements");
		PageList::iterator i;
		for(i=pageList.begin(); i!=pageList.end(); ++i)
		{
			kernelPrintDbg(debug::DBG_DBG, "invalidating page at pos="<<i->first);
			i->second->invalidate();
		}
		pageList.clear();
	}

	// cleans up indirect mapping
	if(indMap.size())
	{
		// checks for held values (smart pointer is not unique, so somebody
		// has to keep shared_ptr to same value)
		for(IndirectMapping::iterator i=indMap.begin(); i!=indMap.end(); ++i)
		{
			IndiRef ref=i->first;
			shared_ptr<IProperty> value=i->second;
			if(!value.unique())
				kernelPrintDbg(debug::DBG_WARN, "Somebody still holds property with with "<<ref);
		}
		kernelPrintDbg(debug::DBG_DBG, "Cleaning up indirect mapping with "<<indMap.size()<<" elements");
		indMap.clear();
	}

	// invalidates pageCount
	pageCount=0;

	if((docCatalog.get()) && (!docCatalog.unique()))
		kernelPrintDbg(debug::DBG_WARN, "Document catalog dictionary is held by somebody.");
	
	kernelPrintDbg(debug::DBG_DBG, "Cleaning up nodeCountCache with "<<nodeCountCache.size()<<" entries");
	utils::clearCache(nodeCountCache);
	
	kernelPrintDbg(debug::DBG_DBG, "Cleaning up pageTreeKidsParentCache with "<<pageTreeKidsParentCache.size()<<" entries");
	utils::clearCache(pageTreeKidsParentCache);

	// cleanup all returned outlines  -------------||----------------- 
	
	// Initialization part:
	// ===================
	
	// Intializes document catalog dictionary.
	// gets Root field, which should contain reference to catalog.
	// If no present or not reference, we have corrupted PDF file and exception
	// is thrown
	kernelPrintDbg(debug::DBG_DBG, "Getting Root field - document catalog");
	IndiRef rootRef(xref->getRootNum(), xref->getRootGen());
	shared_ptr<IProperty> prop_ptr=getIndirectProperty(rootRef);
	if(prop_ptr->getType()!=pDict)
	{
		kernelPrintDbg(debug::DBG_ERR, "Trailer dictionary doesn't point to correct document catalog "
				<<"(type="<<prop_ptr->getType()<<")");
		throw ElementBadTypeException("Root");
	}
	kernelPrintDbg(debug::DBG_DBG, "Document catalog successfully fetched");
	docCatalog=IProperty::getSmartCObjectPtr<CDict>(prop_ptr);
	
	kernelPrintDbg(debug::DBG_DBG, "Registering observers to page tree structure");
	// registers pageTreeRootObserver to document catalog and to Pages property
	// if it is reference
	REGISTER_SHAREDPTR_OBSERVER(docCatalog, pageTreeRootObserver);
	if(docCatalog->containsProperty("Pages"))
	{
		shared_ptr<IProperty> pagesProp=docCatalog->getProperty("Pages");
		if(isRef(pagesProp))
			REGISTER_SHAREDPTR_OBSERVER(pagesProp, pageTreeRootObserver);
		else
			kernelPrintDbg(debug::DBG_WARN, "Pages field is not reference as required");
	}else
		kernelPrintDbg(debug::DBG_WARN, "Document doesn't contain page tree structure");
	
	// registers pageTreeNodeObserver and pageTreeKidsObserver to page tree root
	// dictionary which registers these observers to whole page tree structure
	shared_ptr<IProperty> pageTreeRoot=utils::getPageTreeRoot(_this.lock());
	if(pageTreeRoot.get())
		registerPageTreeObservers(pageTreeRoot);
}

CPdf::CPdf(StreamWriter * stream, OpenMode openMode)
	:pageTreeRootObserver(new PageTreeRootObserver(this)),
	 pageTreeNodeObserver(new PageTreeNodeObserver(this)),
	 pageTreeKidsObserver(new PageTreeKidsObserver(this)),
	 id(NO_PDF_ID),
	 change(false), 
	 modeController(NULL)
{
	// gets xref writer - if error occures, exception is thrown 
	// Note that we can't do anything that could use cobjects here
	// because of weak_ptr & shared_ptr are not initialized yet
	xref=new XRefWriter(stream, this);
	mode=openMode;

	// sets mode accoring openMode
	// ReadOnly and ReadWrite implies xref paranoid mode (default one) 
	// whereas Advanced mode sets easy mode because we want to have full 
	// control over document
	if(mode==Advanced)
		xref->setMode(XRefWriter::easy);

	setPdfId();
	assert(id != NO_PDF_ID);
}

/** Deallocates given storage (with all its elements).
 * @param storage Resolve storage to deallocate.
 */
void deleteResolveRefStorage(ResolvedRefStorage * storage)
{
	for(ResolvedRefStorage::iterator storIter=storage->begin();
			storIter != storage->end(); ++storIter)
	{
		ResolvedRefEntry * entry = storIter->second;
		assert(entry);
		delete entry;
	}
	delete storage;
}

/** Removes entry for given id from mapping.
 * @param originalId Id for cpdf owning map.
 * @param map Resolve mapping.
 * @param id PDF id.
 *
 * Removes mapping entry for given id and deallocates associated
 * resolve storage.
 */
void removeResolveRefMapping(const cpdf_id_t &orignalId, ResolvedRefMapping &map, 
		const cpdf_id_t &id)
{
	ResolvedRefMapping::iterator iter = map.find(id);
	if(iter == map.end())
		return;

	utilsPrintDbg(DBG_DBG, "Releasing ResolvedRefStorage for pdf="
			<< id << " from "<< orignalId);
	ResolvedRefStorage * storage = iter->second;
	deleteResolveRefStorage(storage);
	map.erase(iter);
}

/** Translates pdf to its id.
 * This translation can be reversed by getCPdfFromId method.
 * @return id for pdf instance.
 */
cpdf_id_t getIdFromCPdf(const CPdf * pdf)
{
	return reinterpret_cast<cpdf_id_t>(pdf);
}

/** Translates id to the pdf instance pointer.
 * @param id Pdf id generated by getIdFromCPdf.
 *
 * @return Pointer to CPdf instance associated with given id.
 */
CPdf * getCPdfFromId(const cpdf_id_t &id)
{
	return reinterpret_cast<CPdf *>(id);
}

void CPdf::setPdfId()
{
	// sets id as address of this instance
	id = getIdFromCPdf(this);
	assert(getCPdfFromId(id) == this);
	kernelPrintDbg(DBG_DBG, "pdf "<< this 
			<< " is associated with id=" << id);
	CPdf::allPdfs.push_back(id);
}

void CPdf::releasePdfId()
{
	// removes current id from the list of all life pdfs
	CPdfListContainer::iterator iter = 
		std::find(CPdf::allPdfs.begin(), CPdf::allPdfs.end(), id);
	assert(iter != CPdf::allPdfs.end());
	CPdf::allPdfs.erase(iter);

	// removes id also from all mappings from other alive pdfs
	for(iter=CPdf::allPdfs.begin(); iter != CPdf::allPdfs.end(); ++iter)
	{
		cpdf_id_t pdfId = (*iter);
		CPdf *pdf = getCPdfFromId(pdfId);
		removeResolveRefMapping(pdf->getId(), pdf->resolvedRefMapping, id);
	}
}

void CPdf::invalidate()
{
	kernelPrintDbg(DBG_DBG, "");

	// indirect mapping is cleaned up automaticaly
	
	// discards all returned pages
	for(PageList::iterator i=pageList.begin(); i!=pageList.end(); ++i)
	{
		kernelPrintDbg(DBG_DBG, "Invalidating page at pos="<<i->first);
		i->second->invalidate();
	}
	pageList.clear();

	// idealy we should unregister page tree observers but as the _this
	// is no longer valid in this context (last reference to 
	// shared_ptr<CPdf> was dropped) we can't call unregisterPageObservers 
	// which relies on it (like everything which uses weak_ptr to this 
	// instance) - we will rather mark all types of observers are invalid
	// so they do nothing if they are triggered - which should not happen
	// anyway
	pageTreeRootObserver->setActive(false);
	pageTreeNodeObserver->setActive(false);
	pageTreeKidsObserver->setActive(false);

	// clears all referenced indirect properties
	indMap.clear();

	// clean up resolved reference mapping for different pdf objects
	for(ResolvedRefMapping::iterator i=resolvedRefMapping.begin(); 
			i!=resolvedRefMapping.end(); ++i)
	{
		kernelPrintDbg(DBG_DBG, "Discarding resolved storage (size="
				<<i->second->size()<<") for pdf wih id="
				<<i->first);
		ResolvedRefStorage * storage = i->second;
		assert(storage);
		deleteResolveRefStorage(storage);
	}
	resolvedRefMapping.clear();

	// TODO handle outlines when ready
	
}

CPdf::~CPdf()
{
	kernelPrintDbg(DBG_DBG, "");

	// deallocates XRefWriter
	delete xref;

	releasePdfId();
}

//
// 
// this method can't be const because createObjFromXpdfObj requires 
// CPdf * not const CPdf * given by this
boost::shared_ptr<IProperty> CPdf::getIndirectProperty(const IndiRef &ref)const
{
	using namespace debug;

	check_need_credentials(xref);

	// find the key, if it exists
	IndirectMapping::iterator i = indMap.find(ref);
	if(i!=indMap.end())
	{
		// mapping exists, so returns value
		return i->second;
	}

	kernelPrintDbg(DBG_DBG, "No mapping for "<<ref);

	// mapping doesn't exist yet, so tries to create one
	// fetches object according reference
	shared_ptr< ::Object> obj(XPdfObjectFactory::getInstance(), xpdf::object_deleter());
	assert(xref);
	xref->fetch(ref.num, ref.gen, obj.get());
	
	boost::shared_ptr<IProperty> prop_ptr;

	// creates cobject from value according type - indirect
	// parent is set to object reference (it is its own indirect parent)
	// created object is wrapped to smart pointer and if not pNull also added to
	// the mapping
	if(obj->getType()!=objNull)
	{
		IProperty * prop=utils::createObjFromXpdfObj(_this.lock(), *obj, ref);
		prop_ptr=shared_ptr<IProperty>(prop);
		indMap.insert(IndirectMapping::value_type(ref, prop_ptr));
		kernelPrintDbg(DBG_DBG, "Mapping created for "<<ref);
	}else
	{
		kernelPrintDbg(DBG_DBG, ref<<" not available or points to objNull");
		prop_ptr=shared_ptr<CNull>(CNullFactory::getInstance());
	}

	return prop_ptr;
}


IndiRef CPdf::registerIndirectProperty(const boost::shared_ptr<IProperty> &ip, IndiRef &ref)
{
using namespace debug;
using namespace utils;

	kernelPrintDbg(DBG_DBG, "");

	int state;
	if((state=xref->knowsRef(ref))!=RESERVED_REF)
		kernelPrintDbg(DBG_WARN, "Given reference is not in RESERVED_REF state. State is "<<state);
	
	// gets xpdf Object from given ip (which contain definitive value to
	// be stored), and registers change to XRefWriter (changeObject never 
	// throws in this context because this is first change of object - 
	// so no type check fails). We have to set this pdf temporarily, because
	// _makeXpdfObject function sets xref to created Object from ip->getPdf().
	// Finally restores original pdf value
	shared_ptr<CPdf> original=ip->getPdf().lock();
	ip->setPdf(_this);
	::Object * obj=ip->_makeXpdfObject();
	ip->setPdf(original);
	kernelPrintDbg(DBG_DBG, "Initializating object with type="<<obj->getType()<<" to reserved reference "<<ref);
	assert(xref);
	xref->changeObject(ref.num, ref.gen, obj);

	// xpdf object has to be deallocated
	xpdf::freeXpdfObject(obj);

	// creates return value from xpdf reference structure
	// and returns
	IndiRef reference(ref);
	kernelPrintDbg(DBG_INFO, "New indirect object inserted with reference "<<ref);
	change=true;
	return reference;
}

IndiRef CPdf::addProperty(const boost::shared_ptr<IProperty> &ip, IndiRef &indiRef, 
		ResolvedRefStorage & storage, bool followRefs )
{
	kernelPrintDbg(DBG_DBG, "");
	
	// ip is not from same pdf - may be in different one or stand alone object
	// toSubstitute is deep copy of ip to prevent changes in original data.
	// Also sets same pdf as orignal to cloned to enable dereferencing
	shared_ptr<IProperty> toSubstitute=ip->clone();
	if(hasValidPdf(ip))
	{
		// locks cloned object to prevent making changes (kind of workaround)
		// we need indiref here because of mapping to new referencies and object
		// with valid pdf and indiref calls dispatchChange when something
		// changes
		toSubstitute->lockChange();
		toSubstitute->setPdf(ip->getPdf());
		toSubstitute->setIndiRef(ip->getIndiRef());
	}

	// toSubstitute may contain referencies deeper in hierarchy, so all
	// referencies have to be added or reserved to this pdf too before 
	// toSubstitute can be added itself
	subsReferencies(toSubstitute, storage, followRefs);

	// all possible referencies in toSubstitute are now added to this pdf and so
	// we can add toSubstitute. Reference is in storage mapping
	return registerIndirectProperty(toSubstitute, indiRef);
}

/** Reserves new referenece and creates mapping.
 * @param container Resolved mapping container.
 * @param xref XRefWriter for new reference reservation.
 * @param oldRef Original reference.
 * @param entry Pointer for resolved ref mapping entry.
 *
 * Reserves a new reference with XRefWriter::reserveRef method and if oldRef is
 * valid reference (checks with isRefValid method) also creates mapping [oldRef,
 * newRef to given container with STATE_NEW state and given entry will point
 * to the mapping entry pointer.
 *
 * @return newly reseved reference entry.
 */
IndiRef createMapping(ResolvedRefStorage & container, XRefWriter & xref, 
		IndiRef &oldRef, ResolvedRefEntry ** entry)
{
using namespace debug;

	// this reference is processed for the first time. Reserves new
	// reference for indirect object and stores mapping from original
	// value to container.
	kernelPrintDbg(DBG_DBG, "processing "<<oldRef<<" for the first time");
	IndiRef indiRef(xref.reserveRef());
	
	// creates entry in container to enable reusing old reference to new
	// mapping is created only if oldRef is valid reference, which means that
	// object is indirect and if mapping is not in container yet
	if(isRefValid(&oldRef))
	{
		ResolvedRefStorage::iterator i = container.find(oldRef);
		if(i == container.end())
		{
			*entry = new ResolvedRefEntry(indiRef, STATE_NEW);
			container.insert(ResolvedRefStorage::value_type(oldRef, *entry));
			kernelPrintDbg(DBG_DBG, "Created mapping from "<<oldRef<<" to "<<indiRef);
		}else
			*entry = i->second;
	}else
		*entry = NULL;

	// returns new reference.
	return indiRef;
}

IndiRef CPdf::subsReferencies(const boost::shared_ptr<IProperty> &ip, 
		ResolvedRefStorage & container, bool followRefs)
{
using namespace utils;

	// TODO create constant
	IndiRef invalidRef;
	
	// this method makes sense only for properties from different pdf	
	assert(_this.lock()!=ip->getPdf().lock());
	
	kernelPrintDbg(debug::DBG_DBG,"property type="
			<<ip->getType()<<" ResolvedRefStorage size="
			<<container.size());

	PropertyType type=ip->getType();
	ChildrenStorage childrenStorage;

	switch(type)
	{
		case pRef:
		{
			// checks if this reference has already been considered to prevent
			// endless loops for cyclic structures
			ResolvedRefStorage::iterator i;
			ResolvedRefEntry * refEntry = NULL;
			IndiRef ipRef=getValueFromSimple<CRef>(ip);
			if((i=container.find(ipRef))!=container.end())
			{
				// this reference has already been processed, so reuses
				// reference which already has been created/reserved
				refEntry = i->second;
				kernelPrintDbg(DBG_DBG, ipRef<<" already mapped to "<<refEntry->first);
			}else 
			{
				// No mapping for ipRef
				// reserves new reference and creates mapping
				// refEntry may be null if ipRef is not valid (stand alone
				// object)
				IndiRef ref = createMapping(container, *xref, ipRef, &refEntry);
			}

			// if followRefs is true and target reference is not resolved yet, 
			// adds target property too. Reference  is already registered now
			// and so registerIndirectProperty will use it correctly
			// Following referencies of property with no PDF has no sense
			// so we skipp it too.
			if(followRefs && hasValidPdf(ip) && refEntry->second == STATE_NEW)
			{
				assert(refEntry);
				kernelPrintDbg(DBG_DBG, "Following reference "<<ipRef<<" mapped to "
						<<refEntry->first);	
				// ip is from read pdf and so dereferences target value 					
				// FIXME check for valid pdf
				shared_ptr<CPdf> pdf = ip->getPdf().lock();
				shared_ptr<IProperty> followedIp=pdf->getIndirectProperty(ipRef);

				// adds dereferenced value using addProperty with collected
				// container. Current mapping is set to resolving state to 
				// prevent from endless loops for cyclick referencies
				// returned reference must be same as registered one 
				refEntry->second = STATE_RESOLVING;
				IndiRef addIndiRef=addProperty(followedIp, refEntry->first, container, followedIp);
				assert(addIndiRef==refEntry->first);
				refEntry->second = STATE_RESOLVED;
			}			
			return refEntry->first;
		}	
		// complex types (pArray, pDict and pStream) collects their children to the 
		// container
		case pArray:
			IProperty::getSmartCObjectPtr<CArray>(ip)->_getAllChildObjects(childrenStorage);
			break;
		case pDict:
			IProperty::getSmartCObjectPtr<CDict>(ip)->_getAllChildObjects(childrenStorage);
			break;
		case pStream:
			IProperty::getSmartCObjectPtr<CStream>(ip)->_getAllChildObjects(childrenStorage);
			break;

		// all other simple values are ok, nothing should return
		default:
			return invalidRef;
	}

	// goes throught all collected children and recursively calls this
	// method on each. If return value is non NULL, sets new child value to
	// returned reference.
	ChildrenStorage::iterator i;
	for(i=childrenStorage.begin(); i!=childrenStorage.end(); ++i)
	{
		shared_ptr<IProperty> child=*i;
		if(!isRef(*child) && !isDict(*child) && !isArray(*child) && !isStream(*child))
		{
			// child is none of interesting type which may hold reference
			// inside, so skips such children
			continue;
		}
		
		IndiRef ref=subsReferencies(child, container, followRefs);
		if(isRefValid(&ref))
		{
			// new reference for this child
			boost::shared_ptr<CRef> ref_ptr=IProperty::getSmartCObjectPtr<CRef>(child);
			ref_ptr->lockChange();
			ref_ptr->setValue(ref);
			kernelPrintDbg(debug::DBG_DBG,"Reference changed to " << ref);
			continue;
		}
	}

	// also complex object is same - all referencies in this subtree are added
	// in this moment
	return invalidRef;
}

IndiRef CPdf::addIndirectProperty(const boost::shared_ptr<IProperty> &ip, bool followRefs)
{
using namespace utils;
using namespace debug;
using namespace boost;

	kernelPrintDbg(DBG_DBG, "");

	check_need_credentials(xref);

	if(getMode()==ReadOnly)
	{
		kernelPrintDbg(DBG_ERR, "Document is in read-only mode now");
		throw ReadOnlyDocumentException("Document is in read-only mode.");
	}

	// reference can't be value of indirect property
	if(isRef(*ip))
	{
		kernelPrintDbg(DBG_ERR, "Reference can't be value of indirect property.");
		throw ElementBadTypeException("ip");
	}
	
	// checks whether given ip is from same pdf
	shared_ptr<CPdf> ipPdf = ip->getPdf().lock();
	if(ipPdf.get()==this)
	{
		// ip is from same pdf and so all possible referencies are already in 
		// pdf too. We can clearly register with given indiRef
		kernelPrintDbg(DBG_DBG, "Property from same pdf");
		IndiRef ref = xref->reserveRef();
		return registerIndirectProperty(ip, ref);
	}

	// ip is from different pdf.
	// Get or create mapping for ip's pdf (pdf is identiefied by its id - if 
	// pdf==null - prop from no pdf - then uses NO_PDF_ID constant). 
	// It contains mappings from such pdf indirect reference to coresponding 
	// newly created reference for this pdf.
	cpdf_id_t id=(ipPdf)?ipPdf->getId():CPdf::NO_PDF_ID;
	ResolvedRefMapping::iterator i=resolvedRefMapping.find(id);
	ResolvedRefStorage * resolvedStorage;
	if(i==resolvedRefMapping.end())
	{
		// creates new storage and insert mapping and associates it with ip's
		// pdf (represented by its id and newly created resolvedStorage).
		resolvedStorage=new ResolvedRefStorage();
		resolvedRefMapping.insert(ResolvedRefMapping::value_type(id, resolvedStorage));
		kernelPrintDbg(DBG_DBG, "No resolvedRefMapping entry for "<<id
				<<" pdf. Created new entry");
	}else
		// uses already created storage
		resolvedStorage=i->second;

	// If given ip is indirect and there already is mapping in resolvedStorage,
	// this property or reference to it has already been processed
	ResolvedRefStorage::iterator resIter;
	ResolvedRefEntry * refEntry = NULL;
	IndiRef indiRef;
	if(hasValidRef(ip)&&
	  (resIter=resolvedStorage->find(ip->getIndiRef()))!=resolvedStorage->end())
	{
		refEntry = resIter->second;
		kernelPrintDbg(DBG_DBG, "Property with "<<ip->getIndiRef()
				<<" already in mapping. Mapped to "
				<<refEntry->second);

		// entry state must not be resolving, because we are starting
		// resolution and STATE_RESOLVING would mean that that object
		// couldn't have been completely resolved 
		assert(refEntry->second != STATE_RESOLVING);

		// If property associated with mapped reference is already resolved,
		// there is not much to do
		if(refEntry->second == STATE_RESOLVED)
		{
			kernelPrintDbg(DBG_INFO, "Property with "<<ip->getIndiRef()
					<<" already stored as "<<refEntry->second);
			return refEntry->first;
		}
		indiRef = refEntry->first;
	}else
	{
		// reserves reference for new indirect object and if given ip is indirect
		// object too, creates also resolved mapping for it
		// refEntry will be NULL for ip with invalid reference (with no PDF)
		IndiRef ipRef = ip->getIndiRef();
		indiRef = createMapping(*resolvedStorage, *xref, 
				ipRef, &refEntry);
	}
	assert(isRefValid(&indiRef));

	// everything is checked now and all the work is delegated to recursive
	// addProperty method
	kernelPrintDbg(DBG_DBG, "Adding new indirect object.");
	if(refEntry)
		refEntry->second = STATE_RESOLVING;
	IndiRef addRef = addProperty(ip, indiRef, *resolvedStorage, followRefs);
	assert(addRef == indiRef);
	// sets state to resolved only for follow referencies case because
	// otherwise we have only reserved referencies otherwise falls back
	// to new
	if(refEntry)
		refEntry->second = (followRefs)?STATE_RESOLVED:STATE_NEW;

	kernelPrintDbg(DBG_INFO, "New indirect object added with "
			<<indiRef<<" with type="<<ip->getType());

	return indiRef;
}

void CPdf::changeIndirectProperty(const boost::shared_ptr<IProperty> &prop)
{
	kernelPrintDbg(DBG_DBG, "");
	
	check_need_credentials(xref);

	if(getMode()==ReadOnly)
	{
		kernelPrintDbg(DBG_ERR, "Document is in read-only mode now");
		throw ReadOnlyDocumentException("Document is in read-only mode.");
	}

	shared_ptr<CPdf> _thisP = _this.lock();
	if(utils::isEncrypted(_thisP))
	{
		kernelPrintDbg(DBG_WARN, "Changing properties for encrypted documents si not implemented");
		throw NotImplementedException("changeIndirectProperty");
	}
	
	// checks property at first
	// it must be from same pdf
	if(prop->getPdf().lock() != _thisP)
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
	// in paranoid mode and type check fails - to make it easier for such a case
	// we are using shared_ptr which handles propObject cleanup correctly
	shared_ptr<Object> propObject(prop->_makeXpdfObject(), xpdf::object_deleter());
	kernelPrintDbg(DBG_DBG, "Registering change to the XRefWriter");
	xref->changeObject(indiRef.num, indiRef.gen, propObject.get());

	// checks whether prop is same instance as one in mapping. If so, keeps
	// indirect mapping, because it has just changed some of its direct fields. 
	// Otherwise removes it, because new value is something totaly different. 
	// Mapping will be created in next getIndirectProperty call.
	if(prop==getIndirectProperty(indiRef))
	{
		kernelPrintDbg(DBG_DBG, "Indirect mapping kept for "<<indiRef);
	}
	else
	{
		indMap.erase(indiRef);
		kernelPrintDbg(DBG_INFO, "Indirect mapping removed for "<<indiRef);
	}

	// sets change flag
	change=true;
}

/** Deleter for file based CPdf instance.
 * Used by shared_ptr as destructor. It is initialized from file handle used 
 * for CPdf and it is responsible for proper CPdf deallocation and file handle
 * closing.
 */
class PdfFileDeleter
{
	FILE * file;
public:
	PdfFileDeleter(FILE * f): file(f) {};

	void operator ()(CPdf * pdf)
	{
		assert(pdf);
		pdf->invalidate();
		delete pdf;
		if(fclose(file))
		{
			int err = errno;
			kernelPrintDbg(debug::DBG_ERR, "Unable to close file handle (cause=\""
					<<strerror(err) << "\"");
		}

		kernelPrintDbg(debug::DBG_DBG, "Instance deleted.");
	}
};

boost::shared_ptr<CPdf> CPdf::getInstance(const char * filename, OpenMode mode)
{
using namespace std;

	kernelPrintDbg(debug::DBG_DBG, "");

	if(!pdfedit_core_dev_init_check())
	{
		kernelPrintDbg(debug::DBG_CRIT, "pdfedit-core-dev core is not initialized");
		throw PDFedit_devException();
	}
	
	// openMode is read-only by default
	const char * openMode="rb";
	
	// if mode is ReadWrite or higher, set to read-write mode starting at the 
	// begining.
	if(mode >= ReadWrite)
		openMode="rb+";

	// opens file and creates (xpdf) FileStream
	FILE * file=fopen(filename, openMode);
	if(!file)
	{
		kernelPrintDbg(debug::DBG_ERR, "Unable to open file \""<< 
				filename <<"\" in mode="<< openMode <<
				" (reason="<<strerror(errno)<<")");
		throw PdfOpenException("Unable to open file.");
	}
	kernelPrintDbg(debug::DBG_DBG,"File \"" << filename << "\" open successfully in mode=" << openMode);
	
	// creates FileStream writer to enable changes to the File stream
	Object obj;
	obj.initNull();
	StreamWriter * stream=new FileStreamWriter(file, 0, gFalse, 0, &obj);
	kernelPrintDbg(debug::DBG_DBG,"File stream created");

	// stream is ready, creates CPdf instance
	shared_ptr<CPdf> instance;
	try
	{
		instance = shared_ptr<CPdf>(new CPdf(stream, mode), PdfFileDeleter(file));
		instance->_this = instance;

		// initializes revision specific data for the newest revision
		// We can't do it in constructor because we are using cobjects
		// there and thus shared_ptr and _this have to be initialized
		instance->initRevisionSpecific();

		// We don't want to enable editing linearized documents because
		// it leads to almost 100% damage of content - we are not able
		// to store changes to such a document. So it is simpler to
		// prevent from changes at all.
		if(instance->isLinearized())
			instance->mode = ReadOnly;
		kernelPrintDbg(debug::DBG_INFO, "Instance created successfully file="
				<<filename<<" openMode=" << openMode);
		return instance;
	}catch(std::exception &e)
	{
		// If we have failed in constructor then we have to close file handle
		// manualy as the PdfFileDeleter didn't get chance to do it
		if(!instance) 
		{
			fclose(file);
		}
		kernelPrintDbg(DBG_CRIT, "Pdf instance creation failed. filename="
				<<filename<<" cause="<<e.what());
		string what=string("CPdf open failed. reason=")+e.what();
		throw PdfOpenException(what);
	}
}

// helper macor for position in range testing
// pos must be without sideeffects
#define POSITION_IN_RANGE(pos) (((pos) >= 1) && (pos)<=getPageCount())

boost::shared_ptr<CPage> CPdf::getPage(size_t pos)const
{
using namespace utils;

	kernelPrintDbg(DBG_DBG, "");

	check_need_credentials(xref);

	if(!POSITION_IN_RANGE(pos))
	{
		kernelPrintDbg(DBG_WARN, "Page out of range pos="<<pos);
		throw PageNotFoundException(pos);
	}

	// checks if page is available in pageList
	PageList::const_iterator i;
	if((i=pageList.find(pos))!=pageList.end())
	{
		kernelPrintDbg(DBG_DBG, "Page at pos="<<pos<<" found in pageList");
		return i->second;
	}

	// page is not available in pageList, searching has to be done
	// find throws an exception if any problem found, otherwise pageDict_ptr
	// contians Page dictionary at specified position.
	shared_ptr<CDict> rootPages_ptr=getPageTreeRoot(_this.lock());
	if(!rootPages_ptr.get())
		throw PageNotFoundException(pos);
	shared_ptr<CDict> pageDict_ptr=findPageDict(_this.lock(), rootPages_ptr, 1, pos, &nodeCountCache);

	// creates CPage instance from page dictionary and stores it to the pageList
	CPage * page=CPageFactory::getInstance(pageDict_ptr);
	shared_ptr<CPage> page_ptr(page);
	pageList.insert(PageList::value_type(pos, page_ptr));
	kernelPrintDbg(DBG_DBG, "New page added to the pageList size="<<pageList.size());

	return page_ptr;
}

unsigned int CPdf::getPageCount()const
{
using namespace utils;
	
	kernelPrintDbg(DBG_DBG, "");
	
	check_need_credentials(xref);

	// try to use cached value - if zero, we have to get it from Page tree root
	if(pageCount)
	{
		kernelPrintDbg(DBG_DBG, "cached page count="<<pageCount);
		return pageCount;
	}
	
	shared_ptr<CDict> rootDict=getPageTreeRoot(_this.lock());
	if(!rootDict.get())
		return 0;
	pageCount=getKidsCount(rootDict, &nodeCountCache);
	kernelPrintDbg(DBG_DBG, "page count="<<pageCount);
	return pageCount;
}

bool CPdf::hasNextPage(const boost::shared_ptr<CPage> &page) const
{
	kernelPrintDbg(DBG_DBG, "");

	// check for the credentials is done in getPagePosition
	size_t pos=getPagePosition(page);
	kernelPrintDbg(DBG_DBG, "Page position is "<<pos);
	++pos;
	return POSITION_IN_RANGE(pos);
}

boost::shared_ptr<CPage> CPdf::getNextPage(const boost::shared_ptr<CPage> &page)const
{
	kernelPrintDbg(DBG_DBG, "");

	size_t pos=getPagePosition(page);
	kernelPrintDbg(DBG_DBG, "Page position is "<<pos);
	++pos;
	
	// page in range, uses getPage
	return getPage(pos);

}

bool CPdf::hasPrevPage(const boost::shared_ptr<CPage> &page)const
{
	kernelPrintDbg(DBG_DBG, "");

	size_t pos=getPagePosition(page);
	kernelPrintDbg(DBG_DBG, "Page position is "<<pos);
	--pos;
	return POSITION_IN_RANGE(pos);
}

boost::shared_ptr<CPage> CPdf::getPrevPage(const boost::shared_ptr<CPage> &page)const
{
	kernelPrintDbg(DBG_DBG, "");

	// check for the credentials is done in getPagePosition
	size_t pos=getPagePosition(page);
	kernelPrintDbg(DBG_DBG, "Page position is "<<pos);
	pos--;
	
	// page in range, uses getPage
	return getPage(pos);
}

size_t CPdf::getPagePosition(const boost::shared_ptr<CPage> &page)const
{
	kernelPrintDbg(DBG_DBG, "");
		
	check_need_credentials(xref);

	// search in returned page list
	PageList::iterator i;
	for(i=pageList.begin(); i!=pageList.end(); ++i)
	{
		// compares page instances
		// This is ok even if they manage same page dictionary
		if(i->second == page)
		{
			kernelPrintDbg(DBG_DBG, "Page found at pos="<<i->first);
			return i->first;
		}
	}

	// page not found, it hasn't been returned by this pdf
	throw PageNotFoundException();
}


void CPdf::consolidatePageList(const shared_ptr<IProperty> & oldValue, const shared_ptr<IProperty> & newValue)
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
	if(!isNull(oldValue))
	{
		// oldValue is reference
		
		PageTreeNodeType oldNodeType=getNodeType(oldValue);

		switch(oldNodeType)
		{
			// simple page is compared with all from pageList and if found, 
			// removes it from list and invalidates it.
			// Difference is set to - 1, because one page is removed 
			case LeafNode:
			{
				kernelPrintDbg(DBG_DBG, "oldValue was simple page dictionary");
				difference = -1;
				shared_ptr<CDict> oldDict_ptr=getCObjectFromRef<CDict>(oldValue);

				for(PageList::iterator i=pageList.begin(); i!=pageList.end(); ++i)
				{
					// checks page's dictionary with old one
					shared_ptr<CPage> page=i->second;
					if(page->getDictionary() == oldDict_ptr)
					{
						i->second->invalidate();
						size_t pos=i->first;
						minPos=pos;
						pageList.erase(i);
						kernelPrintDbg(DBG_DBG, "CPage(pos="
								<<pos
								<<") associated with oldValue page dictionary removed. pageList.size="
								<<pageList.size());
						break;
					}
				}
				break;
			}
			case InterNode:
			case RootNode:
			{
				// all CPages from this sub tree are removed and invalidated
				// difference is set to -getKidsCount value (total page lost)
				kernelPrintDbg(DBG_DBG, "oldValue was intermediate node dictionary.");
				difference = -getKidsCount(oldValue, &nodeCountCache);

				// gets reference of oldValue - which is the root of removed
				// subtree
				IndiRef ref=getValueFromSimple<CRef>(oldValue);
				
				bool found=false;
				for(PageList::iterator i=pageList.begin(); i!=pageList.end();)
				{
					shared_ptr<CPage> page=i->second;
					// checks page's dictionary whether it is in oldDict_ptr sub
					// tree and if so removes it from pageList
					if(isNodeDescendant(_this.lock(), ref, page->getDictionary()))
					{
						// sets flag, that at least one descendants is found
						found=true;
						
						// updates minPos with page position (if greater)
						size_t pos=i->first;
						if(pos > minPos)
							minPos=pos;
						
						page->invalidate();
						pageList.erase(i++);
						kernelPrintDbg(DBG_DBG, "CPage(pos="
								<<pos
								<<") associated with oldValue page dictionary removed. pageList.size="
								<<pageList.size());
						continue;
					}
					// if this element is not in subtree and found is true,
					// this is first node which is in different node and so
					// none of following pages can be descendant (PageList
					// is sorted) 
					if(found)
						break;
					// note: we can't do this in for iteration section 
					// because of erase above which invalidas iterator
					++i;
				}
				break;
			}
			default:
				kernelPrintDbg(DBG_DBG, "oldValue is not leaf or intermediate node.");
		}
	}

	// oldValue subtree (if any) is consolidated now
	kernelPrintDbg(DBG_DBG, "All page dictionaries from oldValue subtree removed. count="<<-difference);

	// number of added pages by newValue tree
	int pagesCount=0;
	
	// handles new value - one after change
	// if pNull - no new value is available (subtree has been removed)
	kernelPrintDbg(DBG_DBG, "newValue type="<<newValue->getType());
	if(!isNull(newValue))
	{
		// newValue is reference
		PageTreeNodeType newValueType=getNodeType(newValue);

		// gets count of pages in newValue node (if intermediate all pages
		// under this node)
		switch(newValueType)
		{
			case LeafNode:
				pagesCount=1;
				break;
			case InterNode:
			case RootNode:
				pagesCount=getKidsCount(newValue, &nodeCountCache);
				break;
			default:
				kernelPrintDbg(DBG_DBG, "newValue is not leaf or intermediate node.");
		}
			
		// try to get position of newValue node.  No pages from this subtree can
		// be in the pageList, so we can set minPos to its position.
		// If getNodePosition throws, then this node is ambiguous and so we have
		// no information
		try
		{
			minPos = getNodePosition(_this.lock(), newValue, &nodeCountCache);
		}catch(std::exception &e)
		{
			// position can't be determined
			// no special handling is needed, minPos keeps its value
			kernelPrintDbg(DBG_WARN, "Couldn't get newValue position. reason="<<e.what());
		}

		kernelPrintDbg(DBG_DBG, "newValue sub tree has "<<pagesCount<<" page dictionaries");
	}

	// corrects difference with added pages
	difference += pagesCount;

	// no difference means no speacial handling for other pages
	// we have replaced old sub tree with new subtree with same number of pages
	if(difference==0)
		return;

	kernelPrintDbg(DBG_INFO, "pageList consolidation from minPos="<<minPos<<" with difference="<<difference);
	 
	// all pages with position greater than minPos, has to be consolidated
	PageList::iterator i;
	PageList readdContainer;
	for(i=pageList.begin(); i!=pageList.end();)
	{
		size_t pos=i->first;
		shared_ptr<CPage> page=i->second;

		if(pos>=minPos)
		{
			// collects all removed
			readdContainer.insert(PageList::value_type(pos, page));	
			pageList.erase(i++);
		}else
			++i;
	}
	
	// checks minPos==0 and if so, we have to handle situation special way,
	// because don't have any information about previous position of oldValue
	// subtree. In such case it has to get current position for all pages in
	// pageList
	if(!minPos)
	{
		kernelPrintDbg(DBG_DBG,"Reassingning all pages posititions.");
		for(i=readdContainer.begin(); i!=readdContainer.end(); ++i)
		{
			// uses getNodePosition for each page's dictionary to find out
			// current position. If getNodePosition throws an exception, it
			// means that it can't be determined. Such page is invalidated.
			try
			{
				size_t pos=getNodePosition(_this.lock(), i->second->getDictionary(), &nodeCountCache);
				kernelPrintDbg(DBG_DBG, "Original position="<<i->first<<" new="<<pos);
				pageList.insert(PageList::value_type(pos, i->second));	
			}catch(AmbiguousPageTreeException & e)
			{
				kernelPrintDbg(DBG_WARN, "page with original position="<<i->first<<" is ambiguous. Invalidating.");
				// page position is ambiguous and so it has to be invalidate
				i->second->invalidate();
			}catch(std::exception & e)
			{
				kernelPrintDbg(DBG_CRIT, "Unexpected error. cause="<<e.what()<<" Possibly BUG");
				assert(!"Possibly bug.");
			}
		}
		return;
	}
	
	kernelPrintDbg(DBG_DBG, "Moving pages position with difference="<<difference<<" from page pos="<<minPos);
	// Information about page numbers which should be consolidated is available
	// so just adds difference for each in readdContainer
	// readds all removed with changed position (according difference)
	for(i=readdContainer.begin(); i!=readdContainer.end(); ++i)
	{
		kernelPrintDbg(DBG_DBG, "Original position="<<i->first<<" new="<<i->first+difference);
		pageList.insert(PageList::value_type(i->first+difference, i->second));	
	}
	kernelPrintDbg(DBG_DBG, "pageList consolidation done.");
}


bool CPdf::consolidatePageTree(const boost::shared_ptr<CDict> & interNode, bool propagate)
{
using namespace utils;

	kernelPrintDbg(DBG_DBG, "");

	// gets pdf of the node - must be non null
	assert(interNode->getPdf().lock());
	
	// only internode make sense to consolidate
	PageTreeNodeType nodeType=getNodeType(interNode);
	if(nodeType<InterNode)
	{
		kernelPrintDbg(DBG_DBG, "given node is not intermediate (type="<<nodeType<<"). Ignoring consolidation");
		return true;
	}
		
	IndiRef interNodeRef=interNode->getIndiRef();
	kernelPrintDbg(DBG_DBG, "intermediate node "<<interNodeRef<<" consolidation");
	kernelPrintDbg(DBG_DBG, "consolidating Count field");

	// gets current count of kids and compares it to Count property
	// if values are different, sets new value and sets countChanged to true and
	// also node's parent should be consolidated
	// Doesn't use cache to be sure that value is really collected
	size_t count=getKidsCount(interNode, NULL);
	bool countChanged=false;
	if(interNode->containsProperty("Count"))
	{
		shared_ptr<IProperty> countProp=interNode->getProperty("Count");
		shared_ptr<CInt> countInt;
		if(isRef(countProp))
		{
			try
			{
				countInt=getCObjectFromRef<CInt>(countProp);
			}catch(CObjectException & e)
			{
				// not int, keeps countInt unintialized
			}
		}else
			if(isInt(countProp))
				countInt=IProperty::getSmartCObjectPtr<CInt>(countProp);
		// if countInt is unitialized, Count property has bad type
		if(!countInt.get())
		{
			// removes old with bad type (or bad target type)
			interNode->delProperty("Count");

			// adds new Count property with correct value
			countInt=shared_ptr<CInt>(CIntFactory::getInstance((int)count));
			kernelPrintDbg(DBG_DBG, "replacing old Count property with new property value="<<count);
			interNode->addProperty("Count", *countInt);
			countChanged=true;
		}else
		{
			// checks value
			size_t currCount=getValueFromSimple<CInt>(countInt);
			if(currCount!=count)
			{
				kernelPrintDbg(DBG_DBG, "Count value is changed from "<<currCount<<" to "<<count);
				countInt->setValue(count);
				countChanged=true;
			}
		}
	}else
	{
		// adds new Count property with correct value
		scoped_ptr<IProperty> countInt(CIntFactory::getInstance((int)count));
		kernelPrintDbg(DBG_DBG, "adding new Count property value="<<count);
		interNode->addProperty("Count", *countInt);
		countChanged=true;
	}

	// if page count has changed, discards cache for this node but keeps values
	// in subtree (they contain correct values because change was just in this
	// intermediate node)
	if(countChanged)
		discardKidsCountCache(interNodeRef, _this.lock(), nodeCountCache, false);
	
	kernelPrintDbg(DBG_DBG, "consolidating Kids array members");

	// collects all kids from internode for consolidation
	ChildrenStorage kids;
	getKidsFromInterNode(interNode, kids);
	ChildrenStorage::iterator i;
	size_t index=0;
	for(i=kids.begin(); i!=kids.end(); ++i, ++index)
	{
		shared_ptr<IProperty> child=*i;
		if(!isRef(child))
		{
			// element is not reference, so we print warning and skip it
			// We are in Observer context so CAN'T remove element
			kernelPrintDbg(DBG_WARN, "Kids["<<index<<"] element must be reference. type="<<child->getType());
			continue;
		}

		// ships all unknown and error nodes (everything below LeafNode)
		PageTreeNodeType childType=getNodeType(child);
		if(childType<LeafNode)
		{
			kernelPrintDbg(DBG_WARN, "Kids["<<index<<"] target is not valid leaf or intermediate node. type="<<childType);
			continue;
		}
		
		// gets target dictionary to check and consolidate - this doesn't throw
		// because it is leaf or intermediate node
		shared_ptr<CDict> childDict=getCObjectFromRef<CDict>(child);
		
		// each leaf and inter node has to have Parent property with refernce to
		// this node (which is indirect object and so we can use its
		// NOTE that change in Parent property doesn't require also interNode
		// parent consolidation
		shared_ptr<CRef> parentRef;
		if(childDict->containsProperty("Parent"))
		{
			shared_ptr<IProperty> parentProp=childDict->getProperty("Parent");
			if(isRef(parentProp))
				parentRef=IProperty::getSmartCObjectPtr<CRef>(parentProp);

			// if parentRef is unitialized, Parent has bad type
			if(!parentRef.get())
			{
				// removes old with bad type (or bad target type)
				childDict->delProperty("Parent");

				// adds new Parent property with correct value
				parentRef=shared_ptr<CRef>(CRefFactory::getInstance(interNodeRef));
				kernelPrintDbg(DBG_DBG, "replacing old Parent property with new");
				childDict->addProperty("Parent", *parentRef);
			}else
			{
				// checks value
				IndiRef currParentRef=getValueFromSimple<CRef>(parentRef);
				if(!(currParentRef==interNodeRef))
				{
					kernelPrintDbg(DBG_DBG, "Parent value is changed from "<<currParentRef<<" to "<<interNodeRef);
					parentRef->setValue(interNodeRef);
				}
			}
		}else
		{
			// adds new Count property with correct value
			scoped_ptr<IProperty> countInt(CRefFactory::getInstance(interNodeRef));
			kernelPrintDbg(DBG_DBG, "adding new Parent property");
			childDict->addProperty("Parent", *countInt);
		}
	}

	// If Count property has changed and propagate flag is set, then also parent
	// intermediate node is consolidated
	if(countChanged && propagate)
	{
		if(nodeType==RootNode)
		{
			// root node has no Parent and so recursion is finished, returns
			// true if Count field has kept its value
			return !countChanged;
		}

		// gets parent node and consolidate it with true propagate flag
		// Parent property has to be reference with dictionary target (indirect
		// object). If there is problem, we can't reconstruct correct value, so
		// just prints warning messages and stops recursion
		if(interNode->containsProperty("Parent"))
		{
			shared_ptr<IProperty> parentProp=interNode->getProperty("Parent");
			if(isRef(parentProp))
			{
				try
				{
					shared_ptr<CDict> parentDict=getCObjectFromRef<CDict>(parentProp);
					return consolidatePageTree(parentDict, true);
				}catch(CObjectException & e)
				{
					kernelPrintDbg(DBG_WARN, "InterNode "<<interNodeRef<<" has bad Parent ref. Target is not a dictionary.");
				}
			}else 
				kernelPrintDbg(DBG_WARN, "InterNode "<<interNodeRef<<" has bad typed Parent field. type="<<parentProp->getType());
		}else
			kernelPrintDbg(DBG_WARN, "InterNode "<<interNodeRef<<" has no Parent field (and it is not root).");
	}

	// returns true if count hasn't changed
	return !countChanged;
}

boost::shared_ptr<CPage> CPdf::insertPage(const boost::shared_ptr<CPage> &page, size_t pos)
{
using namespace utils;

	kernelPrintDbg(DBG_DBG, "pos="<<pos);

	check_need_credentials(xref);

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
	// by default it is root of page tree
	interNode_ptr=getPageTreeRoot(_this.lock());
	if(!interNode_ptr.get())
	{
		// Root of page dictionary doesn't exist
		throw NoPageRootException();
	}
	if(count)
	{
		// stores new page at position of existing page
		
		// searches for page at storePosition and gets its reference
		// page dictionary has to be an indirect object, so getIndiRef returns
		// dictionary reference
		shared_ptr<CDict> currentPage_ptr=findPageDict(_this.lock(), interNode_ptr, 1, storePostion, &nodeCountCache);
		currRef=shared_ptr<CRef>(CRefFactory::getInstance(currentPage_ptr->getIndiRef()));
		
		// gets parent of found dictionary which maintains 
		shared_ptr<IProperty> parentRef_ptr=currentPage_ptr->getProperty("Parent");
		interNode_ptr=getCObjectFromRef<CDict>(parentRef_ptr);
	}

	// gets Kids array where to insert new page dictionary
	shared_ptr<IProperty> kidsProp_ptr=interNode_ptr->getProperty("Kids");
	if(kidsProp_ptr->getType()!=pArray)
	{
		kernelPrintDbg(DBG_ERR, "Pages Kids field is not an array type="<<kidsProp_ptr->getType());
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
			kernelPrintDbg(DBG_ERR, "Page can't be created, because page tree is ambiguous for node at pos="
					<<storePostion<<" candidate positions="<<positions.size());
			throw AmbiguousPageTreeException();
		}
		kidsIndex=positions[0]+append;
	}

	// Now it is safe to add indirect object, because there is nothing that can
	// fail
	shared_ptr<CDict> pageDict=page->getDictionary();
	shared_ptr<CPdf> pageDictPdf = pageDict->getPdf().lock();
	if(pageDictPdf && pageDictPdf !=_this.lock())
	{
		// page comes from different valid pdf - we have to create clone and
		// remove Parent field from it. Also inheritable properties have to be
		// handled
		IndiRef pageDictIndiRef=pageDict->getIndiRef();
		pageDict=IProperty::getSmartCObjectPtr<CDict>(pageDict->clone());
		pageDict->delProperty("Parent");

		// clone needs to set pdf and indirect, because these values are not
		// cloned and they are needed for indirect properties dereferencing
		// (pdf) and for internal referencies (some of pageDict members may
		// refer to page). This implies that pageDict has to be locked for
		// dispatchChange.
		pageDict->lockChange();
		pageDict->setPdf(pageDictPdf);
		pageDict->setIndiRef(pageDictIndiRef);
		CPageAttributes::setInheritable(pageDict);
	}

	// Adds pageDict as new indirect property (also with properties referenced 
	// by this dictionary) if it comes from different pdf. Otherwise simply
	// add reference.
	IndiRef pageRef;
	if(pageDict->getPdf().lock() == _this.lock())
	{	
		try
		{
			size_t pos = getPagePosition(page);
			kernelPrintDbg(debug::DBG_ERR, "Page " 
					<< pageDict->getIndiRef() 
					<< " is already in the pdf at "<<
					pos << " position");
			throw AmbiguousPageTreeException();
		}catch (PageNotFoundException &)
		{
			pageRef=pageDict->getIndiRef();
		}
	}
	else
		pageRef=addIndirectProperty(pageDict, true);

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
	pageList.insert(PageList::value_type(storePostion+append, newPage_ptr));
	kernelPrintDbg(DBG_DBG, "New page added to the pageList size="<<pageList.size());
	return newPage_ptr;
}

void CPdf::removePage(size_t pos)
{
using namespace utils;

	kernelPrintDbg(DBG_DBG, "");

	check_need_credentials(xref);

	if(getMode()==ReadOnly)
	{
		kernelPrintDbg(DBG_ERR, "Document is in read-only mode now");
		throw ReadOnlyDocumentException("Document is in read-only mode.");
	}

	// checks position
	if(!POSITION_IN_RANGE(pos))
		throw PageNotFoundException(pos);

	// Searches for page dictionary at given pos and gets its reference.
	// getPageTreeRoot doesn't fail, because we are in page range and so it has
	// to exist
	shared_ptr<CDict> rootDict=getPageTreeRoot(_this.lock());
	shared_ptr<CDict> currentPage_ptr=findPageDict(_this.lock(), rootDict, 1, pos, &nodeCountCache);
	shared_ptr<CRef> currRef(CRefFactory::getInstance(currentPage_ptr->getIndiRef()));
	
	// Gets parent field from found page dictionary and gets its Kids array
	shared_ptr<IProperty> parentRef_ptr=currentPage_ptr->getProperty("Parent");
	shared_ptr<CDict> interNode_ptr=getCObjectFromRef<CDict>(parentRef_ptr);
	shared_ptr<IProperty> kidsProp_ptr=interNode_ptr->getProperty("Kids");
	if(kidsProp_ptr->getType()!=pArray)
	{
		kernelPrintDbg(DBG_ERR, "Pages Kids field is not an array type="<<kidsProp_ptr->getType());
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
		kernelPrintDbg(DBG_ERR, "Page can't be created, because page tree is ambiguous for node at pos="
				<<pos<<" candidate positions="<<positions.size());
		throw AmbiguousPageTreeException();
	}
	
	// removing triggers pageTreeWatchDog consolidation
	size_t kidsIndex=positions[0];
	kids_ptr->delProperty(kidsIndex);
	
	// page dictionary is removed from the tree, consolidation is done also for
	// pageList at this moment
}

void CPdf::save(bool newRevision)const
{
	kernelPrintDbg(DBG_DBG, "");

	// Saving linearized document results in damaged documents
	if(isLinearized())
		throw NotImplementedException("Linearized PDF save is not supported");

	if(getMode()==ReadOnly)
	{
		kernelPrintDbg(DBG_ERR, "Document is in read-only mode now");
		throw ReadOnlyDocumentException("Document is in read-only mode.");
	}
	
	// checks actual revision
	if(!utils::isLatestRevision(*xref))
	{
		kernelPrintDbg(DBG_ERR, "Document is not in latest revision");
		throw ReadOnlyDocumentException("Document is in read-only mode.");
	}
	
	// we are in the newest revision, so changes can be saved
	// delegates all work to the XRefWriter and set change to 
	// mark, that no changes were stored
	// check for credentials is done in XRefWriter
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
	
	// Saving linearized document results in demaged documents
	if(isLinearized())
		throw NotImplementedException("Linearized PDF cloning is not supported");

	// delagates to XRefWriter - check for credentials is done there
	xref->cloneRevision(file);
}

boost::shared_ptr<const CDict> CPdf::getTrailer()const
{
	CDict *trailer = CDictFactory::getInstance(*xref->getTrailerDict());
	trailer->setPdf(_this);

	return boost::shared_ptr<const CDict>(trailer);
}

void CPdf::changeTrailer(std::string &name, const boost::shared_ptr<IProperty> &value)
{
	Object *result;
	boost::shared_ptr<Object> o(value->_makeXpdfObject());
	result = xref->changeTrailer(name.c_str(), o.get());
	if (result) {
		xpdf::object_deleter d;
		d(result);
	}
}


void CPdf::changeRevision(revision_t revisionNum)
{
	kernelPrintDbg(DBG_DBG, "");

	// credentials are checked in XRefWriter
	xref->changeRevision(revisionNum);
	
	// prepares internal structures for new revision
	initRevisionSpecific();
}

void CPdf::canChange () const
{
	check_need_credentials(xref);

	if (xref->isLinearized())
		throw ReadOnlyDocumentException("Document is linearized");

	// Not in lates revision
	if (!utils::isLatestRevision(*xref))
		throw ReadOnlyDocumentException("Document is not in latest revision.");
	// In read only mode
	if (ReadOnly == getMode())
		throw ReadOnlyDocumentException("Document is in Read-only mode.");
}

bool CPdf::needsCredentials()const
{
	return xref->getNeedCredentials();
}

void CPdf::setCredentials(const char * ownerPasswd, const char * userPasswd)
{
	kernelPrintDbg(debug::DBG_INFO, "Setting credentions");
	xref->setCredentials(ownerPasswd, userPasswd);
	initRevisionSpecific();
}


} // end of pdfobjects namespace
