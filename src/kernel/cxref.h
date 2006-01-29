#ifndef __CPAGE_H__
#define __CPAGE_H__

/** 
 * $RCSfile$
 *
 * $Log$
 * Revision 1.4  2006/01/29 21:18:15  hockm0bm
 * minor changes for compilation (TODOs and FIXME comments)
 *
 *
 */

#include"iproperty.h"
#include"utils/objectstorage.h"
//#include"utils/cache.h"
#include"xpdf/XRef.h"

using namespace pdfobjects;

/** Adapter for xpdf XRef class.
 * 
 * This class has responsibility to transparently (same way as XRef do) provide
 * objects synchronized with changes.
 * <br>
 * Class works in three layers:
 * <ul>
 * <li>Maintaining layer - registers changes (in values or new objects), which
 *    uses ObjectStorage instances to know which objects are changed 
 *    (changedStorage field), which objects are new (newStorage field) and 
 *    which are released (releasedStorage field).  
 * <li>xpdf layer - delegation to original xpdf implementation of XRef.  
 * <li>caching layer - holds objects which where required to prevent getting
 *    objects from xpdf (which has to parse stream each time indirect object
 *    is required) or maintaining layer (which has to search in structures with
 *    not so effective searching capabilities). This layer is not mandatory and
 *    CXref can work also without it.
 * </ul>
 * Each request to XRef inherited interface, which manipulates with object which
 * can change, asks Maintaining layer at first. If it is not able to to get
 * required object, xpdf layer is use (delegates to super type).
 * If object is changed and caching is enabled, cache entry has to be discarded.
 * <br>
 * Methods which produces changes are protected to prevent changing in gui, if
 * kernel want's to make changes, it should use XrefWriter subclass which 
 * delegates functionality to inherited protected methods.
 * <p>
 * We have prepared XRef implementation to be virtual, so there is no problem to 
 * use CXref instance as XRef in rest of xpdf code. Initialization of xref from
 * file (respectively from basestream) is done only by xpdf layer (if no changes 
 * are done to the file - Maintaining layer doesn't contain any additional 
 * information than after CXref initialization). 
 * <br>
 * Because, in fact, all object which can be changed (we are meaning change 
 * value inside Object instance) has to be indirect Object, obj and gen number 
 * are used as identificators, when objects are stored to the ObjectStorage. So
 * when someone wants to change object value using changeObject method, he has
 * to suply also obj and gen numbers.
 * Only one exception in pdf format is trailer, which is not indirect object and
 * this can be changed using changeTrailer method. This method can change 
 * entries in trailer. If user whants to change indirect object values stored 
 * in trailer, it can be done in standard way as any other objects.
 * <p>
 * Objects returned by this interface are always copies of original object 
 * stored inside and changes made to them are not visible through this interface
 * until changeObject or changeTrailer is called.
 * 
 */
class CXref: public XRef
{
private:
        //ObjectCache * cache=NULL;       /**< Cache for objects. */
protected:        
        ObjectStorage changedStorage;   /**< Object storage for changed 
                                             objects. */
        ObjectStorage newStorage;       /**< Object storage for newly 
                                             created objects.*/
        ObjectStorage releasedStorage;  /**< Object storage for release 
                                             objects. */

        /** Empty constructor.
         *
         * This constructor is protected to prevent uninitialized instances.
         * We need at least to specify stream with data.
         */
        CXref(): XRef(0){};
        
        /** Registers change in given object addressable through given num and 
         * gen.
         * @param ref Object reference identificator.
         * @param instance Instance of object value (must be direct value,
         * not indirect reference).
         *
         * Discards object from cache and stores it to the changedStorage.
         * Method should be called each time when object has changed its value.
         * Object value is copied not use as it is (creates deep copy).
         */
        void changeObject(Ref ref, Object * instance)
        {
                // TODO
        }

        /** Releases  given object addressable through given num and gen.
         * @param ref Object reference identificator.
         * @param instance Instance of object value (must be direct value,
         * not indirect reference).
         *
         * Discards object from cache, removes it from the changedStorage (if 
         * presented) and stores it to the releasedStorage.
         * Method should be called if indirect reference was removed from 
         * compound value (e.g. changed by other indirect reference or direct
         * value).
         * Object value is copied not use as it is (creates deep copy).
         */
        void releaseObject(Ref ref, Object * instance)
        {
                // TODO
        }

        /** Changes entry in trailer dictionary.
         * @param name Name of the value.
         * @param value Value to be set.
         *
         * Makes changes to the trailer and if value should be set as indirect
         * value, num and gen has to be non negative. In such case calls also
         * changeObject method.
         * <br>
         * If given value is indirect reference, only change in trailer 
         * dictionary is made.
         */
        void changeTrailer(string name, Object * value)
        {
                // TODO
        }

        /** Tries to find object in storages.
         * @param ref Object reference identificator.
         * @param obj Object to be filled by value.
         *
         * Searches cache, newStorage and changedStorage in given order. If
         * object associated with given reference is found, fills given obj
         * to be initialized with found value (uses Object::copy method).
         *
         * @return obj initialized with found value or 0.
         */
        Object * probe(Ref ref, Object * obj)
        {
                Object * o;
                
                // at first try cache if one is used
                /* FIXME uncoment when the cache is ready
                if(cache)
                {
                        if((o=cache->get(ref)))
                                return o->copy(obj);
                }
                */
                
                // tries newStorage
                if((o=newStorage.get(ref)))
                        return o->copy(obj);
                
                // tries changedStorage
                if((o=changedStorage.get(ref)))
                        return o->copy(obj);

                // nothing has been found
                return 0;
        }
public:
        /** Initialize constructor.
         * @param stream Stream with file data.
         *
         * Delegates to XRef constructor with same parameter.
         */
        CXref(BaseStream * stream):XRef(stream)
        {
        }

        /** Initialize constructor with cache.
         * @param stream Stream with file data.
         * @param c Cache instance.
         *
         * Delegates to XRef constructor with the stream parameter and
         * sets cache instance.
         */
        /*
        CXref(BaseStream * stream, ObjectCache * c):XRef(stream), cache(c)
        {
                // FIXME: uncoment when cache is ready 
        }
        */
        
        /** Destructor.
         *
         * Destroys cache.
         */
        virtual ~CXref()
        {
                /* FIXME: uncoment when cache is ready.
                if(cache)
                        delete cache;
                 */
        }
        
        /** Creates new xpdf indirect object.
         * @param type Type of the object.
         * @param ref Structure where to put object num and gen (if null, 
         * nothing is set).
         * 
         * Object instance is registered to the internal structures and so can 
         * be accessible throught its object number and generation number which
         * is set the ref parameter. 
         * Implementation will initialized Object at least with type and value:
         * 
         * <ul>
         * <li>numeric types with zero
         * <li>strings allocated and empty
         * <li>compound types (array, dict) allocated with no members
         * </ul>
         * Returned object MUST NOT be deallocated by user.
         *
         * @return Object instance with given type or 0 if not able to create.
         */
        virtual Object * createObject(ObjType type, Ref * ref)
        {
                // TODO get new num, add to the newStorage and return
                return 0;
        }

        /** Gets value associated with name in trailer.
         * @param name Name of the entry.
         *
         * If value is indirect reference (according specification almost all 
         * compound entries), reference is returned, so normal xref interface
         * can be used to ask for value.
         * @return copy of the object value.
         */
        virtual Object * getTrailerEntry(const string * name)
        {
                // TODO work with the trailerDict - uses fetch if indirect
                return 0;
        }

        // TODO reimplementation of inherited methods
        // three steps
        //      cache                   -|
        //      maintained objects      -|-- probe
        //      original implementation ---- Xref
        // this is only for methods working with changeable objects
        // e.g. getRootNum doesn't have to be change at all

        virtual Object *getCatalog(Object *obj) 
        {
                // uses this fetch implementation to get actual catalog
                return fetch(rootNum, rootGen, obj); 
        }

        virtual Object *fetch(int num, int gen, Object *obj)
        {
                // identificator of the catalog 
                Ref objId={rootNum, rootGen};
                
                // check if catalog is not changed and if so
                // uses changed one
                if((obj=probe(objId, obj)))
                        return obj;

                // not found - has to use XRef original implementation
                return XRef::fetch(num, gen, obj);
        }

        // Return the document's Info dictionary (if any).
        virtual Object *getDocInfo(Object *obj)
        {
                // TODO

                return XRef::getDocInfo(obj);
        }
        
        virtual Object *getDocInfoNF(Object *obj)
        {
                // TODO

                return XRef::getDocInfoNF(obj);
        }

        // Return the number of objects in the xref table.
        virtual int getNumObjects() 
        { 
                // TODO consider storages

                return XRef::getNumObjects();
        }

        // Return the offset of the last xref table.
        virtual Guint getLastXRefPos() 
        { 
                // TODO can change - when older version is used

                return XRef::getLastXRefPos();
        }
        
        //
        // Get end position for a stream in a damaged file.
        // Returns false if unknown or file is not damaged.
        virtual GBool getStreamEnd(Guint streamStart, Guint *streamEnd)
        {
                // TODO figure out

                return XRef::getStreamEnd(streamStart, streamEnd);
        }

        /* TODO figure out - this should not be changed (find out who does
         * use it
         */
        /*
        // Direct access.
        virtual int getSize() { return size; }
        virtual XRefEntry *getEntry(int i) { return &entries[i]; }
        */
};

#endif
