#ifndef __CPAGE_H__
#define __CPAGE_H__

#include"interface.h"
#include"xpdf/Object.h"
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
    ObjectCache * cache;            /**< Cache for objects. */
protected:        
    ObjectStorage changedStorage;   /**< Object storage for changed objects. */
    ObjectStorage newStorage;       /**< Object storage for newly created 
                                         objects.*/
    ObjectStorage releasedStorage;  /**< Object storage for release objects. */
    
    /** Registers change in given object addressable through given num and gen.
     * @param instance Instance of object value (must be direct value,
     * not indirect reference).
     * @param num Object number.
     * @param gen Genereation number of object (FIXME may be doesn't need).
     *
     * Discards object from cache and stores it to the changedStorage.
     * Method should be called each time when object has changed its value.
     * Object value is copied not use as it is (creates deep copy).
     */
    void changeObject(Object * instance, int num, int gen);
    
    /** Releases  given object addressable through given num and gen.
     * @param instance Instance of object value (must be direct value,
     * not indirect reference).
     * @param num Object number.
     * @param gen Genereation number of object (FIXME may be doesn't need).
     *
     * Discards object from cache, removes it from the changedStorage (if 
     * presented) and stores it to the releasedStorage.
     * Method should be called if indirect reference was removed from 
     * compound value (e.g. changed by other indirect reference or direct
     * value).
     * Object value is copied not use as it is (creates deep copy).
     */
    void releaseObject(Object * instance, int num, int gen);

    /** Changes entry in trailer dictionary.
     * @param name Name of the value.
     * @param value Value to be set.
     *
     * Makes changes to the trailer and if value should be set as indirect
     * value, num and gen has to be non negative. In such case calls also
     * changeObject method.
     * <br>
     * If given value is indirect reference, only change in trailer dictionary
     * is made.
     */
    void changeTrailer(string name, Object * value);
    
public:
    /** Creates new xpdf indirect object.
     * @param type Type of the object.
     * Object instance is registered to the internal structures and so can be
     * accessible throught its object number and generation number. 
     * Implementation will initialized Object at least with type, obj and gen
     * num and value in the object is intialized: 
     * <ul>
     * <li>numeric types with zero
     * <li>strings allocated and empty
     * <li>compound types (array, dict) allocated with no members
     * </ul>
     * Returned object MUST NOT be deallocated by user.
     *
     * @return Object instance with given type.
     */
    Object * createObject( type);

    /** Gets value associated with name in trailer.
     * @param name Name of the entry.
     *
     * If value is indirect reference (according specification almost all 
     * compound entries), reference is returned, so normal xref interface
     * can be used to ask for value.
     * @return copy of the object value.
     */
    Object * getTrailerEntry(const string * name);
        
    // TODO reimplementation of inherited methods
    // three steps
    //      cache
    //      maintained objects
    //      original implementation
    // this is only for methods working with changeable objects
    // e.g. getRootNum doesn't have to be change at all
}

#endif
