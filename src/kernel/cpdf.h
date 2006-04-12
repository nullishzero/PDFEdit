// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80

/*
 * Created:  01/28/2006 03:48:14 AM CET

 * $RCSfile$
 *
 * $Log$
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
#include "xrefwriter.h"


// =============================================================================
namespace pdfobjects {

namespace utils {

/**
 * Indirect referencies comparator.
 *
 * Handles comparing of Indirect referencies.
 *
 * FIXME find some more proper place for the class
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

/** CPdf special object.
 *
 * This class is responsible for pdf document maintainance. 
 * <p>
 * <b>Instancing</b><br>
 * Public constructor is not available and instances can be created on by 
 * static factory getInstance method. Also no public destructor is available 
 * and instance can be destroyed only by close method.
 * <p>
 * <b>Open mode</b><br>
 * Each document may be open in several modes. Each open mode brings specific
 * handling of document manipulation. Open mode for document is set only in 
 * constructor and can't be changed during CPdf instance life cycle. (TODO link
 * to OpenMode)
 * <p>
 * TODO indirect properties describtion
 * TODO CXref usage describtion
 * TODO produced objects describtion
 * TODO ...
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
	 */
	enum OpenMode {Advanced, ReadWrite, ReadOnly};
	
protected:

	/**
	 * Indirect properties mapping type.
	 */
	typedef std::map<IndiRef, boost::shared_ptr<IProperty>, utils::IndComparator> IndirectMapping;

	/** Consolidates page tree.
	 * @param interNode Intermediate node dictionary under which change has
	 * occured.
	 *
	 * Consolidation stards by getting number of direct pages under given node.
	 * To do so, goes through Kids array from node and if element refers to Page
	 * dictionary use 1 count and for Pages dictionary uses Count field. Sum is
	 * used to be compared with node's Count field value. If these numbers match
	 * immediatelly returns. Otherwise distribute count change to its parent by
	 * calling this method to Parent dictionary.
	 * <br>
	 * Second step of consolidation checks whether all children from Kids array
	 * have Parent set to given one. If not changes (or add if field is missing)
	 * it the reference to given node.
	 * <p>
	 * <b>Implementation notes</b>:
	 * <ul>
	 * <li>
	 * This method should be called when some change occures in page tree. Given
	 * parameter stands for dereferenced dictionary of intermediate node under
	 * which change occures. Change may be:
	 * <ul>
	 * 		<li>Kids array element was deleted, added
	 * 		<li>Kids array element has changed its value
	 * </ul>
	 * <li> Doesn't perform any parameter checking. Relay on correct value. When
	 * some error occures, exception is thrown.
	 * <li> Changes Count field of each intermediate node if neccessary.
	 * <li> Sets Parent field of direct children, if not set correctly.
	 * <li> Doesn't go deeply in to children
	 * <li> If page tree was consistent before change under this interNode 
	 * occured, it will be consistent after this consolidation too (NOTE all
	 * children of given node must be correct too).
	 * </ul>
	 */
	void consolidatePageTree(boost::shared_ptr<CDict> interNode);
	
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

	// FIXME just for debug
	friend void observerHandler(CPdf & cpdf, boost::shared_ptr<IProperty> oldValue, boost::shared_ptr<IProperty> newValue);

	/** Page tree observer instance.
	 * This instance is inicialized in constructor.
	 */
	//PageTreeObserver pageTreeObserver;
private:
	
	/**************************************************************************
	 * Revision specific data
	 *************************************************************************/
	
	/** Change flag.
	 * @see isChanged
	 */
	bool change;
	
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
	PageList pageList;

	// TODO returned outlines list

	/** Intializes revision specific stuff.
	 * 
	 * trailer field must be initialized correctly before this method can be
	 * called.
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
	
	/** File handle for pdf file.
	 *
	 * This field is initialized when pdf file is open (in constructor) and
	 * destroyed in close method.
	 */
	FILE * pdfFile;

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
	IModeController* modeController;

#ifdef DEBUG
// debuging workaround to enable testing
public:
#endif
	
	/** Empty constructor.
	 *
	 * This constructor is disabled, because we want to prevent uninitialized
	 * instances.
	 * <br>
	 * If you want to create instance, please use static factory method 
	 * getInstance.
	 */
	CPdf ()/*:pageTreeObserver(this)*/{};

	/** Initializating constructor.
	 * @param stream Stream with data.
	 * @param file File handle for stream.
	 * @param openMode Mode for this file.
	 *
	 * Creates XRefWriter and initialize xref field with it.
	 * TODO initializes also other internal structures.
	 */
	CPdf(StreamWriter * stream, FILE * file, OpenMode openMode);
	
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
	 * initialization is done, also internal structures of kernel are
	 * initialized.
	 *
	 * @throw PdfOpenException if file open fails.
	 * @return Initialized (and ready to be used) CPdf instance.
	 */
	static CPdf * getInstance(const char * filename, OpenMode mode);

	/** Closes pdf file.
	 * @param saveFlag Flag which determine whether to save before close
	 *	(parameter may be omited and false is used by default).
	 *
	 * Destroyes CPdf instance in safe way. Instant MUST NOT be used
	 * after this method is called.
	 */
	int close(bool saveFlag=false);

	/** Returns pointer to cross reference table.
	 *
	 * This is pointer to XRef subtype of XRefWriter type field. It
	 * contains actual state of xref table. 
	 * If any of xpdf code is going to be used besides kernel, this
	 * can be safely used.
	 * <br>
	 * This method will return same pointer each time it is called.
	 *
	 * @return Pointer to XRefWriter field casted to XRef super type.
	 */
	XRef * getXRef()
	{
		return (XRef *)xref;
	}
       
	/** Returns actually used mode controller.
	 *
	 * @return IModeController implementator or NULL, if no mode 
	 * controller is used.
	 */
	IModeController* getModeController()
	{
		return modeController;
	}

	/** Checks whether there is something to save.
	 *
	 * If actual revision is 0, checks change field, otherwise returns false,
	 * because it is not possible to have changes in older revision.
	 *
	 * @return true if there are some new changes, false otherwise.
	 */
	bool isChanged()const
	{
		if(xref->getActualRevision())
			return false;

		// the newest revision may be changed
		return change;
	}
	
	/** Sets mode controller.
	 * @param ctrl Mode controller implementator (if NULL, controller
	 * will be disabled).
	 *
	 */
	void setModeController(IModeController* ctrl)
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
	 *
	 * This method is responsible for clear indirect object creation and
	 * also safe indirect object copying between different documents.
	 * <p>
	 * Implementation details:
	 * <br>
	 * If given property is reference, than two things may happen. It 
	 * depends on from where property is. If it is from same pdf, than
	 * nothing is created and just reference is returned. Otherwise
	 * dereference to get indirect object and process with following
	 * <br>
	 * If property comes from different pdf and it is complex type,
	 * all children are checked if they are reference. If yes, this
	 * method is called recursively on them. Normal values are untouched.
	 * This is because referencies from different pdf may point on 
	 * something in this pdf and so mass could be produced.
	 * <br>
	 * When property value is prepared, xpdf Object is created from
	 * its content and new reference is reserved. After this is done
	 * object can be be fully registered using xref::change method.
	 * Reference is now definitive and so it is returned.
	 * <p>
	 * <b>REMARKS</b>: 
	 * <ul>
	 * <li>When object from different cpdf is given as parameter, 
	 * allways copies whole property subtree.
	 * <li>CNull properties are also registered.
	 * </ul>
	 *
	 * @return Reference of new property (see restriction when given
	 * property is reference itself).
	 */ 
	IndiRef addIndirectProperty(boost::shared_ptr<IProperty> prop);

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
	 *
	 * @throw TODO 
	 */
	void save(bool newRevision=false);

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
		
	/** Inserts exisitng page.
	 * @param page Page used for new page creation.
	 * @param pos Position where to insert new page.
	 *
	 * Adds deep copy of given page before given position. If position is
	 * greater than page count, it is added after last page. Storing to 0
	 * position is same as if pos parameter was 1.
	 * <br>
	 * NOTE: Insertion before some position means storing at the position and
	 * move everything starting with this position before storing.
	 * <br>
	 * Method may fail if page at given position is ambigues. This means that it
	 * is not possible to get position of page reference in its parent Kids
	 * array due to multiple occurance in Kids array. @see getNodePosition
	 *
	 * <br>
	 * This method triggers pageList and page tree consolidation same as if the
	 * change has been done manulualy.
	 *
	 * @throw TODO for ambigues.
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
	 * @throw TODO for ambigues.
	 */
	void removePage(size_t pos);

	/** Returns absolute position of given page.
	 * @param page Page to look for.
	 * 
	 * Returns actual position of given page. If given page hasn't been returned
	 * by this CPdf instance or it is no longer available, exception is thrown.
	 * <br>
	 * NOTE: assume CPage implements == operator correctly
	 *
	 * @throw PageNotFoundException if given page is not recognized by CPdf
	 * instance.
	 */
	int getPagePosition(boost::shared_ptr<CPage> page);

	/** Returnes page count.
	 *
	 * Checks Pages field in document catalog. If it has Page type, returns 1
	 * otherwise return Pages' count field value.
	 *
	 * @throw MalformedFormatExeption if page count can't be found or it has bad
	 * type (CPdf instance is almost unusable if this is not correct).
	 * @return Number of pages which are accessible.
	 */
	unsigned int getPageCount();

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
	boost::shared_ptr<CPage> getPage(size_t pos);

	/** Returns first page.
	 *
	 * Calls getPage(1).
	 *
	 * @return CPage instance wrapped by smart pointer.
	 */
	boost::shared_ptr<CPage> getFirstPage()
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
	boost::shared_ptr<CPage> getNextPage(boost::shared_ptr<CPage> page);

	/** Returns previous page.
	 * @param Pointer to the page.
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
	boost::shared_ptr<CPage> getPrevPage(boost::shared_ptr<CPage> page);

	/** Checks for next page.
	 * @param page Page to check.
	 *
	 * @return true if getNextPage() method returns doesn't throw
	 * PageNotFoundException, false otherwise. 
	 */
	bool hasNextPage(boost::shared_ptr<CPage> page)
	{
		try
		{
			getNextPage(page);
			// next page was successful
		}catch(PageNotFoundException & e)
		{
			// nextPage failed
			return false;
		}
	}
	
	/** Checks for previous page.
	 * @param page Page to check.
	 *
	 * @return true if getPrevPage() method returns doesn't throw
	 * PageNotFoundException. 
	 */
	bool hasPrevPage(boost::shared_ptr<CPage> page)
	{
		try
		{
			getPrevPage(page);
			// prev page was successful
		}catch(PageNotFoundException & e)
		{
			// getPrevPage failed
			return false;
		}
	}

	/** Returns last page.
	 * 
	 * Calls getPage(pages.size()-1).
	 *
	 * @return CPage instance wrapped by smart pointer.
	 */
	boost::shared_ptr<CPage> getLastPage()
	{
		return getPage(getPageCount());
	}

	// Outlines methods
	// =================

	/** Returns all top-level outlines.
	 * @param container Template type parameter which will contain outline
	 * pointers (must be allocated and support push_back and clear methods).
	 *
	 * To get whole outline hierarchy, use COutline instances (contains 
	 * information about children).
	 * <br>
	 * NOTE: In first step clears container (calls clear method) and then
	 * fills it with top-level outline instances (uses push_back method).
	 *
	 */
	template<typename T> void getOutlines(T * container)
	{
		if(!container)
		{
			// TODO handle
		}

		// clears actual content
		container->clear();
		
		// get outlines from Outlines field
		// TODO figure out
	}

	/** Removes top-level outline.
	 * @param outline Outlines to remove.
	 *
	 * Removes also all children.
	 */
	void removeOutline(COutline * /*outline*/)
	{
		// actualize outlines
		// remove from Outlines dictionary (in document catalog)
		// change Outline object
		// destroy outline object
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
		// 
		return (!xref->getActualRevision())?mode:ReadOnly;
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

	/** Changes revision to given one.
	 * @param revisionNum Revision number (the newest is 0).
	 *
	 * see XRefWriter::changeRevision
	 */
	void changeRevision(revision_t /*revisionNum*/)
	{
		// set revision xref->changeRevision
		// call initRevisionSpecific
		// TODO kind of notification
		// TODO what has to be dellocated ?
	}

	/** Returns number of available revisions.
	 *
	 * see XRefWriter::getRevisionCount
	 */
	size_t getRevisionsCount()
	{
		return xref->getRevisionCount();
	}
};

} // namespace pdfobjects

#endif // __CPDF_H__
