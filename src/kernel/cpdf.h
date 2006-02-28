/*
 * $RCSfile$
 *
 * $Log$
 * Revision 1.8  2006/02/28 22:41:41  hockm0bm
 * Scratch of the implementation - not compileable
 * Most of functions described with kind of pseudocode
 * Waiting for CObject clarification (to implement stuff)
 *
 *
 */

/*
 * =====================================================================================
 *        Filename:  cpdf.h
 *     Description: CPdf class definition.
 *         Created:  01/28/2006 03:48:14 AM CET
 *          Author:  jmisutka (), 
 *         Changes: 2006/1/28 added mapping support
 *                              2006/1/30 tested objToString () -- ok
 * =====================================================================================
 */


#ifndef __CPDF_H__
#define __CPDF_H__

#include <map>
#include <set>
#include <string>
#include "xpdf/Object.h"
#include "iproperty.h"
#include "xrefwriter.h"

// =====================================================================================
namespace pdfobjects {

class CPage;
class COutline;

/** CPdf special object.
 *
 * This class is responsible for pdf document maintainance.
 * 
 */
class CPdf// : public CDict
{
public:
        // NOTE: this declaration has to be here, because mode field is private and
        // so type has to be declared

        /** Mode for file opening.
         *
         * Possible values:
         * <ul>
         * <li>Advanced - content of PDF can be changed and no special checking
         *      is peformed. So this brings the biggest control over content
         *      but may lead to total content corruption.
         * <li>ReadWrite - content of PDF can be changed.
         * <li>ReadOnly - content can't be changed.
         * </ul>
         */
        enum OpenMode {Advanced, ReadWrite, ReadOnly};
        
protected:
       /**
        * Comparator class for two mapped items.
        */
       class ObjComparator
       {
        public:
                /* FIXME maybe this can be handled by less<T> template */
                bool operator() (const Object* one, const Object* two) const
                {// one < two -- true,     one >= two -- false
                        return ((unsigned int)one < (unsigned int)two);
                };
       };      

       /**
        * Indirect mapping Comparator class for two mapped items.
        */
       class IndComparator
       {
       public:
               bool operator() (const IndiRef one, const IndiRef two) const
               {
                       if (one.num == two.num)
                               return (one.gen < two.gen);
                       else
                               return (one.num < two.num);
               };
        };      

        /**
         * Mapping between (x)pdf Object <--> IProperty
         *
         * We have to know which IProperty is associated with an Object.
         * We use this when implementing CObject::release (), CObject::~CObject.
         * 
         * This is because (x)pdf Object can traverse deeper in its nesting simply
         * by looking at Object pointers. We use Object to hold information about 
         * our properties because it has been well tested. 
         * In spite of its very non-object implementation, we have decided 
         * to use it because of its tremendous functionality. So we have to 
         * make the mapping between IProperty and Object by ourselves.
         * 
         * REMARK:
         * We can make this mapping simply with pointers, because we know
         * that no other can destroy, reallocate our objects, just CObject and
         * its children. CXref and/or GUI/CUI are not permitted to do this.
         */
        typedef std::map<const Object*,const IProperty*, ObjComparator> ObjectMapping;

        /**
         * Mapping between indirect objects <--> IProperty.
         *
         * This is essential when we want to access an indirect object from 
         * pRef object. We know only the id and gen number.
         * 
         */
        typedef std::map<const IndiRef,const IProperty*, IndComparator> IndirectMapping;
private:
        /** Mapping between Object <--> IProperty*. 
         * It is necessary when releasing objects. 
         */
        ObjectMapping objMap;
        
        /** Mapping between Id + Gen <--> IProperty*. 
         * It is necessary when accessing indirect objects. 
         */
        IndirectMapping indMap;

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
         */
        XRefWriter * xref;

        /** Open mode of document.
         */
        enum OpenMode mode;

        /** Pdf trailer dictionary.
         *
         * Source of information about versions.
         */
        ::Dict * trailer;

        /** Document catalog dictionary.
         *
         * Contains properties of whole PDF document. It is source of
         * information about pages, outlines, TODO ...
         */
        ::Dict * catalog;

        /** Storage type for pages.
         */
        typedef std::vector<CPage *> PageStorage;
        
        /** Page objects array.
         * All page objects are stored in this array, to be easily found.
         */
        PageStorage pages;

        /** Storage type for outlines.
         */
        typedef std::vector<COutline *> OutlineStorage;
        
        /** Outline objects array.
         *
         * All top-level outlines are stored in this array.
         */
        OutlineStorage outlines;
        
        /** Empty constructor.
         *
         * This constructor is disabled, because we want to prevent uninitialized
         * instances.
         * <br>
         * If you want to create instance, please use static factory method 
         * getInstance.
         */
        CPdf (){}//:CDict(NULL, sPdf){};

        /** Initializating constructor.
         * @param stream Stream with data.
         * @param file File handle for stream.
         * @param openMode Mode for this file.
         *
         * Creates XRefWriter and initialize xref field with it.
         * TODO initializes also other internal structures.
         */
        CPdf(BaseStream * stream, FILE * file, OpenMode openMode);

        /** Destructor.
         * 
         * It is no available outside class, because we whant to prevent
         * deleting instances without control.
         * <br>
         * Instance can be destryed by close method (which destroyes it
         * using this destructor).
         */
        ~CPdf();

        /** Fills pages array with page objects.
         * @param pageNode Dictionary representing page tree node.
         *
         * This method should be called with top pageNode and it recursively
         * goes through page tree and inserts just Page leafs to the array.
         *
         * <br>
         * TODO error handling
         */
        void fillPages(::Dict * pageNode);

        /** Fills outlines array with outlines objects.
         * TODO
         */
        void fillOutlines(::Dict * outlineNode);
        
        /** Intializes revision specific stuff.
         * @param trailer Trailer dictionary for revision.
         * 
         * Gets document catalog and initializes pages and outlines arrays.
         * <br>
         * NOTE: Assumes that xref corresponds to given trailer.
         */
        void initRevisionSpecific(::Dict * trailer)
public:
        /** Factory method for CPdf instances.
         * @param filename File name with pdf content (if null, new document 
         *      will be created).
         * @param mode Mode to open file.
         *
         * This is only way how to get instance of CPdf type. All necessary 
         * initialization is done, also internal structures of kernel are
         * initialized.
         *
         * @return Initialized (and ready to be used) CPdf instance.
         */
        static CPdf * getInstance(const char * filename, OpenMode mode);

        /** Closes pdf file.
         * @param save Flag which determine whether to save before close
         *      (parameter may be omited and false is used by default).
         * Destroyes CPdf instance in safe way. Instant MUST NOT be used
         * after this method is called.
         */
        int close(bool save=false);

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
        
        /**
         * Returns IProperty associated with (x)pdf object if any.
         *
         * @param  o    (x)pdf object.
         * @return Null if there is no mapping, IProperty* otherwise.
         */
        IProperty* getExistingProperty (const Object* o) const;
        IProperty* getExistingProperty (const IndiRef& pair) const;

        /**
         * Saves relation between (x)pdf object and IProperty*. 
         *
         * @param o      (x)pdf object.
         * @param ip IProperty that will be mapped to Object o.
         */
        void setObjectMapping (const Object* o, const IProperty* ip);
        void setIndMapping (const IndiRef& pair, const IProperty* ip);

        /**
         * Deletes relation between (x)pdf object and IProperty*. 
         *
         * @param o      (x)pdf object.
         * @param ip IProperty that will be mapped to Object o.
         */
        void delObjectMapping (const Object* o);
        void delIndMapping (const IndiRef& pair);

        /** Saves CPdf content (whole document).
         * @param fname File name where to store.
         *
         * Saves actual content to given file. If fname is NULL, uses
         * original file used for instance creation.
         * <br>
         * TODO distinguish saving and creating new revision.
         */
        int save(const char * fname)
        {
                // call xref->change for all changed objects
                // call xref->saveXref
        }

        /** Creates new empty page.
         * @param pos Position where to insert page.
         *
         */
        CPage * createPage(int pos)
        {
                // create new object xref->createObject
                // initialize with default values
                // update PageTree dictionary
                // update pages array
                // call xref->change to new page and page tree
        }

        /** Inserts exisitng page.
         * @param page Page used for new page creation.
         * @param pos Position where to insert new page.
         *
         * Creates new page and uses initialize all stuff according given page.
         */
        CPage * insertPage(const CPage * page, int pos)
        {
                // create page deep copy (if from other file, also referencies)
                // similar as create page
        }

        /** Removes page from given position.
         * @param pos Position of the page.
         */
        int removePage(int pos)
        {
                // get page object
                // change PageTree object
                // update pages array
                // release this object (xref->release())
                // deallocate object (checks number of referencies...)
        }

        /** Returns absolute position of given page.
         * @param page Page to look for.
         * 
         * NOTE: assume CPage implements == operator correctly
         */
        int getPagePosition(const CPage * page)const
        {
                PageStorage::const_iterator iter=pages.begin();
                for(int i=0; iter!=pages.end(); iter++,i++)
                        if(*iter == page)
                                return i;

                // page not found
                // TODO handle error
        }

        /** Returnes page count.
         */
        unsigned int getPageCount() const
        {
                return pages.size();
        }

        // page iteration methods
        // =======================

        /** Returns page at given position.
         * @param pos Position (starting from 0).
         *
         * TODO error handling description
         * @return Pointer to CPage instance.
         */
        CPage * getPage(int pos)
        {
                if(pos<0 || (unsigned long)pos>=pages.size())
                {
                        // out of range error
                        // TODO handle
                }

                return pages[pos];
        }

        /** Returns first page.
         *
         * Calls getPage(0).
         *
         * @return Pointer to the first page instance.
         */
        CPage * getFirstPage()
        {
                return getPage(0);
        }

        /** Returns next page.
         * @param page Pointer to the page.
         *
         * Returns page which is after given one.
         * TODO error handling description.
         *
         * @return CPage pointer.
         */ 
        CPage * getNextPage(CPage * page)
        {
                size_t pos=getPagePosition(page)+1;
                
                if(pos>=pages.size())
                {
                        // no more pages error
                        // TODO handle
                }

                return pages[pos];
        }

        /** Returns previous page.
         * @param Pointer to the page.
         *
         * Returns page which is before given one.
         * TODO error handling description.
         *
         * @return CPage pointer.
         */
        CPage * getPrevPage(CPage * page)
        {
                int pos=getPagePosition(page)-1;
                if(pos<0)
                {
                        // no previous page
                        // TODO handle
                }

                return pages[pos];
        }

        /** Returns last page.
         * 
         * Calls getPage(pages.size()-1).
         *
         * @return CPage pointer.
         */
        CPage * getLastPage()
        {
                return getPage(pages.size()-1);
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
                
                // copies elements (just pointers)
                OutlineStorage::iterator iter;
                for(iter=outlines.begin(); iter!=outlines.end(); iter++)
                        container.push_back(*iter);
        }

        /** Removes top-level outline.
         * @param outline Outlines to remove.
         *
         * Removes also all children.
         */
        void removeOutline(COutline * outline)
        {
                // actualize outlines
                // remove from Outlines dictionary (in document catalog)
                // change Outline object
                // destroy outline object
        }

        /** Creates new top-level outline.
         * TODO parameters (at least name, position a target)
         */
        COutline * createOutline()
        {
                // creates new object
                // initialize with default outline information
                // update Outlines dictionary in document catalog
                // update outlines array
        }


        // Version handling and work around
        // =================================

        /** Returnes mode of this version.
         *
         * Mode is READ_ONLY for all older version than last available and
         * last depends on mode set in creation time.
         */
        OpenMode getMode() const
        {
                // 
                return (!xref->getActualRevision())?mode:READ_ONLY;
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
        void changeRevision(revision_t revisionNum)
        {
                // set revision xref->changeRevision
                // call cleanupRevisionSpecific
                // call initRevisionSpecific
                // TODO kind of notification
                // TODO what has to be dellocated ?
        }

        /** Returns number of available revisions.
         *
         * see XRefWriter::getRevisionCount
         */
        size_t getRevisionsCount()const
        {
                xref::getRevisionCount();
        }
};

} // namespace pdfobjects

#endif // __CPDF_H__
