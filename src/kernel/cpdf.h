// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80

/*
 * Created:  01/28/2006 03:48:14 AM CET

 * $RCSfile$
 *
 * $Log$
 * Revision 1.59  2006/06/05 08:57:32  hockm0bm
 * refactoring CObjectSimple
 *         - getPropertyValue -> getValue
 *         - writeValue -> setValue
 *
 * Revision 1.58  2006/06/05 06:27:02  hockm0bm
 * * CPdf::subsReferencies
 *         - substitues also referencie also for streams
 * * CPage::insertPage
 *         - handles page from different file correctly
 *
 * Revision 1.57  2006/06/03 09:15:02  hockm0bm
 * getCObjectFromRef removed from cobjecthelpers.h to cpdf.h because gcc >=4.0
 * didn't like CPdf usage
 *
 * REMARK:
 * gcc 3.3 was ok with such construction
 *
 * Revision 1.56  2006/06/02 16:38:15  hockm0bm
 * modecontroller field has not been initialized in constructors
 *         - thx to Jozo
 *
 * Revision 1.55  2006/06/02 11:31:46  misuj1am
 *
 * -- ADD: outline get text function
 * -- CHANGE: getOutlines returns just iproperites
 * --tests changed
 *
 * Revision 1.54  2006/06/02 09:21:45  misuj1am
 *
 * -- ADD: CPdf::getOutlines function
 * -- test changed to test that function
 *
 * Revision 1.53  2006/06/01 14:46:24  hockm0bm
 * doc update - doxygen warnings removed (if possible)
 *
 * Revision 1.52  2006/06/01 14:39:49  hockm0bm
 * Outline methods removed
 *
 * Revision 1.51  2006/05/31 20:10:55  hockm0bm
 * IModeController renamed to ModeController
 *
 * Revision 1.50  2006/05/30 19:10:49  hockm0bm
 * isLinearized method added
 *
 * Revision 1.49  2006/05/17 19:43:12  hockm0bm
 * getRevisionSize method added
 *
 * Revision 1.48  2006/05/16 17:50:58  hockm0bm
 * minor changes
 *
 * Revision 1.47  2006/05/15 18:30:23  hockm0bm
 * * isEncrypted bug fixed
 *         - Encrypt can be also reference to dictionary
 * * isDecrypted declaration removed from header file
 *
 * Revision 1.46  2006/05/13 21:36:52  hockm0bm
 * * addIndirectProperty reworked (problem with cyclic reference dependencies)
 *         - new followsRef flag
 *         - throws if given ip is reference
 * * addProperty, subsReferencies, createMapping methods added
 * * addReferencies replaced by subsReferencies
 * * doc update
 *
 * Revision 1.45  2006/05/10 21:09:26  hockm0bm
 * isChanged meaning changed
 *         - returns true if there are any chnages after last save (or pdf creation)
 *         - doesn't check current revision
 *
 * Revision 1.44  2006/05/10 20:42:13  hockm0bm
 * * new Cpdf::getTrailer method
 * * new utils::isEncrypted
 *
 * Revision 1.43  2006/05/09 20:05:19  hockm0bm
 * minor changes
 *
 * Revision 1.42  2006/04/28 17:15:51  hockm0bm
 * * helper functions exported by header file now
 *
 * Revision 1.41  2006/04/27 18:29:11  hockm0bm
 * * all methods which could be set to const are const now
 * * pageList is mutable to enable getPage (and all depended in getPage)
 *   to be const methods
 * * pageCount is mutable to enable getPageCount to be conts
 * * methods which should fail in read-only mode doesn't compare mode
 *   directly but use getMode - because it consideres also current revision
 * * changeIndirectProperty bug fixed - checking for mode is done now
 *
 * Revision 1.40  2006/04/27 05:55:39  hockm0bm
 * * changeRevision implemented
 * * documentation update
 *
 * Revision 1.39  2006/04/25 22:16:41  misuj1am
 *
 * -- ADD: headers cobject.h
 *
 * Revision 1.38  2006/04/24 01:09:10  jahom0bm
 * Oprava vytvarania obrazu stranok pre PageSpace.
 *
 * Revision 1.37  2006/04/23 15:12:58  hockm0bm
 * changeIndirectProperty behaviour changed
 *         - indirect mapping removed only if given property is different
 *           instance than given one
 *         - throws CObjInvalidObject exception is thrown if prop is not from this
 *           pdf or mapping doesn't exist
 *
 * Revision 1.36  2006/04/23 10:37:53  hockm0bm
 * insertPage and removePage
 *         - AmbiguouPageTreeException added to documentation
 *
 * Revision 1.35  2006/04/22 20:11:06  hockm0bm
 * bug fix corrected
 *         - hasNextPage, hasPrevPage returns true if successful
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
 * Revision 1.32  2006/04/19 05:57:46  hockm0bm
 * * pageTreeWatchDog wrapped by shared_ptr
 * * print messages consolidated a bit
 * * exception handling and error prone in page tree functions
 *
 * Revision 1.31  2006/04/17 20:11:47  hockm0bm
 * * OpenMode reorganized
 *         - ReadOnly is first now
 *         - values are ordered according what everything can be done with file
 * * getInstance corrected
 *         - file is opened in append mode (r+) anytime mode is >=ReadWrite
 *           (also for advanced - this didn't work in previous version)
 *
 * Revision 1.30  2006/04/15 08:01:32  hockm0bm
 * * pdfFile field removed
 *         - we are using transparent StreamWriter now
 * * PageTreeWatchDog class implemented as Observer handling page tree changes.
 * * changeIndirectProperty method added
 *         - registers changed property to the XRefWriter
 *
 * Revision 1.29  2006/04/13 18:16:30  hockm0bm
 * insert/removePage, addIndirectProperty, save throws ReadOnlyDocumentException
 *
 * Revision 1.28  2006/04/12 17:55:21  hockm0bm
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
 * Revision 1.27  2006/04/02 08:26:40  hockm0bm
 * PdfOpenException documented for getInstance method
 *
 * Revision 1.26  2006/04/01 07:56:04  hockm0bm
 * CPage include changed to forward declaration
 *         - module compilation ok when include but kernel compilation failed
 *         - forward declaration works better
 *
 * Revision 1.25  2006/04/01 07:27:52  hockm0bm
 * added include cpage.h
 *
 * Revision 1.24  2006/04/01 00:23:22  hockm0bm
 * sync with CVS - no code change - CVS is complaining
 *
 * Revision 1.23  2006/03/31 23:05:11  hockm0bm
 * * hasNextPage  - implemented
 * * hasPrevPage  - implemented
 *
 * Revision 1.22  2006/03/31 22:36:52  hockm0bm
 * names changed from /Name -> Name
 *
 * Revision 1.21  2006/03/31 21:03:00  hockm0bm
 * removePage implemented
 *
 * Revision 1.20  2006/03/30 21:29:33  hockm0bm
 * * findPage renamed to findPageDict
 *         - return value changed to shared_ptr<CDict>
 * * TODOs for exceptions unification
 * * code simplification for dictionary handling
 *         - using utils::getDictFromRef more often
 * * printDbg for more methods
 * * insertPage implemented
 *
 * Revision 1.19  2006/03/29 06:12:34  hockm0bm
 * consolidatePageTree method added
 * starting to use getPageFromRef
 *
 * Revision 1.18  2006/03/27 22:28:42  hockm0bm
 * consolidatePageList method added to CPdf
 *
 * Utils namespace:
 * isDescendant method for subtree information
 * getNodePosition method for CDict -> position
 * searchTreeNode helper method
 *
 * Revision 1.17  2006/03/25 21:23:57  hockm0bm
 * pages handling implemented - pageList created
 *         insertPage, removePage - only in design state
 * page iteration methods implemented
 * initRevisionSpecific handles pageList clean up
 *
 * Revision 1.16  2006/03/25 11:56:39  hockm0bm
 * debuging work around
 *
 * Revision 1.15  2006/03/24 20:06:50  hockm0bm
 * createPage and createOutline removed
 *         - they don't make sense because Pages can be created directly and
 *         then inserted
 *         - same with outlines
 * comments for most of nonimplemented methods
 *
 * Revision 1.14  2006/03/21 23:21:39  hockm0bm
 * minor interface changes
 * compileable but not usabe in this state
 * CPage question not solved yet
 *
 * Revision 1.12  2006/03/13 01:35:10  misuj1am
 *
 *
 * -- modecontroller
 *	get/setModeController, if null OUR modecontroller is used
 *
 * Revision 1.11  2006/03/08 12:13:59  misuj1am
 *
 * -- precompiled headers support
 * -- unused arguments commented out
 *
 * Revision 1.10  2006/03/06 18:18:55  hockm0bm
 * compilable changes - each module is compilable now
 * each object in pdfobjects namespace
 * more comments
 * cpdf - most of methods are just comments how to implement
 *
 * Revision 1.9  2006/02/28 22:57:19  hockm0bm
 * Just few obvious errors - still not compileable (due to errors in cobjectI.h)
 *
 * Revision 1.8  2006/02/28 22:41:41  hockm0bm
 * Scratch of the implementation - not compileable
 * Most of functions described with kind of pseudocode
 * Waiting for CObject clarification (to implement stuff)
 *
 *
 */


#ifndef __CPDF_H__
#define __CPDF_H__

#include "static.h"

// xpdf
#include "xpdf.h"

// mode controller
#include "modecontroller.h"
#include "iproperty.h"
#include "cobject.h"
#include "xrefwriter.h"
#include "cobjecthelpers.h"

// =============================================================================
namespace pdfobjects {

namespace utils {

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
	 * @return true if one &lt two or false otherwise.
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

// forward declarations FIXME remove
class CPage;
class COutline;

/** Type for resolv mapping.
 * Element::first stands for reference in original property and
 * Element::second stands for reserved reference which should replace
 * original one.
 */
typedef std::vector<std::pair<IndiRef, IndiRef> > ResolvedRefStorage;
	
/** CPdf special object.
 *
 * This class is responsible for pdf document maintainance. It provides wrapper
 * on document catalog dictionary with advanced logic concering revision
 * handling, high level object creation and their synchronization with actual
 * properties state and mode depending operations.
 *
 * <p>
 * <b>Instancing</b><br>
 * Public constructor is not available and instances can be created on by 
 * static factory getInstance method. Also no public destructor is available 
 * and instance can be destroyed only by close method.
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
 * document content. It is not accessible outside from class to keep full
 * control upon instance in CPdf. To enable using also xpdf code outside CPdf
 * CXref casted instance of XRefWriter is returned in getCXref. This instance
 * doesn't enable any chnages but enables access to most accurate indirect
 * objects. Changes to XRefWriter can be done only by CPdf methods. Note that
 * changes can be be done only in newest revision (@see save).
 * 
 * <p>
 * <b>Document properties</b><br>
 * Properties from document can be accessible from document catalog which is
 * returned by getDictionary method. New indirect property, which may be used in
 * some other property (by its reference) can be created by addIndirectProperty.
 * Change of indirect property can be registered by changeIndirectProperty.
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
 * same position returns same page instance (unless page tree is not changed).
 * CPdf uses PageTreeWatchDog inner class for page tree synchronization. So
 * changes can be done also directly in page tree (not using CPdf methods) and
 * pageList will contain correct (available) CPage instances.
 * <br>
 * insertPage and removePage enables inserting and removing new pages to the
 * page tree. This way is prefered for making such changes. Other way (as
 * mentioned above) is to change page tree directly using property interface.
 * This way may lead to errors which are not recoverable and so it is strongly
 * discouraged.
 * <br>
 * Pages are counted from 1 (first page) up to getPageCount return value. Note
 * that this may not represent values used for inner page counting writen on the
 * page. CPdf doesn't handle any kind of special document numbering.
 *
 * TODO outlines describtion
 *
 * <p>
 * <b>Revision manipulation</b><br>
 * CPdf provides access for working with document revision handled in XRefWriter
 * field. Actual revision number (the newest revision has 0 number and grows to
 * older revisions) can be obtained by getActualRevision method. Current
 * revision can be changed by changeRevision method. As a result, just object
 * included until current revisions are available. Also no changes can be done
 * if revision is not the newest one, because PDF document doesn't support
 * revision branching.
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
 * clone method. This method stores document content of current revision. This
 * enables to nake snapshot of document of arbitrary revision to separate
 * document and this document (as it has that revision as the newest one)
 * enables making changes.
 * 
 * <p>
 * <b>Implementation notes and limitations</b><br>
 * This version of CPdf and all its components doesn't support linearized pdf
 * files very well. Revision handling and all related, are not prepared for
 * special format and objects deployed for such documents.
 * <br>
 * Newly created revision always uses old style cross reference tables.
 *
 */
class CPdf
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
	
protected:

	/** Observer for Page tree synchronization.
	 *
	 * This observer should be registered on all intermediate nodes for Kids
	 * array and its members (referencies). It should be done before any changes
	 * are done.
	 * <br>
	 * See notify method for implementation details.
	 *
	 */
	class PageTreeWatchDog: public observer::IObserver<IProperty>
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
			PageTreeWatchDog(CPdf * _pdf):pdf(_pdf)
			{
				// given parameter must be non NULL
				// this is used only internaly by CPdf, so assert is enough for
				// checking
				assert(_pdf);
			}

			/** Empty destructor.
			 */
			virtual ~PageTreeWatchDog() throw(){}; 
			
			/** Observer handler.
			 * @param newValue New value of changed property.
			 * @param context Context of the change.
			 *
			 * Checks type of the context. If it is not BasicChangeContextType,
			 * immediately returns, because it is not able to handle situation
			 * without oldValue.
			 * <br>
			 * Checks newValue's type and if its type is CRef, registers 
			 * observer (this instance) to it using registerPageTreeObserver
			 * method on value (this will guarantee that if it is intermediate
			 * node, observer will be register to whole sub tree) and starts 
			 * consolidation of the page tree on indirect parent of newValue. 
			 * <br>
			 * Finaly starts consolidation of pageList where oldValue is used
			 * from context.
			 * <p>
			 * newValue is always element of Kids array or CNull when this
			 * element has been removed from array. Context contains oldValue
			 * which is previous value of element on the same position as
			 * newValue. It also may be CNull when no previous value existed 
			 * (when new element is added to the array). 
			 * <br>
			 * This method guarantees that page tree will contain valid Count
			 * and Parent fields in all affected nodes and pageList will
			 * contain correct mapping from page position to CPage instances
			 * and invalidates all pages which are not accessible anymore,
			 * because whole subtree has been removed.
			 *
			 */
			virtual void notify (boost::shared_ptr<IProperty> newValue, boost::shared_ptr<const observer::IChangeContext<IProperty> > context) const throw();

			/** Reurns observer priority.
			 */
			virtual observer::IObserver<IProperty>::priority_t getPriority()const throw()
			{
				// TODO some constant
				return 0;
			}
	};
	
	/** Page tree watch dog observer.
	 *
	 * This instance is used to handle changes in page tree.
	 */
	boost::shared_ptr<PageTreeWatchDog> pageTreeWatchDog;

	/**
	 * Indirect properties mapping type.
	 */
	typedef std::map<IndiRef, boost::shared_ptr<IProperty>, utils::IndComparator> IndirectMapping;

	/** Consolidates page tree.
	 * @param interNode Intermediate node dictionary under which change has
	 * occured.
	 *
	 * In first step checks the number of page nodes in interNode's subtree.
	 * Collects number for pages just from direct subnodes - members of Kids
	 * array (Page dictionary for 1 and Pages for Count).
	 * <br>
	 * Second step of consolidation checks whether all children from Kids array
	 * have Parent set to given interNode. If not, Parent reference is changed to
	 * contain correct value.
	 * <p>
	 * <b>Implementation notes</b>:
	 * <ul>
	 * <li>
	 * This method should be called when some change occures in page tree. Given
	 * parameter stands for dereferenced dictionary of intermediate node under
	 * which change occures (it should be indirect parent of changed value - 
	 * reference or Kids array). If given dictionary has not Pages type, throws 
	 * exception.
	 * Change event may be:
	 * <ul>
	 * 		<li>Kids array element has been deleted/added
	 * 		<li>Kids array element has changed its value
	 * </ul>
	 * <li> Doesn't perform any parameter checking. Relay on correct value.
	 * <li> Changes Count field of each intermediate node if neccessary.
	 * <li> Sets Parent field of direct children, if not set correctly.
	 * <li> Doesn't go deeply in to children
	 * <li> If page tree was consistent before change under this interNode 
	 * occured, it will be consistent after this consolidation too (NOTE all
	 * children of given node must be correct too).
	 * <li> Exception is thrown only if page tree demage and method is not able
	 * to handle it.
	 * </ul>
	 *
	 * @throw ElementBadTypeException if given dictionary has not Pages type or
	 * Kids array can't be found or any other required field has bad type.
	 * @throw ElementNotFoundException if any of required field can't be found.
	 *
	 * @return true if tree consolidation kept pages count, false otherwise.
	 */
	bool consolidatePageTree(boost::shared_ptr<CDict> interNode);
	
	/** Consolidates pageList after change in Page tree.
	 * @param oldValue Old reference (CNull if no previous state).
	 * @param newValue New reference (CNull if no future state).
	 *
	 * Removes all CPages, which are in old reference sub tree (if oldValue is 
	 * not CNull) from pageList and invalidates them. Uses isDescendant method 
	 * for each page from pageList to find out if it is in sub tree.
	 * <br>
	 * Also calculates difference between lost pages (if oldValue is not CNull) 
	 * and newly added pages (if newValue is not CNull - checks type of node 
	 * and if node is page, only 1 is lost, in case of intermediate node Count 
	 * field is used).
	 * <br>
	 * Tries to determine which pages has to be consolidated (those which
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
	 * This method should be called by handler of change event on Page tree.
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
	void consolidatePageList(boost::shared_ptr<IProperty> oldValue, boost::shared_ptr<IProperty> newValue);

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
	IndiRef registerIndirectProperty(boost::shared_ptr<IProperty> ip, IndiRef ref);
	
	/** Helper method for property adding.
	 * @param ip Property to add.
	 * @param indiRef New reference for object.
	 * @param storage Resolved storage which contains mapping from old indirect
	 * referencies to newly reserved ones.
	 * @param followRefs Flag for reference handling.
	 *
	 * If given property is from same pdf, just calls registerIndirectProperty.
	 * Otherwise makes deep copy of given ip (to prevent changes in original ip
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
	IndiRef addProperty(boost::shared_ptr<IProperty> ip, IndiRef indiRef, ResolvedRefStorage & storage, bool followRefs);

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
	IndiRef subsReferencies(boost::shared_ptr<IProperty> ip, ResolvedRefStorage & container, bool followRefs);
private:
	
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
	IndirectMapping indMap;

	/** Trailer dictionary for this revision.
	 *
	 * This dictionary contains all information for current revision. When
	 * revision is changed this has to be initialized before
	 * initRevisionSpecific method is called. First initialization is done in
	 * constructor.
	 * <br>
	 * Trailer is direct object so it can't be obtained through
	 * getIndirectProperty method. Only way how to do change, add or delete its
	 * members (which is strongly unrecomended unless you know what you are
	 * doing) is to use XRefWriter interface.
	 */
	boost::shared_ptr<CDict> trailer;
		
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
	 * It is association of page position with CPage instance.
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

	// TODO returned outlines list

	/** Intializes revision specific stuff.
	 * 
	 * Cleans up all internal structures which may depend on current discourage  revision.
	 * This includes indirect mapping and pageList (all pages are invalidated).
	 * After clean up is ready, initializes trailer field from Xref trailer xpdf
	 * Object. docCatalog field is initialized same way.
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
	ModeController* modeController;

	/** Empty constructor.
	 *
	 * This constructor is disabled, because we want to prevent uninitialized
	 * instances.
	 * <br>
	 * If you want to create instance, please use static factory method 
	 * getInstance.
	 */
	CPdf ():mode(ReadOnly), modeController(NULL){};

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
	 * It is no available outside class, because we whant to prevent
	 * deleting instances without control.
	 * <br>
	 * Instance can be destryed by close method (which destroyes it
	 * using this destructor).
	 */
	~CPdf();
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
	static CPdf * getInstance(const char * filename, OpenMode mode);

	/** Closes pdf file.
	 * @param saveFlag Flag which determine whether to save before close
	 *	(parameter may be omited and false is used by default).
	 *
	 * Destroyes CPdf instance in safe way. Instance MAY NOT be used
	 * after this method is called.
	 */
	int close(bool saveFlag=false);

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
	ModeController* getModeController()const
	{
		return modeController;
	}

	/** Gets change field value.
	 *
	 * @return true if there are some new changes, false otherwise.
	 */
	bool isChanged()const
	{
		return change;
	}
	
	/** Sets mode controller.
	 * @param ctrl Mode controller implementator (if NULL, controller
	 * will be disabled).
	 *
	 */
	void setModeController(ModeController* ctrl)
	{
		modeController = ctrl;
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
	boost::shared_ptr<IProperty> getIndirectProperty(IndiRef ref);

	/** Adds new indirect object.
	 * @param prop Original property.
	 * @param followRefs Flag for reference properties in complex type
	 * handling (default value is false).
	 *
	 * This method is responsible for clear indirect object creation and
	 * also safe indirect object copying between different documents (properties
	 * from different CPdf instances).
	 * <p>
	 * Implementation details:
	 * <ul>
	 * <li> Value has to be proper for indirect property. This means that it
	 * can't be reference and should be indirect object (if it belongs to pdf).
	 * <li> Property from same pdf is added immediately using
	 * registerIndirectProperty method.
	 * <li> Property from different pdf instance is added using addProperty
	 * method.
	 * <li> followRefs flag controls how referencies in property from different
	 * file should be handled. If value is true, also all referenced properties
	 * are added, otherwise just reserves referencies in this pdf for them and
	 * doesn't care for their values (they may be initialized later). Note that
	 * followRefs may produce rather big bunch of copying (e. g. copying page
	 * dictionary will probably cause copy of all page tree object hierarchy,
	 * because each page contains also reference to its parent and parent
	 * contains all its kids and so on). 
	 * <li> If followRefs is false, doesn't dereference reference values at all,
	 * so just for those directly accessbile are reserved new referencies.
	 * <li> Initializes ResolvedRefStorage storage for addProperty to prevent
	 * endless loops for cyclyc property structures (such as mentioned above in
	 * page dictionary case) and to guarantee that same referencies are mapped
	 * to same in this pdf (@see subsReferencies). 
	 * <li> Method will fail with exception if pdf is in read only mode.
	 * </ul>
	 *
	 * @see registerIndirectProperty
	 * @see addProperty
	 * @throw ReadOnlyDocumentException if mode is set to ReadOnly or we are in
	 * older revision (where no changes are allowed).
	 * @return Reference of new property (see restriction when given
	 * property is reference itself).
	 */ 
	IndiRef addIndirectProperty(boost::shared_ptr<IProperty> prop, bool followRefs=false);

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
	void changeIndirectProperty(boost::shared_ptr<IProperty> prop);
	
	/** Saves changes to pdf file.
	 * @param newRevision Flag for new revision creation.
	 *
	 * If revision is 0 (the newest one), uses XRefWriter::saveChanges method to
	 * store changes. Parameter newRevision has precisely the same meaning.
	 * For more implementation information @see XRefWriter
	 *
	 * <br>
	 * Method will fail if actual revision is greater than 0.
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
	 * Actual document state stands for all obejcts from all revistions until 
	 * current one. This means that kind of fork of document is done. Be
	 * careful, because actual changes are not considered (in revision 0),
	 * because they are not really part of document (you have to save them as
	 * new revision at first and than clone will contain also actual changes).
	 * <p>
	 * <b>Usage</b>
	 * <pre>
	 * save(fileName)
	 * </pre>
	 * this should be used if we want to do fork or copy of document. This may 
	 * be helpful when we want to make changes in older revision, what is not
	 * possible normaly:
	 * <pre>
	 * cpdf->changeRevision(5);					// change to 5th revision
	 * cpdf->save("5th_revision_clone.pdf");	// creates copy of everything
	 * 											// until 5th revision
	 * </pre>
	 * Then you can open this document and make changes inside.
	 * <p>
	 * NOTE: this method doesn't check whether target of FILE handle is same
	 * file as one used in StreamWriter, so caller must take care about this to
	 * prevent unexpecting problems (overwriting currently used data in stream).
	 *
	 */
	void clone(FILE * fname)const;

	/** Returns document catalog for property access.
	 * 
	 * @return Document catalog dictionary wrapped by smart pointer (using
	 * shared_ptr from boost library).
	 */
	boost::shared_ptr<CDict> getDictionary()
	{
		return docCatalog;
	}

	/** Returns trailer dictionary.
	 *
	 * Returns const Dictionary pointer to prevent doing changes to it.
	 * 
	 * @return Trailer dictionary wrapped by smart pointer (using shared_ptr
	 * from boost library).
	 */
	boost::shared_ptr<const CDict> getTrailer()const
	{
		return trailer;
	}
		
	/** Inserts exisitng page.
	 * @param page Page used for new page creation.
	 * @param pos Position where to insert new page.
	 *
	 * Inserts deep copy of given page at given position. If position is
	 * greater than page count, it is added after last page. Storing to 0
	 * position is same as if pos parameter was 1. All pages behind pos
	 * are renumbered.
	 * <br>
	 * Method may fail if page at given position is ambiguous. This means that 
	 * it is not possible to get position of page reference in its parent Kids
	 * array due to multiple occurance in Kids array. @see getNodePosition
	 * 
	 * <br>
	 * Insertion never causes ambigues page tree, because deep copy of given
	 * page is added as new indirect property and so it has different indirect
	 * reference as original one.
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
	 * followRefs set to true (in addIndirectProperty method).
	 *
	 * @throw ReadOnlyDocumentException if mode is set to ReadOnly or we are in
	 * older revision (where no changes are allowed).
	 * @throw AmbiguesPageTreeException if page can't be inserted to given
	 * position because of ambiguous page tree.
	 */
	boost::shared_ptr<CPage> insertPage(boost::shared_ptr<CPage> page, size_t pos);

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
	size_t getPagePosition(boost::shared_ptr<CPage> page)const;

	/** Returnes page count.
	 *
	 * Try to use pageCount field value (if it is valid) or gets value from Page
	 * tree root node (sets new value of pageCount field).
	 * 
	 *
	 * @throw MalformedFormatExeption if page count can't be found or it has bad
	 * type (CPdf instance is almost unusable if this is not correct).
	 * @return Number of pages which are accessible.
	 */
	unsigned int getPageCount()const;

	// page iteration methods
	// =======================

	/** Returns page at given position.
	 * @param pos Position (starting from 0).
	 *
	 * Search for page position (uses find method). If page dictionary is found,
	 * compares it with already returned pages list. If this page was already
	 * returned and is valid (its position is same as given one), returns this 
	 * CPage. Otherwise creates new CPage instance, adds it to the list and 
	 * return.
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
	 * position.
	 * <br>
	 * Uses getPage method.
	 *
	 * @throw PageNotFoundException if page can't be found (given page is last
	 * one or given page can't be found).
	 * @return CPage instance wrapped by smart pointer.
	 */ 
	boost::shared_ptr<CPage> getNextPage(boost::shared_ptr<CPage> page)const;

	/** Returns previous page.
	 * @param page Pointer to the page.
	 *
	 * Returns page which is before given one. Uses getPagePosition to get actual
	 * position.
	 * <br>
	 * Uses getPage method.
	 *
	 * @throw PageNotFoundException if page can't be found (given page is last
	 * one or given page can't be found).
	 * @return CPage instance wrapped by smart pointer.
	 */
	boost::shared_ptr<CPage> getPrevPage(boost::shared_ptr<CPage> page)const;

	/** Checks for next page.
	 * @param page Page to check.
	 *
	 * @return true if getNextPage() method returns doesn't throw
	 * PageNotFoundException, false otherwise. 
	 */
	bool hasNextPage(boost::shared_ptr<CPage> page)const
	{
		try
		{
			getNextPage(page);
			// next page was successful
			return true;
		}catch(PageNotFoundException & e)
		{
			// nextPage failed
			return false;
		}
		return true;
	}
	
	/** Checks for previous page.
	 * @param page Page to check.
	 *
	 * @return true if getPrevPage() method returns doesn't throw
	 * PageNotFoundException. 
	 */
	bool hasPrevPage(boost::shared_ptr<CPage> page)const
	{
		try
		{
			getPrevPage(page);
			// prev page was successful
			return true;
		}catch(PageNotFoundException & e)
		{
			// getPrevPage failed
			return false;
		}
		return true;
	}

	/** Returns last page.
	 * 
	 * Calls getPage(pages.size()-1).
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
	 * Mode is ReadOnly for all older version than last available and
	 * last depends on mode set in creation time.
	 */
	OpenMode getMode() const
	{
		// mode is used only if we are in the newest revision, otherwise we are
		// in ReadOnly
		return (!xref->getActualRevision())?mode:ReadOnly;
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
	 * newest revision. An older revision has higher number than
	 * newer one.
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
	 * @param revisionNum Revision number (the newest is 0).
	 *
	 * Delegates to xref field and reinitializes all internal structures
	 * which are revision specific.
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
	 * see XRefWriter::getRevisionCount
	 */
	size_t getRevisionsCount()
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
	void getOutlines (Container& cont)
	{
		// Clear outline container
		cont.clear ();
		
		// topleve outline if any
		boost::shared_ptr<CDict> toplevel;
		try {
			
			toplevel = utils::getCDictFromDict (docCatalog, "Outlines");
			
		}catch (ElementNotFoundException&)
		{
			kernelPrintDbg (debug::DBG_DBG, "No outlines");
			return;
		}
		assert (toplevel);

		utils::getAllChildrenOfPdfObject (toplevel, cont);
	}
};


// helper methods
namespace utils 
{
	
/** Helper method to find page at certain position.
 * @param pdf Pdf instance where to search.
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
 * from pdf-&gtgetProperty("Pages"). This is indirect reference (according
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
boost::shared_ptr<CDict> findPageDict(const CPdf & pdf, boost::shared_ptr<IProperty> pagesDict, size_t startPos, size_t pos);

/** Gets position of given node.
 * @param pdf Pdf where to examine.
 * @param node Node to find (CRef or CDict instances).
 *
 * Start searching of given node from root of the page tree (document catalog
 * Pages field). Uses recursive searchTreeNode function for searching and
 * provides just error handling wrapper to this method.
 * <br>
 * Prefer to use this function instead of searchTreeNode if you are not sure you
 * know what you are doing.
 *
 * @throw PageNotFoundException
 * @throw ElementBadTypeException
 * @throw MalformedFormatExeption
 * @throw AmbiguousPageTreeException if node position can't be determined bacause
 * of page tree ambiguity (see searchTreeNode for more information).
 * @return Node position.
 */
size_t getNodePosition(CPdf & pdf, boost::shared_ptr<IProperty> node);

/** Checks if given child is descendant of node with given reference.
 * @param pdf Pdf where to resolv referencies.
 * @param parent Reference of the parent.
 * @param child Dictionary of page(s) node.
 *
 * Checks if child's Parent field has same reference as given one as parent
 * parameter. If yes then child dictionary is descendant of node with parent
 * reference. If not, dereference child's parent and continues in recursion
 * using derefenced direct parent as new child for recursion call.
 * <br>
 * NOTE: this method doesn't perform any checking of parameters.
 *
 * @throw MalformedFormatExeption if child contains Parent field which doesn't
 * point to the dictionary.
 * @return true If given child belongs to parent subtree, false otherwise.
 */
bool isDescendant(CPdf & pdf, IndiRef parent, boost::shared_ptr<CDict> child);

/** Checks whether file content is encrypted.
 * @param pdf Pdf instance to check.
 * @param filterName Name of the filter used for encryption (set only if
 * content is encrypted).
 *
 * Checks for Encrypt entry in documents trailer. If it is present, it means
 * that content is enctypted and so tries to get FilterName entry from Encrypt
 * dictionary (if filterName is non NULL).
 * 
 * @return true if file content is encrypted, false otherwise.
 */
bool isEncrypted(CPdf & pdf, std::string * filterName);

/** Returns cobjects from given reference property.
 * @param refProp Reference property (must be pRef typed).
 *
 * Gets reference value from property and dereferences indirect object from it.
 * Uses refProp's pdf for dereference.
 * Checks target object for given template pType and if it matches casts to
 * given CType and returns.
 *
 * @throw ElementBadTypeException if refProp is not CRef instance or indirect
 * object is not CType instance.
 * @return CType instance wrapped by shared_ptr smart pointer.
 */
template<typename CType, PropertyType pType>
boost::shared_ptr<CType> getCObjectFromRef(boost::shared_ptr<IProperty> refProp)
{
	// REMARK
	// This helper has to be here because of gcc template manipulation
	// (cobjecthelpers.h can't include from this header file)
	
	if(!isRef(refProp))
		throw ElementBadTypeException("");
	
	// gets reference value and dereferences indirect object
	IndiRef ref;
	IProperty::getSmartCObjectPtr<CRef>(refProp)->getValue(ref);
	boost::shared_ptr<IProperty> indirect_ptr=refProp->getPdf()->getIndirectProperty(ref);
	if(indirect_ptr->getType() != pType)
		throw ElementBadTypeException("");
	return IProperty::getSmartCObjectPtr<CType>(indirect_ptr);
}


} // namespace utils

} // namespace pdfobjects

#endif // __CPDF_H__
