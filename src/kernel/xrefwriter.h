#ifndef _XREFWRITER_
#define _XREFWRITER_

/* 
 * $RCSfile$
 *
 * $Log$
 * Revision 1.5  2006/03/06 18:18:55  hockm0bm
 * compilable changes - each module is compilable now
 * each object in pdfobjects namespace
 * more comments
 * cpdf - most of methods are just comments how to implement
 *
 * Revision 1.4  2006/02/28 19:52:10  hockm0bm
 * preparation for revision handling (kind of pseudocode)
 *         - RevisionStorage type and field
 *         - collectRevision method (pseudocode)
 *         - getRevisionCount method
 *         - getActualRevision method  (pseudocode)
 *         - changeRevision method
 * methods depedning on changes are not available in older revisions
 *         - error handling not solved yet - exception, return value?
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
#include <vector>
#include "cxref.h"

namespace pdfobjects
{
        
/** Mode for XRefWriter.
 * This controls behaviour. Following values are possible:
 * <ul>
 * <li>easy - no checking is performed when some change is done.
 * <li>paranoid - type safety and reference existance is checked.
 * </ul>
 */
enum XRefWriterMode {readOnly, easy, paranoid};

/** CXref writer class.
 *
 * This wrapper of the CXref class enables:
 * <ul>
 * <li> making changes to the CXref - overrides protected methods and makes 
 *      some checking. (changeTrailer, changeObject, releaseObject, 
 *      createObject methods).
 * <li> controling checking routines - mode field controls level of checking.
 *      (getMode, setMode methods).
 * <li> manipulating revisions of document. Revisions are available through
 *      their numbers (0 for newest and getRevisionCount-1 for latest). 
 *      Changes can be made only in newest revision. Older revisions are only 
 *      read-only. (getRevisionCount, getActualRevision, changeRevision 
 *      methods).
 * </ul>
 *
 * <br>
 * TODO provide undo
 */
class XRefWriter:public CXref
{
        /** Mode for checking. */
        XRefWriterMode mode;

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
         * This is array containing file offset for startxref for each 
         * revision. First element is newest revision.
         */
        typedef std::vector<size_t> RevisionStorage;

        /** Array of all revisions.
         * This array contains everytime at least one element. It is 
         * initialized in constructor from parsed information and new 
         * elements are appended when saveXref is called.
         */
        RevisionStorage revisions;

        /* Empty constructor.
         *
         * It's not available to prevent uninitialized instances.
         * Sets mode to paranoid.
         */
        XRefWriter():CXref(), mode(paranoid), revision(0){}
protected:

        /** Checking for paranoid mode.
         * @param ref Reference of object.
         * @param obj Object to check.
         *
         * Checks if we are in paranoid mode. If not immediately returns.
         * Otherwise checks if given reference exists (uses knowsRef method).
         * Then replacing with given object is type safe.
         * <br>
         * TODO error handling exceptions
         */
        bool paranoidCheck(Ref ref, ::Object * obj);

        /** Collects all revisions information.
         *
         */
        void collectRevisions()
        {
                // clear revisions if non empty
                // store lastXrefPos revisions
                // 
                // LOOP (start with trailerDict)
                // IF dict->prev
                        // jump to prev in stream (prev XRef start)
                        // append position to the revisions
                        // lookup trailer dict
                        // dict = parse trailer
                // ELSE ENDLOOP
        }
public:
        /** Initialize constructor with stream.
         * @param stream Stream fith file data.
         *
         * Delegates to CXref with stream parameter.
         * Sets mode to paranoid. Collects all revisions (uses collectRevisions
         * method).
         */
        XRefWriter(BaseStream * stream):CXref(stream), mode(paranoid)
        {
                collectRevisions();
        };

        /** Initialize constructor with cache.
         * @param stream Stream with file data.
         * @param c Cache instance.
         *
         * Delegates to CXref constructor with the stream and cache parameter.
         */
        /* FIXME uncoment when cache is available
        XRefWriter(BaseStream * stream, ObjectCache * c):CXRef(stream, c){};
        */
        
        /** Gets mode.
         * 
         * @return Actualy set mode.
         */
        XRefWriterMode getMode()const
        {
                return mode;
        }

        /** Sets mode.
         * @param mode Mode to set.
         *
         */
        void setMode(XRefWriterMode mode)
        {
                this->mode=mode;
        }
        
        /** Releases object.
         * @param num Number of object.
         * @param gen Generation number of object.
         *
         * If revision is 0 (most recent), delegates to the 
         * CXref::releaseObject method. Otherwise deny to make chage, because
         * it is not possible to do changes to a older release (TODO how to 
         * announce).
         * <br>
         * TODO provide undo information
         */
        void releaseObject(int num, int gen);

        /** Inserts new object.
         * @param num Number of object.
         * @param gen Generation number of object.
         * @param obj Instance of object.
         *
         * If revision is 0 (most recent), delegates to the to the 
         * CXref::insertObject method. Otherwise deny to make chage, because
         * it is not possible to do changes to a older release (TODO how to 
         * announce).
         * <br>
         * If mode is set to paranoid, checks the reference existence and after
         * type safety. If tests are ok, operation is permitted otherwise 
         * operation fails.
         * <br>
         * TODO provide undo information
         *
         */ 
        void changeObject(int num, int gen, ::Object * obj);

        /** Changes trailer entry.
         * @param name Name of the entry.
         * @param obj New value.
         *
         * If revision is 0 (most recent), delegates to the 
         * CXref::changeTrailer method. Otherwise deny to make chage, because
         * it is not possible to do changes to a older release (TODO how to 
         * announce).
         * <br>
         * If mode is set to paranoid, checks the reference existence and after
         * type safety. If tests are ok, operation is permitted otherwise 
         * operation fails.
         * <br>
         * TODO provide undo information
         * 
         */
        ::Object * changeTrailer(char * name, ::Object * value);
        
        /** Saves xref with changes and creates new revision.
         * @param f File where to write (has to be open).
         *
         * This method has different semantic according actual revision.
         * Changes to the document may be done only in the newest revision.
         * So if revision is 0, saves all changed objects to the file and 
         * appends new xref with new trailer to the given file.
         * <br>
         * In all later revisions saves all content until actual revision
         * to the file (and forget everything behind). This enables to create
         * new file with actual revision as newest and so making changes is
         * available in that file. (This is kind of document fork). 
         * Nevertheless caller must be aware that given file is not one actualy
         * used, otherwise document may be damaged.
         * <br>
         * TODO some other kind of temporal saving - not new revision
         */
        void saveXref(FILE * f)
        {
                // IF revision == 0
                        // put unstored changed objects
                        // discard new storage (all new initialized are in 
                        //      changed now)
                        // select changed entries as stored
                        // put xref entries
                        // set trailer prev to lastXrefPos
                        // put trailer dictionary
                        // update startxref
                        // put new element to the revisions array (to the 
                        //      begining)
                        // parse xref table again
                // ELSE
                        // saves everything until this revision
                        // f shouldn't be same as target of the XRef::stream
        }
        
        /** Changes revision of document.
         * @param revNumber Number of the revision.
         *
         * Jumps to the given revision. 0 stands for newest revision, older 
         * have higher number. The oldest revision has getRevisionCount()-1
         * number.
         * <p>
         * XRefWriter is changed to read-only mode if revision is not 0 (most
         * actual). This means that all methods producing changes are invalid 
         * and produces error when called.
         * <br>
         * This is because branching is not implementable in PDF structure.
         * saveXref method will forget everithing behind this revision, so it
         * is possible to make changes in arbitrary revision with restriction, 
         * that change in older revision is possible only in separate file.
         */ 
        void changeRevision(unsigned revNumber)
        {
               // constrains check
               // empty XRef internals (entries, trailer, TODO find out ...)
               // jump to the revisions[revNumber] position
               // parse XRef from this position
               // discard cache
               // sets revision field
        }

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

        /**********************************************************************
         *
         * Reimplementation of CXref methods, which may depend on changed
         * object, which shouldn't be available in an later revision.
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
         * @return true if reference is known, false otherwise.
         */
        virtual bool knowsRef(Ref ref)
        {
                // if we are in newest revision, delegates to CXref
                if(!revision)
                        return CXref::knowsRef(ref);
                                
               // has to be found in entries
               if(entries[ref.num].type==xrefEntryFree)
                       return false;
               // object number is ok, so also gen must fit
               return entries[ref.num].gen==ref.gen;
        }

        /** Creates new indirect object.
         *
         * This is just wrapper for CXref::createObject method.
         * Only thing which is done here is that revision field
         * is checked and if revision is 0 (most recent), delegates 
         * to the to the CXref::createObject. Otherwise deny to make create, 
         * because it is not possible to do changes to a older release (TODO 
         * how to announce).
         */
        virtual ::Object * createObject(ObjType type, Ref * ref)
        {
                if(!revision)
                        CXref::createObject(type, ref);

                // TODO handle
        };
        
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
