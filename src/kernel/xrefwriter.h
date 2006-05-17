// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80
#ifndef _XREFWRITER_
#define _XREFWRITER_

/* 
 * $RCSfile$
 *
 * $Log$
 * Revision 1.24  2006/05/17 19:40:07  hockm0bm
 * * getRevisionEnd
 *         - signature changed - uses position where to start searching
 *           not using current revision information
 *         - position is restored before return
 *         - method is protected now
 * * getRevisionSize method added
 *
 * Revision 1.23  2006/05/16 18:01:00  hockm0bm
 * pdf field value may be NULL - makes sense for stand alone XRefWriter instances
 *
 * Revision 1.22  2006/05/13 22:16:31  hockm0bm
 * * memory leak removed
 *         - changeObject didn't deallocate object returned from CXRef (previously changed value)
 *         - pdfWriter is deallocated in destructor
 * * ~XRefWriter destructor added
 * * log messages improved
 * * uses RefState
 *
 * Revision 1.21  2006/05/09 20:08:31  hockm0bm
 * * collectRevisions
 * 	- considers also xref streams
 * 	- checks for endless loop caused by wrong Trailer::Prev values (cycle)
 * * checkLinearized new parameter xref needed for proper parsing
 *
 * Revision 1.20  2006/05/08 22:21:56  hockm0bm
 * * isLinearized method added
 *         - linearized field added
 * * checkLinearized is called in constructor
 * * saveChages prints WARNING if file is linearized
 * * changeRevision throws an exception if file is linearized
 * * collectRevisions doesn't collect if file is linearized
 *
 * Revision 1.19  2006/05/08 21:24:33  hockm0bm
 * checkLinearized function added
 *
 * Revision 1.18  2006/05/08 20:17:42  hockm0bm
 * * key words removed from here
 * * new model of data writing
 *         - uses IPdfWriter interface for implementation specific writer
 *         - uses OldStylePdfWriter by default
 *         - new implementator can be set by setPdfWriter method
 *         - saveChanges collects object from changedStorage and uses IPdfWriter
 *           to write them to the stream so it is independed on implementation
 *
 * Revision 1.17  2006/05/06 08:40:20  hockm0bm
 * knowsRef delegates to XRef::knowsRef if not in the newest revision
 *
 * Revision 1.16  2006/04/27 18:21:09  hockm0bm
 * * deallocation of Object corrected
 * * changeRevision
 *         - saves revision
 *
 * Revision 1.15  2006/04/23 15:09:34  hockm0bm
 * note for getRefisionEOF method in documentation
 *
 * Revision 1.14  2006/04/23 13:13:20  hockm0bm
 * * getRevisionEnd method added
 *         - to get end of stream contnet for current revision
 * * cloneRevision method added
 *         - to support cloning of pdf in certain revision
 *
 * Revision 1.13  2006/04/19 06:00:23  hockm0bm
 * * changeRevision - first implementation (not tested yet)
 * * collectRevisions -  first implementation (not tested yet)
 * * minor changes in saveChanges
 *
 * Revision 1.12  2006/04/17 19:53:34  hockm0bm
 * documentation update
 *
 * Revision 1.11  2006/04/13 18:08:49  hockm0bm
 * * releaseObject method removed
 * * readOnly mode removed - makes no sense in here
 * * documentation updated
 * * contains information about CPdf
 *         - because of pdf read only mode
 * * buildXref method added
 * * XREFROWLENGHT and EOFMARKER macros added
 *
 * Revision 1.10  2006/04/12 17:52:25  hockm0bm
 * * saveChanges method replaces saveXRef method
 *         - new semantic for saving changes
 *              - temporary save
 *              - new revision save
 *         - storePos field added to mark starting
 *           place where to store changes
 *         - saveChanges moves with storePos if new
 *           revision is should be created
 * * findPDFEof function added
 * * StreamWriter in place of BaseStream
 *         - all changes will be done via StreamWriter
 *
 * Revision 1.9  2006/03/23 22:13:51  hockm0bm
 * printDbg added
 * exception handling
 * TODO removed
 * FIXME for revisions handling - throwing exeption
 *
 * Revision 1.8  2006/03/10 18:07:14  hockm0bm
 * reserveRef method added
 * createObject uses reserveRef
 * one FIXME in createObject - commented
 *
 * Revision 1.7  2006/03/08 12:09:24  misuj1am
 *
 * -- commented out unused arguments
 *
 * Revision 1.6  2006/03/07 18:29:27  hockm0bm
 * createObject returns CXref::createObject return value
 *
 * Revision 1.5  2006/03/06 18:18:55  hockm0bm
 * compilable changes - each module is compilable now
 * each object in pdfobjects namespace
 * more comments
 * cpdf - most of methods are just comments how to implement
 *
 * Revision 1.4  2006/02/28 19:52:10  hockm0bm
 * preparation for revision handling (kind of pseudocode)
 *	   - RevisionStorage type and field
 *	   - collectRevision method (pseudocode)
 *	   - getRevisionCount method
 *	   - getActualRevision method  (pseudocode)
 *	   - changeRevision method
 * methods depedning on changes are not available in older revisions
 *	   - error handling not solved yet - exception, return value?
 * saving new revision (pseudocode)
 *
 * Revision 1.3  2006/02/13 22:03:53  hockm0bm
 * Implementation moved to cc file
 *
 * Revision 1.2  2006/02/12 18:30:07  hockm0bm
 * first implementation (compileable) - not for real use
 *
 * Revision 1.1  2006/01/27 19:02:15  hockm0bm
 * writer class for the CXref class
 *
 *
 */

/**
 * @file xrefwriter.h
 *
 * Implementation of CXRef wrapper to enable making changes to the document.
 */

#include "static.h"
#include "cxref.h"
#include "streamwriter.h"
#include "pdfwriter.h"


namespace pdfobjects
{

namespace utils
{

/** Checks whether given stream is linearized.
 * @param stream Pdf stream to read (from the file begin).
 * @param xref XRef instance.
 * @param ref Pointer to reference where to set object and generation number.
 *
 * Searches first indirect object in the stream from the begining and if it is
 * dictionary, checks whether it contains Linearized version entry. If so
 * returns true and if given ref is not NULL, sets object and generation number.
 * Otherwise just returns false and doesn't care for ref parameter.
 *
 * @return true if first indirect object is Linearized dictionary, false
 * otherwise.
 */
bool checkLinearized(StreamWriter & stream, XRef * xref, Ref * ref);

} // end of namespace utils

	
/** CXref writer class.
 *
 * This wrapper of the CXref class enables:
 * <ul>
 * <li> making changes to the CXref - overrides protected methods and makes 
 *	some checking. (changeTrailer, changeObject, createObject methods).
 * <li> controling checking routines - mode field controls level of checking.
 *	(getMode, setMode methods).
 * <li> manipulating revisions of document. Revisions are available through
 *	their numbers (0 for newest and getRevisionCount-1 for latest). 
 *	Changes can be made only in newest revision. Older revisions are only 
 *	read-only. (getRevisionCount, getActualRevision, changeRevision 
 *	methods).
 * <li> enables writing content to the stream and duplication of current revision
 * to other file. Uses implemetation independant way for content storing which
 * may be changed by runtime (implementator of IPdfWriter abstract class).
 * </ul>
 * <br>
 * Each public method which produces content changes checks whether associated 
 * pdf instance is not in ReadOnly mode and if it is, throws an exception to 
 * prevent from changes.
 */
class XRefWriter:public CXref
{
public:
	/** Mode for XRefWriter.
	 * This controls behaviour. Following values are possible:
	 * <ul>
	 * <li>easy - no checking is performed when some change is done.
	 * <li>paranoid - type safety and reference existance is checked.
	 * </ul>
	 */
	enum writerMode {easy, paranoid};
private:
	/** Mode for checking. */
	writerMode mode;

	/** Pdf instance which maintains this xref writer.
	 *
	 * Instance is used to get higher level information whether making changes
	 * is allowed.
	 * <br>
	 * Value is initialized in constructor and doesn't change during instance
	 * life cycle.
	 */
	const CPdf * pdf;
	
	/** Actual revision.
	 *
	 * Latest revision is represented by 0 and each older one has bigger
	 * number.
	 * <br>
	 * Instance is created in 0 revision number and this can be changed by
	 * changeRevision method.
	 */
	unsigned revision;

	/** Type for revision storage.
	 *
	 * Each element stands for revision with number same as index in array.
	 * The newest revision is first element (with index 0). Value stored in
	 * array is stream offset where xref section for such revision starts.
	 */
	typedef std::vector<size_t> RevisionStorage;

	/** Array of all revisions.
	 * This array contains everytime at least one element. It is 
	 * initialized in constructor from parsed information and new 
	 * elements are appended when saveXref is called with newRevision flag.
	 */
	RevisionStorage revisions;

	/** File offset for write changes.
	 *
	 * This offset is used as file position where to start writing changes. It
	 * is used by saveChanges method which can move this offset depending on
	 * saving mode (it is moved if changes are saved as new revision).
	 * <br>
	 * The value is initialized by constructor to point to the PDF end of file
	 * marker %%EOF. This invariant is allways kept.
	 */
	size_t storePos;

	/** Pdf writer implementator.
	 *
	 * Uses OldStylePdfWriter by default. This can be changed by setPdfWriter
	 * method.
	 */
	utils::IPdfWriter * pdfWriter;
	
	/** Flag for linearized pdf content.
	 * This value is set in constructor and it tells whether file is linearized.
	 * Some operations are not implemented for those files (e. g. revision
	 * handling, ... - it is always described in method if it is problem)
	 */
	bool linearized;
	
	/* Empty constructor.
	 *
	 * It's not available to prevent uninitialized instances.
	 * Sets mode to paranoid.
	 */
	XRefWriter():CXref(), mode(paranoid), pdf(NULL), revision(0), linearized(false)
	{
	}
protected:

	/** Checking for paranoid mode.
	 * @param ref Reference of object.
	 * @param obj Object to check.
	 *
	 * Checks if we are in paranoid mode. If not immediately returns with
	 * success.
	 * Otherwise checks if given reference exists (uses knowsRef method).
	 * Then checks whether replacing with given object is type safe (uses
	 * typeSafe method).
	 *
	 * @return true if all checks are ok, false otherwise.
	 */
	bool paranoidCheck(::Ref ref, ::Object * obj);

	/** Collects all revisions information.
	 *
	 * If linearized is set to true, immediately returns with warning message.
	 * Otherwise stores position of xref section start to the revisions storage.
	 * Parses Trailer dictionary and gets Prev field value. If not present,
	 * assumes no more revisions are available. Otherwise stores that position
	 * to revisions storage as later revision and continues same way.
	 */
	void collectRevisions();

	/** Returns end of current revision offset. 
	 * @param xrefStart Stream offset of xref section start.
	 *
	 * Finds startxref from given position and returns offset to the first
	 * byte after xref offset number line or end of stream position if startxref
	 * has not been found.
	 * <p>
	 * <b>Implementation notes:</b>
	 * <br>
	 * This method assumes that revision has been added by incremental update
	 * and no object from this revision can be behind this position. This
	 * assumption is not fullfilled specially for linearized pdf documents.
	 *
	 * @return Offset immediately after last information for this revision.
	 */
	size_t getRevisionEnd(size_t xrefStart)const;

public:
	/** Initialize constructor with file stream writer.
	 * @param stream File stream with pdf content.
	 * @param _pdf Pdf instance which maintains this instance (may be also NULL,
	 * which means that instance is standalone).
	 *
	 * Sets mode to paranoid. Sets file to FILE handle from stream. Collects 
	 * all revisions (uses collectRevisions method) and sets storePos to the 
	 * %%EOF position.
	 * <br>
	 * Allocates OldStylePdfWriter for pdfWriter field.
	 * <br>
	 * Stream is supplied to CXref constructor.
	 *
	 * @throw MalformedFormatExeption if XRef creation fails (instance is
	 * unusable in such situation).
	 */
	XRefWriter(StreamWriter * stream, CPdf * _pdf);

	/** Destrucrtor.
	 *
	 * Deallocates pdfWriter field if it is non NULL.
	 */
	~XRefWriter()
	{
	using namespace debug;
	
		kernelPrintDbg(DBG_DBG, "");
		if(pdfWriter)
			delete pdfWriter;
	}
	
	/** Sets new pdf writer implementator.
	 * @param writer Implementation of IPdfWriter (must be non NULL).
	 *
	 * Sets writer (if parameter is non NULL) and returns current
	 * implementation. If parameter is NULL, just returns current implementator.
	 * <br>
	 * Given parameter has to be allocated by new operator, because it is
	 * deallocated in destructor by delete operator.
	 *
	 * @return Previous pdf writer implemetator (if not NULL, caller is
	 * responsible for deallocation).
	 */
	utils::IPdfWriter * setPdfWriter(utils::IPdfWriter * writer);

	/** Returns linearized flag value.
	 * 
	 * @return true if file is linearized, false otherwise.
	 */
	bool isLinearized()const
	{
		return linearized;
	}
	
	/** Initialize constructor with cache.
	 * @param stream Stream with file data.
	 * @param c Cache instance.
	 *
	 * Delegates to CXref constructor with the stream and cache parameter.
	 * Sets mode to paranoid. Collects all revisions (uses collectRevisions
	 * method).
	 *
	 * @throw MalformedFormatExeption if XRef creation fails (instance is
	 * unusable in such situation).
	 */
	/* FIXME uncoment when cache is available
	XRefWriter(FileStreamWriter * stream, ObjectCache * c):CXRef(stream, c){};
	*/
	
	/** Gets mode.
	 * 
	 * @return Actualy set mode.
	 */
	writerMode getMode()const
	{
		return mode;
	}

	/** Sets mode.
	 * @param mode Mode to set.
	 *
	 */
	void setMode(writerMode _mode)
	{
		this->mode=_mode;
	}
	
	/** Inserts new object.
	 * @param num Number of object.
	 * @param gen Generation number of object.
	 * @param obj Instance of object.
	 *
	 * If revision is 0 (most recent), delegates to the to the 
	 * CXref::insertObject method. Otherwise deny to make chage, because
	 * it is not possible to do changes to a older release.
	 * <br>
	 * If mode is set to paranoid, checks the reference existence and after
	 * type safety. If tests are ok, operation is permitted otherwise 
	 * operation fails.
	 *
	 * @throw ReadOnlyDocumentException if no changes can be done because actual
	 * revision is not the newest one or if pdf is in read-only mode.
	 * @throw ElementBadTypeException if mode is paranoid and paranoidCheck
	 * method fails for obj.
	 */ 
	void changeObject(int num, int gen, ::Object * obj);

	/** Changes trailer entry.
	 * @param name Name of the entry.
	 * @param obj New value.
	 *
	 * If revision is 0 (most recent), delegates to the 
	 * CXref::changeTrailer method. Otherwise deny to make chage, because
	 * it is not possible to do changes to a older release.	 
	 * <br>
	 * If mode is set to paranoid, checks the reference existence and after
	 * type safety. If tests are ok, operation is permitted otherwise 
	 * operation fails.
	 * 
	 * @throw ReadOnlyDocumentException if no changes can be done because actual
	 * revision is not the newest one or if pdf is in read-only mode.
	 * @return Previous value of object or 0 if previous revision not
	 * available (new name value pair in trailer).
	 */
	::Object * changeTrailer(const char * name, ::Object * value);
	
	/** Saves changed objects and new xref and trailer.
	 * @param newRevision Flag controlling new revision creation.
	 *
	 * Checks all objects which are changed (in CXref::changeStorage) and stores
	 * them from actual storePos file offset.
	 * Also append new xref table for changed objects and finally new trailer is
	 * added. Trailer's Prev field is set to contain file offset to previous
	 * xref position.
	 * <p>
	 * <b>Revision handling</b>:
	 * <br>
	 * Method gets also newRevision flag parameter which says whether to save
	 * changes as new revision. If new revision is created, storePos is moved
	 * after stored data (more precisely after new trailer) and CXref super type
	 * is forced to reopen (CXref::reopen method is called TODO link) to handle
	 * new revision creation.
	 * Otherwise storePos is not moved and all objects from changeStorage are 
	 * kept as they are. This means that default behaviour doens't have any 
	 * influence on internal structure. 
	 * <br>
	 * By default no new revision is set. This implies that each call of this
	 * method overwrittes previous save (from same storePos).
	 * <br>
	 * Use default behaviour if you want to be sure that you don't lose your
	 * changes and create new revision if you want to have certain set of
	 * changes grouped together (e. g. when changes have some significancy).
	 * <br>
	 * NOE: if document is linearized prints warning but continue with saving
	 * even if output may brake linearization rules and content is still marked
	 * as linearized (Linearized dictionary is kept because we never change
	 * original content and all changes are strictly appended). Method should be
	 * called with linearized documents very carefully (especially when braking
	 * linearization can confuse pdf reader - e. g. xpdf doesn't care for
	 * linearized pdfs special handling and reads file allways from the end). 
	 *
	 * @throw ReadOnlyDocumentException if no changes can be done because actual
	 * revision is not the newest one.
	 */
	void saveChanges(bool newRevision=false);
	
	/** Changes revision of document.
	 * @param revNumber Number of the revision.
	 *
	 * Jumps to the given revision. 0 stands for newest revision, older 
	 * have higher number. The oldest revision has getRevisionCount()-1
	 * number.
	 * <p>
	 * XRefWriter doesn't enable to make changes if revision is not 0 (most
	 * actual). This means that all methods producing changes are invalid 
	 * and produces error when called.
	 * <br>
	 * This is because branching is not implementable in PDF structure.
	 * 
	 * @throw OutOfRange if revNumber doesn't stand for any known revisions.
	 * @throw NotImplementedException if pdf content is linearized.
	 */ 
	void changeRevision(unsigned revNumber);

	/** Returns actual revision.
	 *
	 * @return Revision number.
	 */
	unsigned getActualRevision()const
	{
		return revision;
	}

	/** Returns count of revisions.
	 *
	 * @return Number of revisions.
	 */
	size_t getRevisionCount()const
	{
		// revisions contains all revisions
		return revisions.size();
	}

	/** Clones content of stream until end of current position.
	 * @param file File handle where to copy content.
	 * 
	 * Clone contains everything from stream starting from 0 position until 
	 * getRevisionEnd.
	 *
	 * @see XRefWriter::getRevisionEnd for limitations.
	 */
	void cloneRevision(FILE * file)const;

	/** Returns size of given revision.
	 * @param rev Revision to examine.
	 * @param includeXref Flag controling whether also xref section with trailer
	 * should be also considered.
	 *
	 * Gets size of data used for given revision. This means number of bytes
	 * starting from previous revision (or file start if no previous is exists)
	 * until start of given one's xref section if includeXref is false or end of
	 * whole given revision (return value from getRevisionEnd) if flag is true.
	 * <br>
	 * This implies that getRevisionSize with default behaviour returns number
	 * of bytes used for data of given revision meanwhile 
	 * getRevisionSize(true) returns whole size of revision.
	 * 
	 * @see XRefWriter::getRevisionEnd for limitations.
	 *
	 * @return number of bytes used for current revision (see for includeXref
	 * paramter specific behaviour).
	 */
	size_t getRevisionSize(unsigned rev, bool includeXref=false)const;

	/**********************************************************************
	 *
	 * Reimplementation of CXref methods, which may depend on changed
	 * objects, which shouldn't be available in an later revision.
	 *
	 *********************************************************************/

	/** Fetches indirect object.
	 *
	 * This is just wrapper for CXref::fetch implementation
	 * and only checks wheter we are in the newest revision and if so
	 * delegates to the CXref::fetch method (this is because newest
	 * revision may contain changes). Otherwise delegates to the
	 * XRef::fetch method.
	 * 
	 */ 
	virtual ::Object * fetch(int num, int gen, ::Object * obj)
	{
		// newest revision may contain changes, so uses
		// CXref implementation
		if(!revision)
			return CXref::fetch(num, gen, obj);

		// we are in later revision, we have to use only XRef
		// implementation
		return XRef::fetch(num, gen, obj);
	}
		
	/** Checks if given reference is known.
	 * @param ref Reference to check.
	 *
	 * Checks if revision is 0 (the newest one) delegates to CXref
	 * implementation, because there can be also new referencies.
	 * Otherwise searches only in XRef::entries (only referencies
	 * from document).
	 *
	 * @see RefState
	 * @return Reference state.
	 */
	virtual RefState knowsRef(::Ref ref)
	{
		// if we are in newest revision, delegates to CXref
		if(!revision)
			return CXref::knowsRef(ref);
				
		// otherwise use XRef directly
		return XRef::knowsRef(ref);
	}
	
	/** Checks if given reference is known.
	 * @param ref Indirect reference to check.
	 *
	 * Gets xpdf Ref value and delegates to knowsRef(Ref).
	 *
	 * @see RefState
	 * @return Reference state.
	 */
	virtual RefState knowsRef(IndiRef ref)
	{
		::Ref xpdfRef={ref.num, ref.gen};
		// otherwise use XRef directly
		return knowsRef(xpdfRef);
	}

	/** Registers new reference.
	 *
	 * This is just wrapper for CXref::reference method.
	 * Only thing which is done here is that revision field
	 * is checked and if revision is 0 (most recent), delegates 
	 * to the to the CXref::reserveRef. Otherwise deny to create, 
	 * because it is not possible to do changes to a older release.
	 *
	 * @throw ReadOnlyDocumentException if no changes can be done because actual
	 * revision is not the newest one or if pdf is in read-only mode.
	 */
	virtual ::Ref reserveRef();
	
	/** Creates new indirect object.
	 *
	 * This is just wrapper for CXref::createObject method.
	 * Only thing which is done here is that revision field
	 * is checked and if revision is 0 (most recent), delegates 
	 * to the to the CXref::createObject. Otherwise deny to make create, 
	 * because it is not possible to do changes to a older release.
	 *
	 * @throw ReadOnlyDocumentException if no changes can be done because actual
	 * revision is not the newest one or if pdf is in read-only mode.
	 */
	virtual ::Object * createObject(::ObjType type, ::Ref * ref);
	
	/** Returns number of indirect objects.
	 *
	 * If revision is 0 (the newest revision) delegates to the 
	 * CXref::getNumObjects (because new object may have been created),
	 * otherwise delegates to XRef::getNumObjects.
	 * 
	 * @return number of objects.
	 */
	virtual int getNumObjects() 
	{ 
		if(!revision)
			return CXref::getNumObjects();

		return XRef::getNumObjects();
	}
};

} // end of pdfobjects namespace
#endif
