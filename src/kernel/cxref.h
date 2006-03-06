#ifndef __CPAGE_H__
#define __CPAGE_H__

/** 
 * $RCSfile$
 *
 * $Log$
 * Revision 1.9  2006/03/06 18:18:55  hockm0bm
 * compilable changes - each module is compilable now
 * each object in pdfobjects namespace
 * more comments
 * cpdf - most of methods are just comments how to implement
 *
 * Revision 1.8  2006/02/28 19:19:26  hockm0bm
 * changeStorage changed  - Object is associated with stored flag
 * getNumObjects - reimplemented (only initialized objects)
 *
 * Revision 1.7  2006/02/13 21:11:54  hockm0bm
 * Implementation of the class moved to cc file
 * fetch method prepared for cache
 *
 * Revision 1.6  2006/02/12 17:19:07  hockm0bm
 * first implementation (compileable) - not for real use
 *
 * Revision 1.5  2006/02/02 15:44:58  misuj1am
 *
 *
 *
 * Zmeny (trochu vacsie).
 *
 * 0) spravil som exceptions.h a na observera spravil novy file
 * 1) vyhodil som IId, strcil som to do IProperty
 * 2) trochu "som si zlepsil" qt projekt file
 * 3) gcc ma s TEMPLATE stale problemy je to jednak slabou implementaciou gcc,
 * jednak samotnym standardom, kde sa kvoli jednoduchosti programovania kompileru zatial
 * vynechali nejake veci.
 *
 * POZOR: bacha na uplne nezmyselne compile errors suvisiace s template, vascinou to moze byt zabudnutie
 * strednika alebo podobna ptakovina...
 *
 *
 * Je par obmedzeni kvoli ktorym som sa rozhodol pridat/odstranit nejake subory:
 *
 * a)
 * template a implementacia musia byt v tom istom """priestore viditelnosti""", teda napr. v tom
 * istom subore, alebo vo viacerych suboroch, alebo sa moze pisat nieco ako
 * #include "123.cc"
 * a podobne. Ma to svoj zmysel napr. v tom, ze templates svojou podstatou nie su "kniznica", ze by
 * sa to musel nejako predkompilovat atd...
 *
 * Preto som nakoniec zvolil pomenovanie cobject.h <-- definicia cobjectI.h <-- samotna implementacia
 * niekedy bude implementacia aj v samotnom cobject.h, ale to je detail.
 *
 *
 * jozo
 *
 *
 * ////
 * /////// len tak mimochodom
 * ////
 * S CPdf je to trosku tazsie. CPdf musi pouzivat CObject a naopak.. I ked CPDf nie je templatova trieda nemoze byt v cpdf.cc.
 * Dovodom je, ze pri kompilovani
 * skompiluje gcc main.cc. Do main.o sa (napr.) vytvori instancia CPdf. Teda kompiler instaciuje danu triedu CObject z ktorej dedi CPdf,
 * teda CObject<pDict>. Z toho vyplyva, ze main.o obsahuje instanciu CObject<pDict>. Ked sa bude kompilovat samotny CPdf tak sa tiez
 * spravi instancia ale uz v cpdf.o a s tym moze (ma) linker problemy..... Preto som tam zbytocne nedaval .cc subory, ked ich nie je treba.
 *
 * Revision 1.4  2006/01/29 21:18:15  hockm0bm
 * minor changes for compilation (TODOs and FIXME comments)
 *
 *
 */

#include"iproperty.h"
#include"utils/objectstorage.h"
//#include"utils/cache.h"
#include"xpdf/XRef.h"
#include"xpdf/Object.h"
#include "ErrorCodes.h"

namespace pdfobjects
{
        
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
 * required object, xpdf layer is used (delegates to super type).
 * <br>
 * Methods which produces changes are protected to prevent changing in gui, if
 * kernel wants to make changes, it should use XrefWriter subclass which 
 * delegates functionality to inherited protected methods and adds some 
 * checking.
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
        /** Empty constructor.
         *
         * This constructor is protected to prevent uninitialized instances.
         * We need at least to specify stream with data.
         */
        CXref(): XRef(NULL){};

        /** Entry for ObjectStorage.
         *
         * Each entry contains pointer to changed object and flag which
         * says whether this object has been stored to the file from last
         * change.
         */
        typedef struct
        {
              ::Object * object;
              bool stored;
              
        } ObjectEntry;
        
        /** Object storage for changed objects.
         * Mapping from object referencies to the ObjectEntry structure.
         * This structure contains new Object value for reference and 
         * flag. 
         */
        ObjectStorage<Ref, ObjectEntry*, RefComparator> changedStorage;   

        /** Object storage for newly created objects.
         * Value is the flag, whether value has been changed after createObject
         * method has been called. Uninitialized values can be skipped.
         */
        ObjectStorage<Ref, bool, RefComparator> newStorage;       

        /** Object storage for released referencies.
         * Value of the association stores number of releases called on 
         * given reference.
         */
        ObjectStorage<Ref, int, RefComparator> releasedStorage;  

        /** Registers change in given object addressable through given 
         * reference.
         * @param ref Object reference identificator.
         * @param instance Instance of object value (must be direct value,
         * not indirect reference).
         *
         * Discards object from cache and stores it to the changedStorage.
         * Method should be called each time when object has changed its value.
         * Object value is copied not use as it is (creates deep copy).
         * If object with same reference already was in changedStorage, this
         * is returned to enable history handling (and also to deallocate it). 
         * 0 return value means first revision of object.
         * <br>
         * If given reference is in newStorage, value is set to true to 
         * signalize that value has been changed after object has been created.
         * <br>
         * Note that this function doesn't perform any value ckecking.
         *
         * @return Old value of object from changedStorage or NULL if it's first
         * revision of object.
         */
        ::Object * changeObject(Ref ref, ::Object * instance);

        /** Releases  given object addressable through given reference.
         * @param ref Object reference identificator.
         *
         * Notes, that given reference has been released from compund type. 
         * All released referencies are stored in releasedStorage with counter
         * which stores number of release operation on given reference. 
         */
        void releaseObject(Ref ref);

        /** Changes entry in trailer dictionary.
         * @param name Name of the value.
         * @param value Value to be set.
         *
         * Makes changes to the trailer dictionary. If given value is indirect
         * reference, it must be known.
         * <br>
         * NOTE: doesn't perform any value checking.
         *
         * @return Previous value of object or 0 if previous revision not
         * available (new name value pair in trailer).
         */
        ::Object * changeTrailer(char * name, ::Object * value);

public:
        /** Initialize constructor.
         * @param stream Stream with file data.
         *
         * Delegates to XRef constructor with same parameter.
         */
        CXref(BaseStream * stream):XRef(stream)
        {
                if(getErrorCode() !=errNone)
                {
                        // xref is corrupted
                        // TODO throw an exception
                }
        }

        /** Initialize constructor with cache.
         * @param stream Stream with file data.
         * @param c Cache instance.
         *
         * Delegates to XRef constructor with the stream parameter and
         * sets cache instance.
         */
        /* FIXME uncoment when cache is available
        CXref(BaseStream * stream, ObjectCache * c):XRef(stream), cache(c)
        {
                // FIXME: uncoment when cache is ready 
        }
        */
        
        /** Destructor.
         *
         * Destroys cache.
         */
        virtual ~CXref();
        
        /** Creates new xpdf indirect object.
         * @param type Type of the object.
         * @param ref Structure where to put object num and gen (if null, 
         * nothing is set).
         * 
         * New reference is registered to the newStorage with false flag. 
         * Created object is accesible only if default value has been changed.
         * This means that returned object has to be changed and after change
         * method has to be called. This is mainly because we want to prevent 
         * unintialized object inside.
         * <br>
         * Implementation will initialize Object with type and value depending 
         * on type:
         * <ul>
         * <li>objBool - false
         * <li>objInt - 0
         * <li>objReal - 0
         * <li>objString - NULL
         * <li>objName - NULL
         * <li>objArray - sets this instance as Xref parameter
         * <li>objDict - sets this instance as Xref parameter
         * <li>objCmd - NULL
         * <li>objStream - NULL
         * <li>objNull - no special value is used, Object is just pdf NULL 
         *      object
         * <ul>Following values are not supported (it doesn't make sense to
         *      do so - because they can't be value of indirect object)
         *      <li>objRef
         *      <li>objError
         *      <li>objEOF
         *      <li>objNone
         * </ul>
         * </ul>
         * To change value of the object, use init{Type} method then call
         * change method to register change and makes object visible by
         * fetch method.
         * <br>
         *
         * @return Object instance with given type or 0 if not able to create.
         */
        virtual ::Object * createObject(ObjType type, Ref * ref);

        /** Checks if given reference is known.
         * @param ref Reference to check.
         *
         * First examine if reference is in newStorage and if not found
         * tries to check entries array.
         * <br>
         * If returns true, object can be accessed by fetch method.
         *
         * @return true if reference is known, false otherwise.
         */
        bool knowsRef(Ref ref);

        /** Checks whether obj1 can replace obj2.
         * @param obj1 Original object.
         * @param obj2 Replace object.
         *
         * obj2 can replace obj1 only iff at least 1 condition is true:
         * <ul>
         * <li>obj1 is direct and obj2 has same type (if indirect then
         *      dereferenced value)
         * <li>obj2 is indirect and obj2 has same type (if indirect then
         *      derefereced value)
         * </ul>
         *
         * This means that in indirect value can replace direct one only
         * if both have same value type.
         * <br>
         * TODO: What to do with objNull
         *
         * @return true if obj2 can replace obj1, false otherwise.
         */
        bool typeSafe(::Object * obj1, ::Object * obj2);

        /** Gets value associated with name in trailer.
         * @param name Name of the entry.
         *
         * If value is indirect reference (according specification almost all 
         * compound entries), reference is returned, so normal xref interface
         * can be used to ask for value.
         * If no value with given name can be found, objNull type object is
         * returned.
         * <br>
         * Changes made to returned object doesn't affect trailer entry with 
         * given name.
         *
         * @return Deep copy of the object value.
         */
        virtual ::Object * getTrailerEntry(char * name);

        // Returns the document's Info dictionary (if any).
        virtual ::Object *getDocInfo(::Object *obj);
        
        // Returns the document's Info dictionary (dereferenced
        // if indirect value
        virtual ::Object *getDocInfoNF(::Object *obj);

        /** Returns number of indirect objects.
         *
         * Delegates to XRef::getNumObjects and adds also number of all
         * newly inserted (and initialized) objects.
         *
         * @return Total number of objects.
         */
        virtual int getNumObjects(); 

        /** Fetches object.
         * @param num Object number.
         * @param gen Object generation.
         * @param obj Object where to store content.
         *
         * Try to find object in changedStorage and if not found, delegates
         * to original implementation.
         * <br>
         * NOTE:
         * Returned value is deepCopy of object and changes made to object 
         * don't affect internally maintained values (e.g. it can be 
         * deallocated). 
         * To register a change use change method.
         * <br>
         * This method provide transparent access to changed objects throught
         * XRef (xpdf class) interface.
         *
         * @return Pointer with initialized object given as parameter, if not
         * found obj is set to objNull.
         */
        virtual ::Object * fetch(int num, int gen, ::Object *obj);
};

} // end of pdfobjects namespace

#endif
