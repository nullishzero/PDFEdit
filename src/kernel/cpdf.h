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

#ifndef __CPDF_H__
#define __CPDF_H__

#include "kernel/static.h"
#include "kernel/cobjecthelpers.h"
#include "kernel/xrefwriter.h"
#include "kernel/modecontroller.h"
#include "kernel/iproperty.h"
#include "kernel/cstream.h"

class StreamWriter;

// =============================================================================
namespace pdfobjects {

class IProperty;
class CDict;
class CXref;
class CPage;
template<typename IP> inline boost::shared_ptr<CDict> getCDictFromDict (IP& ip, const std::string& key);

namespace utils {

template<typename Container> void getAllChildrenOfPdfObject (boost::shared_ptr<CDict> topdict, Container& cont);

class IPdfWriter;

/**
 * Indirect referencies comparator.
 *
 * Handles comparing of Indirect referencies.
 */
class IndComparator
{
public:
	/** Ordering functional operator.
	 * @param one Indirect reference.
	 * @param two Indirect reference.
	 *
	 * Strict weak ordering comparision of referencies.
	 * <br>
	 * Referencies are compared by their fields. First num is compared (it is
	 * more significant) and if nums are same, gen number is compared.
	 *
	 * @return true if one &lt; two or false otherwise.
	 */
	bool operator() (const pdfobjects::IndiRef one, const pdfobjects::IndiRef two) const
	{
		if (one.num == two.num)
			return (one.gen < two.gen);
		else
			return (one.num < two.num);
	}
};

} // namespace utils

	
/** Type for page tree node count chache.
 * It is mapping where key is indirect reference of page tree node and
 * associated value is current number of direct pages under this node.
 */
typedef std::map<IndiRef, size_t, utils::IndComparator> PageTreeNodeCountCache;

/** Type for page tree kids array to parrent mapping.
 * @see CPdf::pageTreeKidsParentCache
 */
typedef std::map<IndiRef, IndiRef, utils::IndComparator> PageTreeKidsParentCache;

/** State of reference translation.
 * <ul>
 * <li><b>STATE_NEW</b> represents a new mapping. This means that a new
 * reference was for translation and it is not backed up by any real 
 * object.
 * <li><b>STATE_RESOLVING</b> represents mapping which is currently 
 * in translation meaning that the real object which will be backed by
 * translated reference is in progress of construction.
 * <li><b>STATE_RESOLVED</b> represents mapping which is finally backed
 * by real indirect object.
 * </ul>
 */
enum ResolveRefState {STATE_NEW, STATE_RESOLVING, STATE_RESOLVED};

/** Entry for reference translation mapping.
 * Firts value stands for the new reference (translated one) and the second
 * keeps state of the translation. 
 */
typedef std::pair<IndiRef, enum ResolveRefState> ResolvedRefEntry;

/** Type for reference translation mappings.
 * Key stands for reference in original property and associated value stands 
 * for reserved reference (with its current resolvetion state) which stands
 * for resolved reference.
 * <br>
 * Mapping is used for adding indirect properties from different pdf. Referncies
 * are bound to (unique in) specific document. If we want to insert subtree of
 * indirect properties from one document to another we have to translate all 
 * referenced indirect objects which all needs translation. Therefore a new
 * reference is created for each and this mapping holds translation from 
 * the original to the created one. 
 *
 * @see CPdf::addIndirectProperty
 */
typedef std::map<IndiRef, ResolvedRefEntry*, utils::IndComparator > ResolvedRefStorage;

/**
 * Indirect properties mapping type.
 */
typedef std::map<const IndiRef, boost::shared_ptr<IProperty>, utils::IndComparator> IndirectMapping;

/** Type for pdf identificator.
 */
typedef uintptr_t cpdf_id_t;

/** Type for mapping from pdfs to their resolved storage.
 * Maps pdf identificators to their resolved reference storage.
 */
typedef std::map<cpdf_id_t, ResolvedRefStorage *> ResolvedRefMapping;

/** CPdf special object.
 *
 * This class is responsible for pdf document maintainance. It provides wrapper
 * on document catalog dictionary with advanced logic concerning revision
 * handling, high level object creation and their synchronization with actual
 * properties state and mode depending operations. Provides interface for making
 * changes to document. In a result it provides fascade for all oparations which
 * have document scope.
 *
 * <p>
 * <b>Instancing</b><br>
 * Public constructor is not available and instances can be created on by 
 * static factory getInstance method. Also no public destructor is available 
 * because instance returned by getInstance methods is wrapped by shared_ptr
 * smart pointer which keeps instance alive until reference to the pointer
 * exists (this is based on reference counting scheme - see smart_ptr 
 * documentation for more information).
 * 
 * <p>
 * <b>Open mode</b><br>
 * Each document may be open in several modes. Each open mode brings specific
 * handling of document manipulation. Open mode for document is set only in 
 * constructor and can't be changed during CPdf instance life cycle. 
 * @see CPdf::openMode
 *
 * <p>
 * <b>Document changes</b><br>
 * CPdf instance contains XRefWriter typed field which maintains all changes to
 * document content. It is not accessible outside from the class to keep full
 * control over instance in CPdf. To enable using also xpdf code outside CPdf
 * CXref casted supertype of XRefWriter is returned in getCXref. This instance
 * doesn't enable any changes but enables access to most accurate indirect
 * objects. Changes to XRefWriter can be done only by CPdf methods. Note that
 * changes can be be done only in newest revision (@see save).
 * 
 * <p>
 * <b>Document properties</b><br>
 * Properties from document can be accessible from document catalog which is
 * returned by getDictionary method. New indirect property, which may be used in
 * some other property (by its reference) can be created by addIndirectProperty.
 * Change of indirect property can be registered to the XRefWriter by 
 * changeIndirectProperty method.
 * Finaly each indirect property is accessible by getIndirectProperty. All these
 * methods are just some wrappers to XRefWriter internal field with CObject to
 * xpdf Object conversion logic. Using them guaranties that all changes are
 * synchronized correctly.
 *
 * <p>
 * <b>Pages maintainance</b><br>
 * One of CPdf responsiblities is to keep CPage instances synchronized with
 * current state of page tree. Page instances (CPage typed) can be obtained by
 * getPage, getFirstPage, getLastPage, getNextPage, getPrevPage methods. All
 * returned instances are kept in pageList to guarantee that request for page at
 * same position returns same page instance (unless page tree is changed).
 * CPdf uses several observers to keep this synchronization. Observer classes
 * are inner to this class to have good access to protected and private fields.
 * Each observer is specialized for one type of change in page tree:
 * <ul>
 * <li>PageTreeRootObserver - synchronizes changes which affects page tree root.
 * <li>PageTreeNodeObserver - synchronizes changes which affects Kids array
 * property
 * <li>PageTreeKidsObserver - synchronizes changes which affects Kids array
 * content and elements
 * </ul>
 * <br>
 * insertPage and removePage enables inserting and removing new pages to the
 * page tree. This way is prefered for making such changes. Other way (as
 * mentioned above) is to change page tree directly using property interface.
 * This way may lead to errors which are not recoverable (may destroy valid 
 * pdf page tree format) and so it is strongly discouraged unless you exactly
 * know what you are doing and CPdf interface doesn't provide right way how 
 * to do it.
 * <br>
 * Pages are counted from 1 (first page) up to getPageCount return value. Note
 * that this may not represent values used for inner page counting writen on the
 * page. CPdf doesn't handle any kind of special document numbering.
 * <p>
 * <b>Revision manipulation</b><br>
 * CPdf provides interface also for document revision handling done in XRefWriter
 * class. Actual revision number (the newest revision has the highest number and 
 * gets smaller towards the older revisions with the 0 used for the oldest one) 
 * can be obtained by getActualRevision method. Current revision can be changed 
 * by changeRevision method. As a result, just objects included until the current 
 * revisions are available. Also no changes can be done if revision is not the 
 * newest one, because PDF document doesn't support revision branching. 
 * All these operations are just delegated (after som checks) to XRefWriter.
 * <br>
 * All internal data structures which may depend on current revision are
 * intialized and cleaned up in initRevisionSpecific method.
 * <br>
 * Note that linearized documents brings some restrictions and so no revision
 * changing is supported by XRefWriter.
 * @see XRefWriter 
 *
 * <p>
 * <b>Document saving</b><br>
 * Actual changes can be saved by save method. This works in two modes depending
 * on given parameter. One creates new revision with changes, which
 * means swithces to this revision whereas second approach saves changes to the
 * end of file and doesn't change revision. Both of apporoaches have their pros
 * and cons. Basicaly, new revision should be used if changes are ended and
 * saving with no revision should be used if we want to temporarily store
 * changes to prevent data lost by program crash or whatever problem.
 * @see save(bool).
 * 
 * Different way of page content storing is so called document clonig done by
 * clone method. This method stores document content up to current revision. This
 * enables to make snapshot of document of arbitrary revision to separate
 * document and this document (as it has that revision as the newest one)
 * enables making changes.
 * 
 * <p>
 * <b>Implementation notes and limitations</b><br>
 * This version of CPdf and all its components doesn't support linearized pdf
 * files very well. Revision handling and all related, are not prepared for
 * special format and objects deployed for such documents.
 * <br>
 * Each instance cotains unique identificator which is returned by getId 
 * method.
 *
 */
class CPdf: public noncopyable
{
public:
	// NOTE: this declaration has to be here, because mode field is private and
	// so type has to be declared and also type has to be public

	/** Mode for file opening.
	 *
	 * Possible values:
	 * <ul>
	 * <li>Advanced - content of PDF can be changed and no special checking
	 * is peformed. So this brings the biggest control over content
	 * but may lead to total content corruption.
	 * <li>ReadWrite - content of PDF can be changed.
	 * <li>ReadOnly - content can't be changed.
	 * </ul>
	 * Values are placed according power of making changes. ReadOnly provides no
	 * changes, ReadWrite can change objects byt with some restrictions and
	 * Advanced have full control. If you want to add new enum value please
	 * consider this ordering.  
	 */
	enum OpenMode {ReadOnly, ReadWrite, Advanced};

	/** Constant for pdf id of no pdf.
	 * This is used for properties which comes from no pdf. Each CPdf instance
	 * must have id different from this value.
	 */
	static const cpdf_id_t NO_PDF_ID=0;

protected:
	/** Type for list of all alive pdfs.
	 */
	typedef std::vector<cpdf_id_t> CPdfListContainer;

	/** List of all aive pdfs.
	 */
	static CPdfListContainer allPdfs;

	/** Sets pdf id.
	 * Should be called only from constructor context.
	 * <br>
	 * Currently adds id to the allPdfs static array.
	 */
	void setPdfId();

	/** Releases pdf id.
	 * Should be called only from destructor context.
	 * <br>
	 * Currently removes id from the allPdfs static array and from all alive
	 * pdfs' resolve mappings.
	 */
	void releasePdfId();

	/** Observer for page tree root synchronization.
	 * 
	 * This observer is registered on Document catalog and if Pages property 
	 * is reference (as it should be) also to this reference property. Whenever 
	 * document catalog is changed and this change is done either in Pages
	 * property or directly in reference value, notify method will handle this
	 * situation and synchronize pdf internal structures with new state.
	 *
	 * @see notify
	 */
	class PageTreeRootObserver: public observer::IObserver<IProperty>
	{
		/** Pdf instance.
		 * This instance is used as page tree holder, so watch dog uses it to
		 * handle changes.
		 */
		CPdf * pdf;

	public:
		/** Initialization constructor.
		 * @param _pdf CPdf instance.
		 *
		 * Sets pdf field according parameter.
		 */
		PageTreeRootObserver(CPdf * _pdf):pdf(_pdf)
		{
			// given parameter must be non NULL
			// this is used only internaly by CPdf, so assert is enough for
			// checking
			assert(_pdf);
		}

		/** Empty destructor.
		 */
		virtual ~PageTreeRootObserver() throw(){}
		
		/** Observer handler.
		 * @param newValue New value of changed property.
		 * @param context Context of the change.
		 *
		 * This observer handles change in page tree root which is
		 * represented by Pages property of Document catalog dictionary.
		 * <br>
		 * Handling depends on given context type:
		 * <ul>
		 * <li>BasicChangeContext means that Pages reference value has changed.
		 * Property itself is kept as it is. 
		 * <li>ComplexChangeContext means that Document catalog dictionary has
		 * changed (dictionary property was added, removed or replaced). Checks 
		 * valueId from context and if it is not Pages, immediately returns.
		 * Otherwise checks oldValue type and if it is reference, unregisters
		 * this observer from property. If newValue is reference registers this
		 * observer to property.
		 * </ul>
		 * In any case: 
		 * <ul>
		 * <li>tries to get dictionary from oldValue (if it is reference) and 
		 * unregister observers from whole page tree (uses 
		 * pdf::unregisterPageTreeObservers method). 
		 * <li>invalidates pdf-::pageCount
		 * <li>clears pdf::pageList and invalidates all pages.
		 * <li>clears pdf::nodeCountCache
		 * <li>tries to get dictionary from newValue (if it is reference) and
		 * registers observers to whole new page tree (uses
		 * pdf::registerPageTreeObservers method).
		 * </ul>
		 */
		virtual void notify (
				boost::shared_ptr<IProperty> newValue, 
				boost::shared_ptr<const observer::IChangeContext<IProperty> > context) const throw();

		/** Reurns observer priority.
		 */
		virtual observer::IObserver<IProperty>::priority_t getPriority()const throw()
		{
			// TODO some constant
			return 0;
		}
	};

	/** Observer for page tree node synchronization.
	 * 
	 * This observer is responsible for intermediate page tree node change
	 * handling. From all changes in node's dictionary just Kids array property
	 * is monitored. If this property is replaced, added or removed or if it 
	 * is reference property and reference its value is changed, notify method 
	 * will handle change.
	 * <br>
	 * Note that this observer handles whole Kids property change. Kids array
	 * content is not handled here (this is done in PageTreeKidsObserver).
	 *
	 * @see notify
	 */
	class PageTreeNodeObserver: public observer::IObserver<IProperty>
	{
		/** Pdf instance.
		 * This instance is used as page tree holder, so watch dog uses it to
		 * handle changes.
		 */
		CPdf * pdf;

	public:
		/** Initialization constructor.
		 * @param _pdf CPdf instance.
		 *
		 * Sets pdf field according parameter.
		 */
		PageTreeNodeObserver(CPdf * _pdf):pdf(_pdf)
		{
			// given parameter must be non NULL
			// this is used only internaly by CPdf, so assert is enough for
			// checking
			assert(_pdf);
		}

		/** Empty destructor.
		 */
		virtual ~PageTreeNodeObserver() throw(){}
		
		/** Observer handler.
		 * @param newValue New value of changed property.
		 * @param context Context of the change.
		 *
		 * This observer handles change in page tree node.
		 * <br>
		 * Handling depends on given context type:
		 * <ul>
		 * <li>BasicChangeContext means that intermediate node has Kids property
		 * with reference type and its value has changed. Property itself is
		 * same.
		 * <li>ComplexChangeContext means that node's dictionary has changed, so
		 * valueId from context is checked. If it is not Kids (some other
		 * element is changed) immediately returns. Otherwise checks oldValue
		 * type and if it is reference, unregister this observer from property.
		 * If newValue is reference, register this observer to the property.
		 * </ul>
		 * In any case: 
		 * <ul>
		 * <li>tries to get array from oldValue (if it is reference,
		 * dereferences target object), unregisters
		 * pdf-&gt;pageTreeKidsObserver from array property and collects all
		 * reference elements from array.
		 * <li>similary does with newValue except that register
		 * pageTreeKidsObserver to the array property.
		 * <li>consolidates parent of parent node (either newValue or oldValue -
		 * depends on which is defined, because one of them may be CNull)
		 * <li>unregisters observers for all collected properties from oldValue
		 * array (uses pdf::unregisterPageTreeObservers) and consolidates
		 * pageList for each element (equivalent to removig this node)
		 * <li>similary to collected referencies from newValue array property,
		 * except that observers are registered and pageList is consolidated as
		 * if elemented has been inserted
		 * </ul>
		 * 
		 */
		virtual void notify (
				boost::shared_ptr<IProperty> newValue, 
				boost::shared_ptr<const observer::IChangeContext<IProperty> > context) const throw();

		/** Reurns observer priority.
		 */
		virtual observer::IObserver<IProperty>::priority_t getPriority()const throw()
		{
			// TODO some constant
			return 0;
		}
	};

	/** Observer for page tree node kids array synchronization.
	 * 
	 * This observer is registered on Kids array and all referecence elementes
	 * from this array. Change notified to this observer is allways page tree
	 * node insertion, delete or replacement.
	 * <br>
	 * Note that this observer is used for Kids array content or member value
	 * change, not for whole Kids array property change (like @see
	 * PageTreeNodeObserver).
	 *
	 * @see notify
	 */
	class PageTreeKidsObserver: public observer::IObserver<IProperty>
	{
		/** Pdf instance.
		 * This instance is used as page tree holder, so watch dog uses it to
		 * handle changes.
		 */
		CPdf * pdf;

	public:
		/** Initialization constructor.
		 * @param _pdf CPdf instance.
		 *
		 * Sets pdf field according parameter.
		 */
		PageTreeKidsObserver(CPdf * _pdf):pdf(_pdf)
		{
			// given parameter must be non NULL
			// this is used only internaly by CPdf, so assert is enough for
			// checking
			assert(_pdf);
		}

		/** Empty destructor.
		 */
		virtual ~PageTreeKidsObserver() throw(){} 
		
		/** Observer handler.
		 * @param newValue New value of changed property.
		 * @param context Context of the change.
		 *
		 * Checks given context type and if it is ComplexChangeContextType then
		 * Kids array has changed. If oldValue is reference property,
		 * unregisters this observer from property and if newValue is reference
		 * registers this observer property. 
		 * <br>
		 * If given context is BasicChangeContext, Kids array reference element
		 * has changed its value.
		 * <br>
		 * In any case:
		 * <ul>
		 * <li>If both newValue and oldValue are not referencies, there is 
		 * nothing to do here, because both values are just mess in array and 
		 * so immediatelly returns.
		 * <li>If oldValue is reference, than observers from whole subtree have
		 * to be unregistered because it is no more accessible from the tree.
		 * Uses pdf::unregisterPageTreeObservers method.
		 * <li>consolidates page tree for intermediate node, where change has
		 * occured. Uses getIndiRef from oldValue or newValue (depends on which
		 * is defined, because one can be CNull) and checks
		 * CPdf::pageTreeKidsParentCache. If cache entry exists, uses it. This is
		 * kind of work around to handle situation when Kids array is indirect
		 * property (cache entries are done just for such Kids arrays). Uses
		 * CPdf::consolidatePageTree method. If this method returns with
		 * false, discards CPdf::pageCount field (sets it to 0). Consolidation
		 * will change node's Count property and checks all direct childs
		 * whether they contain correct reference to parent (consolidated node).
		 * <li>consolidate CPdf::pageList with Cpdf::consolidatePageList
		 * method. Consolidation will remove and invalidate all pages from
		 * oldValue subtree and moves all which position has changed because of
		 * this removing.
		 * <li>If oldValue is reference, discards Cpdf::nodeCountCache for it
		 * and all nodes in its subtree.
		 * <li>If newValue is reference, registers obserers to new subtree. Uses
		 * CPdf::registerPageTreeObservers method.
		 * </ul>
		 */
		virtual void notify (
				boost::shared_ptr<IProperty> newValue, 
				boost::shared_ptr<const observer::IChangeContext<IProperty> > context) const throw();

		/** Reurns observer priority.
		 */
		virtual observer::IObserver<IProperty>::priority_t getPriority()const throw()
		{
			// TODO some constant
			return 0;
		}
	};
	
	/** Observer for page tree root.
	 *
	 * This observer handles changes in page tree root.
	 */
	boost::shared_ptr<PageTreeRootObserver> pageTreeRootObserver;

	/** Observer for potential intermediate nodes.
	 *
	 * This observer handles changes of Kids array in intermediate nodes.
	 */
	boost::shared_ptr<PageTreeNodeObserver> pageTreeNodeObserver;

	/** Observer for Kids array in intermeadiate nodes.
	 *
	 * This observer handles changes of Kids array members.
	 */
	boost::shared_ptr<PageTreeKidsObserver> pageTreeKidsObserver;

	/** TODO
	 */
	void unregisterPageObservers();

	/** Mapping for pdf's to their resolved storage.
	 * This mapping is used during new indirect property addition. Each separate
	 * document requires its own resolve storage because this storage contains
	 * translation mappings from that document the those used for current 
	 * document. 
	 * <br>
	 * Indirect objects with no PDF are associated with NO_PDF_ID id.
	 *
	 */
	ResolvedRefMapping resolvedRefMapping;

	/** Consolidates page tree.
	 * @param interNode Intermediate node dictionary under which change has
	 * occured.
	 * @param propagate Flag whether to consolidate also patent of given
	 * internode if it is needed (default is false - not to propagate).
	 *
	 * Recursively checks and consolidates intermediate nodes. All given nodes
	 * which are not intermediate (according getNodeType function) are ignored.
	 * In first step checks the number of page (leaf) nodes in interNode's 
	 * subtree. If this number is different than Count interNode's property then
	 * sets correct value and also interNode's parent should be consolidated. 
	 * <br>
	 * In second step, checks all childrens' Parent property to refere to this 
	 * interNode. If property is missing or has wrong value (or type), sets 
	 * correct value. Also consolidates all child which are intermediate nodes
	 * with recursive call of this method with false propagate flag (because it
	 * is enough to propage from this call).
	 * <br>
	 * Finally checks propagate flag and if it is true and also parent should be
	 * consolidated, calls method recursivelly to interNode parent (unless it is
	 * RootNode) with true propagate flag.
	 * <p>
	 * <b>Implementation notes</b>:
	 * <ul>
	 * <li>
	 * This method should be called when some change occures in page tree. Given
	 * parameter stands for dereferenced dictionary of intermediate node under
	 * which change occured (it should be indirect parent of changed value - 
	 * reference or Kids array).
	 * Change event may be:
	 * <ul>
	 * 		<li>Kids array element has been deleted/added
	 * 		<li>Kids array element has changed its value
	 * </ul>
	 * <li> Changes Count field of each intermediate node if neccessary.
	 * <li> Sets Parent field of direct children, if not set correctly.
	 * </ul>
	 *
	 * @return true if tree consolidation kept pages count, false otherwise.
	 */
	bool consolidatePageTree(const boost::shared_ptr<CDict> & interNode, bool propagate=false);
	
	/** Consolidates pageList after change in Page tree.
	 * @param oldValue Old reference (CNull if no previous state).
	 * @param newValue New reference (CNull if no future state).
	 *
	 * Removes all CPages, which are in old reference sub tree (if oldValue is 
	 * not CNull - what means that new element to Kids array has been added) 
	 * from pageList and invalidates them. Uses isNodeDescendant method for each 
	 * page from pageList to find out if it is in sub tree.
	 * <br>
	 * Also calculates difference between lost pages and newly added pages (each
	 * is calculated by number of leaf nodes from oldValue resp. newValue sub
	 * tree - uses getKidsCount helper function). If oldValue or newValue is
	 * CNull 0 is used.
	 * <br>
	 * Tries to determine which pages have to be consolidated (those which
	 * position has changed). If newValue is CNull, we have no information about 
	 * oldValue position so we can either get page information from all pages in 
	 * the list or if at least one page from oldValue subtree has been removed 
	 * from pageList, we can use this as starting point and just change position
	 * according calculated difference.
	 * <br>
	 * Page position getting is rather complex operation and may lead to whole 
	 * page tree searching. This is done only if no page position is available 
	 * from oldValue subtree.
	 * <br>
	 * This guaranties, that pages from removed subtree are not available 
	 * anymore and are invalidated and also valid returned CPage instances are 
	 * associated with correct position in pageList.
	 * <p>
	 * <b>Implementation notes</b><br>
	 * This method should be called by obsever monitoring Kids array and Kids
	 * array elements.
	 * Some previous checking and consolidation of page tree should be done
	 * before, because this method relies on proper page tree structure.
	 * <br>
	 * No parameters checking is done here. Caller must be sure that:
	 * <ul>
	 * <li>parameter must be CNull or CRef
	 * <li>CRef must point to dictionary
	 * <li>dictionary must have Type field with value Page or Pages
	 * <li>Pages dictionary must contain Count field with integer value
	 * </ul>
	 * <br>
	 * oldValue and newValue can be CNull or CRef instances. CNull case stands 
	 * for adding (if oldValue) resp. deleting (if newValue) event. If both of 
	 * them are CNull no change is done by this method. 
	 * <br>
	 * CRef stands for reference from Kids array in Intermediate node. It has 
	 * to refer to page or pages dictionary. 
	 * <br>
	 * If both values are CRef instances then oldValue has been replaced by 
	 * newValue reference. This implementation assumes that both of them were 
	 * on same position in the page tree - one sub tree was replaced by new one
	 * but on the same position. It is not possible that these values could be 
	 * on different positions (this would cause page numbering problem).
	 */
	void consolidatePageList(const boost::shared_ptr<IProperty> & oldValue, const boost::shared_ptr<IProperty> & newValue);

	/** Registers definitive value of property to the xref.
	 * @param ip Property to be used.
	 * @param ref Reference for property
	 *
	 * Converts given property to xpdf Object and calls XRefWriter::changeObject
	 * method to register it. Reference should be in RESERVED_REF.
	 * <br>
	 * As a side effect sets change field to true;
	 * 
	 * @return Reserved reference to changed object.
	 */
	IndiRef registerIndirectProperty(const boost::shared_ptr<IProperty> &ip, IndiRef &ref);
	
	/** Registers page tree observers.
	 * @param prop Page tree node reference or dictionary.
	 *
	 * If given prop is not dictionary or reference to dictionary immediatelly 
	 * returns, otherwise registers PageTreeNodeObserver to the dictionary. 
	 * Then checks Kids property. If it is reference, registers same observer
	 * also to reference property and creates entry for pageTreeKidsParentCache.
	 * Finally registers PageTreeKidsObserver to Kids array (dereferenced if 
	 * Kids is reference), to all its reference members and calls method
	 * recursively for such elements for whole subtree handling.
	 * <br>
	 * Note that this method will register observers to whole page tree if given
	 * parameter is page tree root dictionary. PageTreeRootObserver has to be
	 * registered separately.
	 * <br>
	 * This method should be called in the initialization and when new node is
	 * added to the tree (either intermediate node or leaf node).
	 */
	void registerPageTreeObservers(boost::shared_ptr<IProperty> &prop);

	/** Unregisters page tree observers.
	 * @param prop Page tree node reference or dictionary.
	 * @param cleanup Clean up flag.
	 *
	 * This method is inverse to registerPageTreeObservers (with same observers 
	 * but, with unregistration rather than registration). Also removes cache
	 * entry from pageTreeKidsParentCache.
	 * <br>
	 * This method should be called with clenaup flag set to false when 
	 * node is removed from the tree and set to true when clean up for CPdf
	 * is done (e.g. when document about to be destroyed).
	 * <br>
	 * NOTE: If position of given node is ambiguous, unregistration is skipped,
	 * because node is still in the tree.
	 */
	void unregisterPageTreeObservers(boost::shared_ptr<IProperty>& prop, bool cleanup=false);

	/** Helper method for property from different pdf adding.
	 * @param ip Property to add.
	 * @param indiRef New reference for object.
	 * @param storage Resolved storage which contains mapping from old indirect
	 * referencies to newly reserved ones.
	 * @param followRefs Flag for reference handling.
	 *
	 * Makes deep copy of given ip (to prevent changes in original ip
	 * - and also different pdf where it belongs to) and calls subsReferencies
	 * to replace all referencies in property with valid in this pdf. Finally
	 * calls registerIndirectProperty with corrected property.
	 * <br>
	 * storage parameter is not changed or used in this method directly, but it
	 * is used for subsReferencies method (see for more details). Also
	 * followRefs is not used here directly. subsReferencies may call this
	 * method, so both of parameters has to be used also here.
	 * <br>
	 * Caller has to reserve new reference for object, he wants to add and give
	 * it as indiRef parameter. Also mapping for this new reference should be
	 * done by caller. 
	 *
	 * @see registerIndirectProperty
	 * @see subsReferencies
	 * @return reference of added property.
	 */
	IndiRef addProperty(const boost::shared_ptr<IProperty> &ip, IndiRef &indiRef, 
			ResolvedRefStorage & storage, bool followRefs);

	/** Substitues reference(s) with valid in this pdf.
	 * @param ip Property to examine.
	 * @param container Resolved storage which contains mapping from old indirect
	 * referencies to newly reserved ones.
	 * @param followRefs Flag for reference handling.
	 *
	 * This method should be called on property from different pdf to prevent
	 * reference mismatching (reference in different pdf may have different
	 * meaning in this pdf).
	 * <br>
	 * Checks if given ip is reference and if it is, checks if there is existing
	 * mapping in container (searches for container element with first same as
	 * ip's reference). If there is such element, returns element::second. This
	 * guaranties two things:
	 * <ul>
	 * <li> All same referencies in original ip are mapped to same reference in
	 * result.
	 * <li> No endless loop can occure in cyclic structured property. This could
	 * happen if followRefs (see later) is true and property contains reference
	 * to another property, which contains reference back to property.
	 * </ul>
	 * If no mapping exists yet, checks followRefs parameter which controls
	 * whether referencies should be also added to the pdf. If its value is
	 * true, target property is dereferenced and addProperty method is called
	 * to add it. Given property may also be stand alone (with no pdf) and in
	 * such situation CNull property is used (because we can't get its target
	 * value and followRefs should add all indirect properties). Returned value
	 * from addProperty (reference) is used to create new mapping in container
	 * ([original, returned] mapping) and for return value.
	 * <br>
	 * followRefs with false value just reserves new reference and doesn't care
	 * for target property. Returned value from XRefWriter::reserveRef is also
	 * used for mapping creation and for return value.
	 * <p>
	 * For complex properties collects all children and calls this method
	 * recursively. If it returns with non NULL, changes child to contain new
	 * reference otherwise keeps an old one and finally returns with NULL
	 * because complex type itself didn't need reference change.
	 * <p>
	 * All other simple properties are ignored and returns with NULL.
	 * 
	 * @see addProperty
	 * @return invalid reference if no substitution has to be done or reference
	 * which should be used instead (use isRefValid for checking).
	 */
	IndiRef subsReferencies(const boost::shared_ptr<IProperty> &ip, ResolvedRefStorage & container, bool followRefs);
private:
	/** Identificator for this pdf instance.
	 */
	cpdf_id_t id;

	/**************************************************************************
	 * Revision specific data
	 *************************************************************************/
	
	/** Change flag.
	 *
	 * Value is true if and only if current changes have not been saved by save
	 * method.
	 * <br>
	 * Value is mutable because
	 *
	 * @see isChanged
	 */
	mutable bool change;
	
	/** Mapping between IndiRef and indirect properties. 
	 *
	 * This is essential when we want to access an indirect object from 
	 * refernce. We know only the id and gen number. All indirect objects
	 * with same reference has to share value and this is guarantied by this 
	 * mapping.
	 */
	mutable IndirectMapping indMap;

	/** Document catalog dictionary.
	 *
	 * It is used for document property handling. Initialization is done by
	 * initRevisionSpecific method because catalog may be specific for each
	 * revision (although this is not very often situation).
	 * Value is wrapped by shared_ptr smart pointer for instance safety.
	 */
	boost::shared_ptr<CDict> docCatalog;

	/** Type of returned pages list.
	 *
	 * It is association of page position with CPage instance. Elements are
	 * sorted according their position.
	 */
	typedef std::map<size_t, boost::shared_ptr<CPage> > PageList;

	/** Returned pages list.
	 *
	 * This container stores CPage instances returned by this class. Each time
	 * new page is returned, it is stored here with actual page position. This
	 * is because, page position may change from property interface from page
	 * tree. Each time this tree is changed, this list is consolidated and all
	 * page instances which are no more available on its position are
	 * invalidated (Uses CPage::invalidate method). 
	 * <br>
	 * pageList is invalidate on each revision change (with all its pages).
	 * <br>
	 * It is safe to try to find page in this list at first and if not found,
	 * than searching is neccessary. 
	 * <br>
	 * This storage behaves like CPage cache.
	 */
	mutable PageList pageList;

	/** Number of pages in document.
	 *
	 * Keeps value of actual number of pages or 0 if value is invalid and
	 * getPageCount has to find out it. Whenever this is set to non 0 and change
	 * in page tree occures which can change total number of pages, it is set to
	 * 0. Value is also invalidated in initRevisionSpecific method.
	 * <br>
	 * This is kind of optimalization to prevent geting Root of page tree node
	 * each time when total number of pages is required.
	 */
	mutable size_t pageCount;

	/** Cache for page count information for intermediate nodes.
	 *
	 * Each node which queries for its leaf pages count by getKidsCount
	 * function is cached with found value (more preciselly mapping from
	 * reference to page count is stored). So getKidsCount can use this cache
	 * next time when it is called to prevent from searching intermediate node
	 * subtree and calculating total page count again because this operation is
	 * used very often and so it would be serious performance problem.
	 * <br>
	 * Cached value has to be discarded each time when page count is changed
	 * under node intermediate node. This is handled in consolidatePageTree 
	 * method.
	 */
	mutable PageTreeNodeCountCache nodeCountCache;

	/** Cache for indirect Kids arrays mapping to their parents.
	 *
	 * This cache enables to overcome problem with indirect Kids arrays in
	 * Intermediate page tree node. The problem is that members of Kids array
	 * are not able to get reference to parent (if we don't want to rely on
	 * children Parent property - which is problem, because we need to get this
	 * information in page tree consolidation when also this information is
	 * checked and corrected) unless Kids array is direct object. 
	 * <br>
	 * Whenever Kids property from intermediate node is reference to array,
	 * mapping is created with array reference as key and current node's
	 * reference as value. This is done in registerPageTreeObservers which
	 * registers obsevers when page tree changes somehow. TODO when to discard?
	 */
	PageTreeKidsParentCache pageTreeKidsParentCache;

	// TODO returned outlines list

	/** Intializes revision specific stuff.
	 * 
	 * Cleans up all internal structures which may depend on the current revision.
	 * This includes indirect mapping and pageList (all pages are invalidated).
	 * After clean up is ready, initializes docCatalog field.
	 * <br>
	 * Finally registers pageTreeWatchDog observer. Uses
	 * registerPageTreeObserver method with Pages reference as parameter.
	 *
	 * @throw ElementNotFoundException if Root property is not found.
	 * @throw ElementBadTypeException if Root property is found but doesn't 
	 * contain reference or reference does not point to document catalog 
	 * dictionary.
	 * 
	 */
	void initRevisionSpecific();

	/**************************************************************************
	 * End of revision specific data
	 *************************************************************************/
	
	/** Cross reference table.
	 * This field holds XRefWriter implementation of XRef interface.
	 * It enables making changes to the table and also making changes to
	 * indirect objects.
	 * <br>
	 * This is only access point for making changes. It can be casted to 
	 * XRef types which provides information about actual object values,
	 * and so original xpdf code doesn't has to be changed.
	 * <br>
	 * This is only part which uses xpdf Object objects as arguments.
	 * <br>
	 * Instance is created in constructor.
	 */
	XRefWriter * xref;

	/** Open mode of document.
	 * 
	 */
	OpenMode mode;

	/** Mode controller instance.
	 *
	 * This class is responsible for correct assigment of mode to 
	 * properties. If not set, it's not used. 
	 * <br>
	 * Use setModeController to set one and getModeController to get
	 * actually used one.
	 */
	configuration::ModeController* modeController;

	/** Weak reference to this instance for proper reference counting
	 * with combination to published shared_ptr.
	 */
	boost::weak_ptr<CPdf> _this;

	/** Empty constructor.
	 *
	 * This constructor is disabled, because we want to prevent uninitialized
	 * instances.
	 * <br>
	 * If you want to create instance, please use static factory method 
	 * getInstance.
	 */
	CPdf ():mode(ReadOnly), modeController(NULL){}

	/** Initializating constructor.
	 * @param stream Stream with data.
	 * @param openMode Mode for this file.
	 *
	 * Creates XRefWriter, initializes pageTreeWatchDog and finally calls
	 * initRevisionSpecific method for initialization of internal structures
	 * which depends on current revision.
	 */
	CPdf(StreamWriter * stream, OpenMode openMode);
	
	/** Destructor.
	 * 
	 * It is not available outside the class, because we want to keep
	 * absolute control over instance deleting. The instance is destroyed
	 * after the its last reference to the smart pointer is released.
	 */
	~CPdf();

	/** Helper method to cleanup all data structure before CPdf destroying.
	 * This includes invalidating all pages from pageList (those returned 
	 * by getPage), unregistering all observers, clearing all cached
	 * indirect objects and cleaning up resolvedRefMapping.
	 * <br>
	 * Method has to be called before the object itself is deleted.
	 */
	void invalidate();

 	friend class PdfFileDeleter;
public:
	/** Factory method for CPdf instances.
	 * @param filename File name with pdf content (if null, new document 
	 *	will be created).
	 * @param mode Mode to open file.
	 *
	 * This is only way how to get instance of CPdf type. All necessary 
	 * initialization is done.
	 *
	 * @throw PdfOpenException if file open fails.
	 * @return Initialized (and ready to be used) CPdf instance.
	 */
	static boost::shared_ptr<CPdf> getInstance(const char * filename, OpenMode mode);

	/** Returns unique identificator for this pdf.
	 *
	 * @return Identificator of this pdf.
	 */
	cpdf_id_t getId()const
	{
		return id;
	}
	
	/** Returns pointer to cross reference table.
	 *
	 * This is pointer to CXref subtype of XRefWriter type field. It
	 * contains actual state of xref table. 
	 * If any of xpdf code is going to be used besides kernel, this
	 * can be safely used.
	 * <br>
	 * This method will return same pointer each time it is called.
	 *
	 * @return Pointer to XRefWriter field casted to CXref super type.
	 */
	CXref * getCXref()const
	{
		return dynamic_cast<CXref *>(xref);
	}
       
	/** Returns actually used mode controller.
	 *
	 * @return IModeController implementator or NULL, if no mode 
	 * controller is used.
	 */
	configuration::ModeController* getModeController()const
	{
		return modeController;
	}

	/** Sets mode controller.
	 * @param ctrl Mode controller implementator (if NULL, controller
	 * will be disabled).
	 *
	 */
	void setModeController(configuration::ModeController* ctrl)
	{
		modeController = ctrl;
	}

	/** Gets change field value.
	 *
	 * @return true if there are some new changes, false otherwise.
	 */
	bool isChanged()const
	{
		return change;
	}

	/** Returns IProperty associated with given reference.
	 * @param  ref Id and gen number of an object.
	 * 
	 * If given reference is not found in mapping, tries to fetch
	 * object using xref. If real object is returned (this means something
	 * different than objNull), creates new property object (according
	 * Object type) and creates mapping. In objNull case, returns CNull
	 * property (according PDF specification).
	 * 
	 * @return IProperty wrapped by shared_ptr smart pointer.
	 */
	boost::shared_ptr<IProperty> getIndirectProperty(const IndiRef &ref)const;

	/** Adds new indirect object.
	 * @param prop Original property.
	 * @param followRefs Flag for reference properties in complex type
	 * handling (default value is false).
	 *
	 * This method is responsible for clear indirect object creation and
	 * also safe indirect object copying between different documents (properties
	 * from different CPdf instances).
	 * <br>
	 * If given prop is from the same pdf, it is just deep copied and inserted as
	 * a new indirect object with the new reference (calls registerIndirectProperty 
	 * with newly reserved reference).
	 * <br>
	 * Otherwise tries to get ResolvedRefStorage for prop's pdf (uses getId as
	 * pdf identificator). If it doesn't exist in resolvedRefMapping, new entry 
	 * is created otherwise reuses ResolvedRefStorage instance associated with 
	 * prop's pdf.
	 * <br>
	 * Then checks whether given prop is indirect object (uses hasValidRef). If
	 * yes and there already is mapping in ResolvedRefStorage and mapped
	 * reference points to non CNull object, no object is added and just mapped
	 * reference is returned. This situation may happen when object has already
	 * been added before - e. g. as a result of addition of property which
	 * refere to this one.
	 * Otherwise creates mapping in ResolvedRefStorage (calls createMapping) and 
	 * delegates the rest to addProperty method.
	 * <p>
	 * Implementation details:
	 * <ul>
	 * <li> Value has to be proper for indirect property. This means that it
	 * can't be reference and should be indirect object (if it belongs to pdf).
	 * <li> Property from same pdf is added immediately using
	 * registerIndirectProperty method.
	 * <li> Property from different pdf instance is added by recursive 
	 * addProperty helper method.
	 * <li> Property from different pdf instance is added only if it hasn't been
	 * added before.
	 * <li> followRefs flag controls how referencies in property from different
	 * file should be handled. If value is true, also all referenced properties
	 * are added, otherwise just reserves referencies in this pdf for them and
	 * doesn't care for their values (they may be initialized later). Note that
	 * followRefs may produce rather big bunch of copying (e. g. copying page
	 * dictionary will probably cause copy of whole page tree object hierarchy,
	 * because each page contains also reference to its parent and parent
	 * contains all its kids and also its parent and so on). 
	 * <li> Initializes ResolvedRefStorage storage for addProperty to prevent
	 * endless loops for cyclyc property structures (such as mentioned above in
	 * page dictionary case) and to guarantee that same referencies are mapped
	 * to same in this pdf (@see subsReferencies). 
	 * <li> Method will fail with exception if pdf is in read only mode.
	 * </ul>
	 * <br>
	 * As a side effect sets change field to true
	 *
	 * @see registerIndirectProperty
	 * @see addProperty
	 * @throw ReadOnlyDocumentException if mode is set to ReadOnly or we are in
	 * older revision (where no changes are allowed).
	 * @return Reference of new property (see restriction when given
	 * property is reference itself).
	 */ 
	IndiRef addIndirectProperty(const boost::shared_ptr<IProperty> &prop, bool followRefs=false);

	/** Registers change of indirect property to the xref.
	 * @param prop Indirect property.
	 *
	 * Checks prop's pdf instance and if it is different than this, throws an
	 * exception. Then checks if there is mapping for prop's indiRef. If not 
	 * also throws an exception.
	 * <br>
	 * After all checking is done, creates xpdf Object from prop and calls
	 * XRefWriter::change method. 
	 * If prop is same instance as one in mapping, keeps mapping, because this
	 * means that indirect property has changed its contnet (value). Otherwise
	 * removes mapping because original property has been replaced by new
	 * property.
	 * <br>
	 * As a side effect sets change field to true
	 *
	 * @throw CObjInvalidObject if prop is not from same pdf or indirect mapping
	 * doesn't exist yet.
	 * @throw ReadOnlyDocumentException if mode is set to ReadOnly or we are in
	 * older revision (where no changes are allowed).
	 * @throw ElementBadTypeException if XrefWriter is in paranoid mode and
	 * paranoid check fails for new value.
	 */
	void changeIndirectProperty(const boost::shared_ptr<IProperty> &prop);
	
	/** Saves changes to pdf file.
	 * @param newRevision Flag for new revision creation.
	 *
	 * If revision is 0 (the newest one), uses XRefWriter::saveChanges method to
	 * store changes. Parameter newRevision has precisely the same meaning.
	 * For more implementation information @see XRefWriter
	 *
	 * <br>
	 * Method will fail if the current revision is not the most recent one.
	 * <p>
	 * <b>Usage</b>
	 * <ul>
	 * <li>
	 * <pre>
	 * save(true)
	 * </pre>
	 * should be used if changes are suitable to produce new revision. This can
	 * be when we have finished work with some topic and we want to keep
	 * information that this everything is related. 
	 * 
	 * <li><pre>
	 * save()
	 * </pre>
	 * should be used if we want to temporarily store changes to be sure that we
	 * don't lose information if some problem happens (e. g. application crashes).
	 * Next call of this function will overwrite older one.
	 * </ul>
	 * <br>
	 * As a side effect sets change field to false
	 *
	 * @throw ReadOnlyDocumentException if mode is set to ReadOnly or we aren't
	 * in the newest revision (where changes are enabled).
	 */
	void save(bool newRevision=false)const;

	/** Makes clone to file.
	 * @param fname File handle, where to store content.
	 * 
	 * Stores actual document state to the given file.
	 * Actual document state stands for all objects from all revistions until 
	 * current one. This means that kind of fork of document is done. Be
	 * careful, because actual changes are not considered (in the most recent 
	 * revision), because they are not really part of document (you have to 
	 * save them as new revision at first and than clone will contain also 
	 * actual changes).
	 * <p>
	 * <b>Usage</b>
	 * <pre>
	 * save(fileName)
	 * </pre>
	 * this should be used if we want to do fork or copy of document. This may 
	 * be helpful when we want to make changes in older revision, what is not
	 * possible normaly:
	 * <pre>
	 * cpdf->changeRevision(5);			// change to 5th revision
	 * cpdf->save("5th_revision_clone.pdf");	// creates copy of everything
	 * 						// until 5th revision
	 * </pre>
	 * Then you can open this document and make changes inside.
	 * <p>
	 * NOTE: this method doesn't check whether target of FILE handle is same
	 * file as one used in StreamWriter, so caller must take care about this to
	 * prevent unexpected problems (overwriting currently used data in stream).
	 *
	 */
	void clone(FILE * fname)const;

	/** Returns document catalog for property access.
	 * 
	 * @return Document catalog dictionary wrapped by smart pointer (using
	 * shared_ptr from boost library).
	 */
	boost::shared_ptr<CDict> getDictionary()const
	{
		// TODO need credentials?
		return docCatalog;
	}

	/** Returns trailer dictionary.
	 *
	 * Returns const Dictionary pointer to prevent doing changes to it.
	 * 
	 * @return Trailer dictionary wrapped by smart pointer (using shared_ptr
	 * from boost library).
	 */
	boost::shared_ptr<const CDict> getTrailer()const;

	/** Changes/Adds given value associated with the given name to the trailer.
	 * @param name Property name.
	 * @param value Property value.
	 * 
	 * If a property with the given name already exists, its value will be
	 * replaced by the given one (if it is safe to do so see XRefWriter::changeTrailer)
	 * or simply add a new one entry to the trailer.
	 * @throw ReadOnlyDocumentException if no changes can be done because actual
	 * revision is not the newest one or if pdf is in read-only mode.
	 * @throw NotImplementedException if document is encrypted or when trailer
	 * dictionary can't be cloned (because clone method failes).
	 * @throw ElementBadTypeException if the change is not allowed (either due to
	 * type safety or that given entry cannot be changed).
	 */
	void changeTrailer(std::string &name, const boost::shared_ptr<IProperty> &value);
		
	/** Inserts given page to the document.
	 * @param page Page used for new page creation.
	 * @param pos Position where to insert new page.
	 *
	 * If position is greater than page count, it is added after last page. 
	 * Storing to 0 position is same as if pos parameter was 1. All pages 
	 * behind pos are renumbered.
	 * <br>
	 * Method may fail if page at position is ambiguous or given page is 
	 * already stored in document. This means that it is not (or won't be) 
	 * possible to get position of page reference in its parent Kids
	 * array due to multiple occurance in Kids array. @see getNodePosition 
	 * <br>
	 * If given page comes from different valid (non NULL) pdf, some more tasks
	 * are done comparing to normal property adding to page tree. At first
	 * deep copy of page dictionary is done before adding to this pdf (all other
	 * oparations are done on cloned value). Then Parent field is removed,
	 * because we want to follow referencies in given dictionary and copying of
	 * father would lead to whole page tree structure copying (Parent contains
	 * all its Kids and also its own Parent and so on recursively). In further
	 * step all inheritable page attributes are set (because original page may
	 * not have contained them directly). Finally dictionary is added with
	 * followRefs set to true (in addIndirectProperty method) so that all 
	 * referenced objecs are deep-copied too.
	 *
	 * @throw ReadOnlyDocumentException if mode is set to ReadOnly or we are in
	 * older revision (where no changes are allowed).
	 * @throw AmbiguesPageTreeException if page can't be inserted to given
	 * position because of ambiguous page tree or page is already in the tree.
	 * @throw NoPageRootException if no page tree root can be found.
	 */
	boost::shared_ptr<CPage> insertPage(const boost::shared_ptr<CPage> &page, size_t pos);

	/** Removes page from given position.
	 * @param pos Position of the page.
	 *
	 * Removes given page from its parent Kids array. This method triggers
	 * pageList and page tree consolidation same way as it was removed manualy. 
	 * As a result page count is decreased. 
	 * <br>
	 * Method may fail if page at given position is ambigues. This means that it
	 * is not possible to get position of page reference in its parent Kids
	 * array due to multiple occurance in Kids array. @see getNodePosition
	 *
	 * <br>
	 * Intermediate nodes with no direct page are kept in page tree in this
	 * implementation.
	 *
	 * @throw PageNotFoundException if given page couldn't be found.
	 * @throw ReadOnlyDocumentException if mode is set to ReadOnly or we are in
	 * older revision (where no changes are allowed).
	 * @throw AmbiguesPageTreeException if page can't be inserted to given
	 * position because of ambiguous page tree.
	 */
	void removePage(size_t pos);

	/** Returns absolute position of given page.
	 * @param page Page to look for.
	 * 
	 * Returns actual position of given page. If given page hasn't been returned
	 * by this CPdf instance or it is no longer available, exception is thrown.
	 * <br>
	 * NOTE: instances are same if they are stand for same instance.
	 *
	 * @throw PageNotFoundException if given page is not recognized by CPdf
	 * instance.
	 */
	size_t getPagePosition(const boost::shared_ptr<CPage> &page)const;

	/** Returnes total page count.
	 *
	 * Calculates (uses getKidsCount helper function) all direct pages under
	 * page tree root node.
	 * <br>
	 * Note that if page tree root doesn't exists, it will return 0 rather than
	 * error (exception) announcing.
	 *
	 * @return Number of pages which are accessible.
	 */
	unsigned int getPageCount()const;

	// page iteration methods
	// =======================

	/** Returns page at given position.
	 * @param pos Position (starting from 1).
	 *
	 * At first tries to find page with given position in pageList. If found,
	 * returns instance from list. Otherwise, searches page tree by findPageDict
	 * helper function and if page dictionary is found, creates new CPage
	 * instance and inserts new mapping (postion to CPage instance) to pageList.
	 *
	 * @throw PageNotFoundException if pos can't be found or out of range.
	 * @return CPage instance wrapped by smart pointer.
	 */
	boost::shared_ptr<CPage> getPage(size_t pos)const;

	/** Returns first page.
	 *
	 * Calls getPage(1).
	 *
	 * @return CPage instance wrapped by smart pointer.
	 */
	boost::shared_ptr<CPage> getFirstPage()const
	{
		return getPage(1);
	}

	/** Returns next page.
	 * @param page Pointer to the page.
	 *
	 * Returns page which is after given one. Uses getPagePosition to get actual
	 * position of given page.
	 * <br>
	 * Uses getPage method.
	 *
	 * @throw PageNotFoundException if page can't be found (given page is last
	 * one or given page can't be found).
	 * @return CPage instance wrapped by smart pointer.
	 */ 
	boost::shared_ptr<CPage> getNextPage(const boost::shared_ptr<CPage> &page)const;

	/** Returns previous page.
	 * @param page Pointer to the page.
	 *
	 * Returns page which is before given one. Uses getPagePosition to get actual
	 * position of given page.
	 * <br>
	 * Uses getPage method.
	 *
	 * @throw PageNotFoundException if page can't be found (given page is last
	 * one or given page can't be found).
	 * @return CPage instance wrapped by smart pointer.
	 */
	boost::shared_ptr<CPage> getPrevPage(const boost::shared_ptr<CPage> &page)const;

	/** Checks for next page.
	 * @param page Page to check.
	 *
	 * @return true if getNextPage() method returns page and doesn't throw
	 * PageNotFoundException, false otherwise. 
	 * @throw PageNotFoundException if given page doesn't come from this CPdf
	 * instance.
	 */
	bool hasNextPage(const boost::shared_ptr<CPage> &page)const;
	
	/** Checks for previous page.
	 * @param page Page to check.
	 *
	 * @return true if getPrevPage() method returns page and doesn't throw
	 * PageNotFoundException. 
	 * @throw PageNotFoundException if given page doesn't come from this CPdf
	 * instance.
	 */
	bool hasPrevPage(const boost::shared_ptr<CPage> &page)const;

	/** Returns last page.
	 * 
	 * Calls getPage(getPageCount()).
	 *
	 * @return CPage instance wrapped by smart pointer.
	 */
	boost::shared_ptr<CPage> getLastPage()const
	{
		return getPage(getPageCount());
	}

	// Version handling and work around
	// =================================

	/** Returns mode of this version.
	 *
	 * @return mode field value or ReadOnly if current revision is not the
	 * newest one.
	 */
	OpenMode getMode() const
	{
		// mode is used only if we are in the newest revision, otherwise we are
		// in ReadOnly
		return (utils::isLatestRevision(*xref))?mode:ReadOnly;
	}

	/** Checks whether this pdf is linearized.
	 *
	 * Delegates to XRefWriter::isLinearized.
	 *
	 * @see XRefWriter::isLinearized
	 * @return true if document is linearized, false otherwise.
	 */
	bool isLinearized()const
	{
		return xref->isLinearized();
	}

	/** Revision type.
	 * This is used to determine which revision should or is used.
	 * It is only alias to unsigned number and 0 stands for the 
	 * oldest revision and newer revisions have higher numbers.
	 */
	typedef unsigned revision_t;

	/** Returns name of actual revision number.
	 *
	 * Just delegate to the XRefWriter typed xref field.
	 * see XRefWriter::getActualRevision() method
	 */
	revision_t getActualRevision()const
	{
		return xref->getActualRevision();
	}

	/** Gets size of given revision.
	 * @param rev Revision to examine.
	 * @param includeXref Flag controling whether also xref section with trailer
	 * should be also considered.
	 *
	 * Delegates to XRefWriter::getRevisionCount.
	 *
	 * @see XRefWriter::getRevisionCount
	 * @return Size of given revision in bytes.
	 */
	size_t getRevisionSize(unsigned rev, bool includeXref=false)const
	{
		// just delegates to xref
		return xref->getRevisionSize(rev, includeXref);
	}

	/** Changes revision to given one.
	 * @param revisionNum Revision number (the oldest is 0, getRevisionCount()-1 
	 * for the newest one).
	 *
	 * Delegates to xref field and reinitializes all internal structures
	 * which are revision specific (calls initRevisionSpecific method).
	 * <br>
	 * NOTE: indirect mapping is cleared so, all indirect properties are lost
	 * and shouldn't be used anymore.
	 *
	 * @see XRefWriter::changeRevision
	 * @see initRevisionSpecific
	 */
	void changeRevision(revision_t revisionNum);

	/** Returns number of available revisions.
	 *
	 * @see XRefWriter::getRevisionCount
	 */
	size_t getRevisionsCount()const
	{
		return xref->getRevisionCount();
	}

	/** Returns container of outlines and the string they represent.
	 * @param cont Output container.
	 *
	 * Traverses tree like structure of outlines and stores them in the order they
	 * are visited.
	 */
	template<typename Container>
	void getOutlines (Container& cont)const
	{

		check_need_credentials(xref);

		// Clear outline container
		cont.clear ();
		
		// topleve outline if any
		boost::shared_ptr<CDict> toplevel;
		try {
			
			toplevel = docCatalog->getProperty<CDict>("Outlines");
			
		}catch (ElementNotFoundException&)
		{
			kernelPrintDbg (debug::DBG_DBG, "No outlines");
			return;
		}
		assert (toplevel);

		utils::getAllChildrenOfPdfObject (toplevel, cont);
	}

	/** Returns current xref's pdf content writer.
	 * This instance can't be deallocated! It should be used only for observer
	 * registration or similar purposes.
	 */
	utils::IPdfWriter * getPdfWriter()
	{
		return xref->setPdfWriter(NULL);
	}

	/** Throws an exception if this document can not be changed. */
	void canChange () const;

	/** Checks whether encryption credentials are required for docuement.
	 * @return true if no credentials have been set yet and they are
	 * required (setCredentials method has to be called), false otherwise.
	 */
	bool needsCredentials()const;

	/** Sets credentials for encrypted document.
	 * Delegates to CXref::setCredentials method.
	 */
	void setCredentials(const char * ownerPasswd, const char * userPasswd);
};


// helper methods
namespace utils 
{
	
/** Type enumeration for page tree nodes.
 * Type of dictionary in page tree. Possible values are:
 * <ul>
 * <li>ErrorNode - node has bad type (it is not dictionary or reference to
 * dictionary).
 * <li>UnknownNode - node is dictionary but it is not possible to get node type 
 * <li>LeafNode - leaf tree node (Page dictionary).
 * <li>InterNode - intermediate node (Pages dictionary).
 * <li>RootNode - intermediate root node.
 * </ul>
 *
 * <p>
 * Implementation node:<br>
 * Note that order is significant, because we assume that everything lower than
 * LeafNode is kind of error (problem) and also that greater or equal than
 * InterNode is intermediate node.
 */
enum PageTreeNodeType { ErrorNode, UnknownNode, LeafNode, InterNode, RootNode };

/** Gets page tree root node dictionary.
 * @param pdf Pdf where to search.
 *
 * Gets Pages field from pdf dictionary and dereference it to dictionary. If it
 * is not reference or target object is not a dictionary, returns NULL
 * dictionary.
 * <br>
 * Note that this function never throws.
 *
 * @return Dictionary wrapped by shared_ptr (NULL dictionary if not found).
 */
boost::shared_ptr<CDict> getPageTreeRoot(const boost::shared_ptr<CPdf> &pdf);
	
/** Helper method to find page at certain position.
 * @param pdf Pdf instance where to search.
 * @param pagesDict Reference to or Page or Pages dictionary representing 
 * page node (see Pdf standard notes).
 * @param startPos Starting position for searching (see note below).
 * @param pos Page position (starting from 1) to find.
 * @param cache Cache for reference to page count mapping.
 *
 * Method recursively goes through page subtree starting with given page tree 
 * node until given page position is found or no such position can be found. If 
 * position can't be found under given page node, exception is thrown.
 * <br>
 * If given pagesDict is reference, uses CPdf::getIndirectProperty to get 
 * target indirect object, which should be a dictionary (otherwise throws
 * ElementBadTypeException).
 * <br>
 * Note that this function is not able to handle cycles in page tree and if any
 * occures, endless loop will happen.
 * <br>
 * startPos stands for position of pagesDict in whole page tree. In fact it is
 * position of first leaf node in subtree.
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
 * <li>Page dictionary - leaf node which represents direct page.
 * </ul>
 * This structure is rather complex but enables effective way to access 
 * arbitrary page in short time (some applications provide balanced tree form
 * to enable very effective access).
 * <p>
 * <p>
 * <b>Implementation notes</b>:<br>
 * Function tries to find page also in page tree structure which doesn't follow
 * pdf specification. All wierd page tree elements are ignored and just those
 * which may stand for intermediate or leaf nodes are condidered. Also doesn't
 * use Count or Parent field information during searching. Uses getKidsCount
 * function to get intermediate leaf nodes count. getKidsCount method requieres 
 * also cache which stores already known nodes to their counts mapping. This 
 * function just delegates given cache parameter to getPageCount and doesn't 
 * care for it much more. If it is NULL, it is not used.
 * <p>
 * <b>Usage notes</b>:<br>
 * If searching from begining, page tree root should be used as pagesDict
 * parameter and startPos set to 1.
 * <pre>
 * Example:
 * shared_ptr<CDict> pageTreeRoot=getPageTreeRoot(pdf);
 * if(pageTreeRoot.get())
 * 	findPageDict(pdf, pageTreeRoot, 1, posToSearch, NULL);
 * </pre>
 * <br>
 * Searching can start also from different intermediate node than root, but 
 * startPos has to be correct position of this node in the tree (same value 
 * as returned by getNodePosition method applied on such node). This usage can
 * be used for searching optimization when just part of the tree is searched.
 *
 * @throw PageNotFoundException if given position couldn't be found under
 * subtree defined by pagesDict.
 * @throw ElementBadTypeException if pagesDict is not dictionary or reference to
 * dictionary.
 * @return Dereferenced page (wrapped in shared_ptr) dictionary at given 
 * position.
 */
boost::shared_ptr<CDict> findPageDict(
		const boost::shared_ptr<CPdf> &pdf, 
		const boost::shared_ptr<IProperty> &pagesDict, 
		size_t startPos, 
		size_t pos, 
		PageTreeNodeCountCache * cache);

/** Gets position of given node.
 * @param pdf Pdf where to examine.
 * @param node Node to find (CRef or CDict instances).
 * @param cache Cache for reference to page count mapping.
 *
 * Starts searching for given node from root of the page tree (returned from 
 * getPageTreeRoot function). Uses recursive searchTreeNode function for 
 * searching and provides just error handling wrapper to this function. 
 * searchTreeNode reqieres also cache which stores already known nodes to their 
 * counts mapping. This function just delegates given cache parameter to 
 * getPageCount and doesn't care for it much more. If it is NULL, it is not 
 * used.
 * <br>
 * Prefer to use this function instead of searchTreeNode if you are not sure you
 * know what you are doing.
 *
 * @throw PageNotFoundException If node can't be found.
 * @throw ElementBadTypeException If given node is not dictionary or reference
 * to dictionary.
 * @throw AmbiguousPageTreeException if node position can't be determined bacause
 * of page tree ambiguity (see searchTreeNode for more information).
 * @return Node position.
 */
size_t getNodePosition(const boost::shared_ptr<CPdf> &pdf, 
		const boost::shared_ptr<IProperty> &node, 
		PageTreeNodeCountCache * cache);

/** Calculates number of direct pages under given node property.
 * @param interNodeProp Page tree node property (must be dictionary or reference
 * to dictionary).
 * @param cache Cache with node reference to leaf page count (if NULL. it is not
 * used).
 *
 * Checks whether given node is LeafNode and if so, immediatelly returns with 1
 * (leaf contains one direct page). Otherwise tries to get node dictionary
 * from given property. If not able to do so, returns 0, because this node is
 * probably invalid and so it can't contain any direct page node. 
 * Then checks whether given cache parameter is non NULL and if so checks cached
 * value for given node (uses getCachedValue function). If cache entry exists
 * for this node, uses cached value. Otherwise collects all Kids elements from 
 * dictionary (uses getKidsFromInterNode function) and calls this function 
 * recursively on each reference element. Collected number is returned and if 
 * cache is non NULL also caches value (uses updateCache function).
 * <br>
 * Note that this function never throws.
 *
 */
size_t getKidsCount(const boost::shared_ptr<IProperty> & interNodeProp, PageTreeNodeCountCache * cache)throw();
	
/** Checks given node for its page tree type.
 * @param nodeProp Node property (must be dictionary or reference to
 * dictionary).
 *
 * Gets node dictionary in first step (either directly from parameter or
 * dereference). If not able to get it, returns ErrorNode type. Then checks
 * whether node dictionary is same as Page tree root node and if so, returns
 * RootNode type. If not able to get root node, returns UnknownNode.
 * <br>
 * In first step compares dictionary with pageTreeRoot dictionary. If they are
 * same (uses == operator), returns RootNode. Otherwise checks for Type field 
 * in node dictionary and if present, checks its value. It must be name object.
 * If so and value is Page, returns LeafNode, or if value is Pages, returns 
 * InterNode. Otherwise returns (also if Type field type is not name), returns
 * UnknownNode.
 * <br>
 * Finally tries to determine node type from existing fields. If dictionary
 * contains Kids array, it is considered to be InterNode. Otherwise returns
 * UnknownNode.
 * <br>
 * Note that this function never throws.
 *
 * @return Node type.
 */
PageTreeNodeType getNodeType(const boost::shared_ptr<IProperty> & nodeProp)throw();

/** Checks if given child is descendant of node with given reference.
 * @param pdf Pdf where to resolv referencies.
 * @param parent Reference of the parent.
 * @param child Dictionary of page(s) node.
 *
 * Checks if child's Parent field has same reference as given one as parent
 * parameter. If yes then child dictionary is descendant of node with parent
 * reference. If not, dereference child's parent and continues in recursion
 * using dereferenced parent dictionary as new child for recursion call.
 * <br>
 * NOTE: this method doesn't perform any checking of parameters and doesn't work
 * if Parent property has not valid value (according page tree state).
 *
 * @return true If given child belongs to parent subtree, false otherwise.
 */
bool isNodeDescendant(const boost::shared_ptr<CPdf>& pdf, const IndiRef &parent, const boost::shared_ptr<CDict> &child);

/** Collects all kids elements from internode dictionary.
 * @param interNodeDict Intermediate node dictionary.
 * @param container Container where to store kids.
 *
 * Clears given container at first. Gets Kids array from given dictionary and
 * adds all its elements to given container. If not able to do so, immediatelly
 * returns. Given container is empty if no kid is found (or any other problem
 * occures).
 * <br>
 * Container template type must store shared_ptr<IProperty> types and support
 * clear and push_back methods.
 * <br>
 * Note that this function never throws.
 */
template<typename Container>
void getKidsFromInterNode(const boost::shared_ptr<CDict> & interNodeDict, Container & container)throw();

/** Checks whether file content is encrypted.
 * @param pdf Pdf instance to check.
 * 
 * Simple helper which asks its cross reference table.
 *
 * @return true if file content is encrypted, false otherwise.
 */
bool isEncrypted(const boost::shared_ptr<CPdf> &pdf);

// Following functions have to be in this header files because they are templates
// (so have to be in header) and cobjecthelpers.h cannot include cpdf.h which
// is necessary for CPdf type completness

/** Returns cobjects from given reference property.
 * @param refProp Reference property (must be pRef typed).
 *
 * Gets reference value from property and dereferences indirect object from it.
 * Uses refProp's pdf for dereference (so it has to be valid).
 * Checks target object type and if it is correct (CType::type), casts it to
 * given CType (uses IProperty::getSmartCObjectPtr).
 *
 * @throw ElementBadTypeException if refProp is not CRef instance or indirect
 * object is not CType instance.
 * @return CType instance wrapped by shared_ptr smart pointer.
 */
template<typename CType>
boost::shared_ptr<CType> getCObjectFromRef(const boost::shared_ptr<IProperty> &refProp)
{
	// REMARK
	// This helper has to be here because of gcc template manipulation
	// (cobjecthelpers.h can't include from this header file)
	
	if(!isRef(refProp))
		throw ElementBadTypeException("");
	
	// gets reference value and dereferences indirect object
	boost::shared_ptr<CPdf> pdf = refProp->getPdf().lock();
	assert(pdf);
	IndiRef ref;
	IProperty::getSmartCObjectPtr<CRef>(refProp)->getValue(ref);
	boost::shared_ptr<IProperty> indirect_ptr=pdf->getIndirectProperty(ref);
	if(indirect_ptr->getType() != CType::type)
		throw ElementBadTypeException("");
	return IProperty::getSmartCObjectPtr<CType>(indirect_ptr);
}

/** Returns cobjects from given reference and pdf.
 * @param ref Indirect reference.
 * @param pdf CPdf instance where to fetch.
 *
 * Calls getIndirectProperty on given pdf and cast returned indirect object to
 * given CType (if it has correct type CType::type).
 *
 * @throw ElementBadTypeException if indirect object is not CType instance.
 * @return CType instance wrapped by shared_ptr smart pointer.
 */
template<typename CType>
boost::shared_ptr<CType> getCObjectFromRef(const IndiRef& ref, const boost::shared_ptr<CPdf>& pdf)
{
	// REMARK
	// This helper has to be here because of gcc template manipulation
	// (cobjecthelpers.h can't include from this header file)	
	assert(pdf);
	boost::shared_ptr<IProperty> indirect_ptr=pdf->getIndirectProperty(ref);
	if(indirect_ptr->getType() != CType::type)
		throw ElementBadTypeException("");
	return IProperty::getSmartCObjectPtr<CType>(indirect_ptr);
}

} // namespace utils

} // namespace pdfobjects

#endif // __CPDF_H__
