/*                                                                              
 * PDFedit - free program for PDF document manipulation.                        
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko, 
 *                                              Miroslav Jahoda,       
 *                                              Jozef Misutka, 
 *                                              Martin Petricek                                             
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit                                                                      
 */ 
// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80
#ifndef _CXREF_H_
#define _CXREF_H_

// xpdf
#include "kernel/xpdf.h"

#include "kernel/iproperty.h"
#include<limits.h>

namespace pdfobjects
{

/** Maximal object number.
 */
const int MAXOBJNUM = INT_MAX;

/** Maximal object generation number.
 *
 * Cross reference entry with this generation number can't be reused.
 */
const int MAXOBJGEN = 65535;

/** Adapter for xpdf XRef class.
 * 
 * This class has responsibility to transparently (same way as XRef do) provide
 * objects synchronized with changes.
 * <br>
 * Class works in three layers:
 * <ul>
 * <li>Maintaining layer - registers changes (in values or new objects), which
 *    uses ObjectStorage instances to know which objects are changed 
 *    (changedStorage field) and which objects are new (newStorage field).  
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
 * Objects returned by this interface are always deep copies of original object 
 * stored inside and changes made to them are not visible through this interface
 * until changeObject or changeTrailer is called.
 * 
 */
class CXref: public XRef
{
private:
	//ObjectCache * cache=NULL;		/**< Cache for objects. */
protected:
	/** Empty constructor.
	 *
	 * This constructor is protected to prevent uninitialized instances.
	 * We need at least to specify stream with data.
	 */
	CXref(): XRef(NULL){}

	/** Entry for ObjectStorage.
	 *
	 * Each entry contains pointer to changed object. It may be extended in
	 * future, so structure is used.
	 */
	typedef struct
	{
		::Object * object;
	} ObjectEntry;
	
	/** Object storage for changed objects.
	 * Mapping from object referencies to the ObjectEntry structure.
	 * This structure contains new Object value for reference and 
	 * flag. 
	 */
	ObjectStorage< ::Ref, ObjectEntry*, xpdf::RefComparator> changedStorage;   

	/** Object storage for newly created objects.
	 * Value is the flag of newly created reference. When new entry is added, it
	 * should have Reserved state and when changed for the first time
	 * Initialized. Unused is default value for not found, so unknown
	 * (ObjectStorage returns 0 if entry is not found). 
	 */
	ObjectStorage< ::Ref, RefState, xpdf::RefComparator> newStorage;

	/** Registers change in given object addressable through given 
	 * reference.
	 * @param ref Object reference identificator.
	 * @param instance Instance of object value (must be direct value,
	 * not indirect reference).
	 *
	 * Discards object from cache and stores given to the changedStorage.
	 * Method should be called each time when object has changed its value.
	 * Object value is copied not use as it is (creates deep copy by clone
	 * method).
	 * <br>
	 * If object with same reference already was in changedStorage, this
	 * is returned to enable history handling (and also to deallocate it). 
	 * 0 return value means first change of object.
	 * <br>
	 * If given reference is in newStorage, value is set to true to 
	 * signalize that value has been changed after object has been created.
	 * <br>
	 * Note that this function doesn't perform any value ckecking.
	 *
	 * @throw NotImplementedException if object cloning fails.
	 *
	 * @return Old value of object from changedStorage or NULL if it's first
	 * revision of object - caller is responsible for deallocation.
	 */
	::Object * changeObject(::Ref ref, ::Object * instance);

	/** Changes entry in trailer dictionary.
	 * @param name Name of the value.
	 * @param value Value to be set.
	 *
	 * Makes changes to the trailer dictionary. If given value is indirect
	 * reference, it must be known.
	 * <br>
	 * NOTE: doesn't perform any value checking.
	 *
	 * @throw NotImplementedException if object cloning fails.
	 * @return Previous value of object or 0 if previous revision not
	 * available (new name value pair in trailer).
	 */
	::Object * changeTrailer(const char * name, ::Object * value);

	/** Reinitializes all internal structures.
	 * @param xrefOff Offset of cross reference table from which to start.
	 *
	 * Clears all internal structures and forces XRef super type to throw away
	 * all internal structures too and parse them again from stream starting
	 * from given position.
	 * <br>
	 * This method should be called if new revision is appended to the stream.
	 * Otherwise all information about changes are lost!
	 */
	void reopen(size_t xrefOff);

	/** Reserves reference for new indirect object.
	 *
	 * Searches for free object number and generation number and uses
	 * it to register reference for new indirect object. Reference is stored
	 * to the newStorage with Reserved flag. This is changed to Initialized if 
	 * real object is stored to the CXref (using change method). 
	 * <br>
	 * Created object is accesible only if default value has been changed.
	 * This means that returned object has to be changed and after change
	 * method has to be called. This is mainly because we want to prevent 
	 * unintialized object inside.
	 * <br>
	 * TODO reuse strathegy - REUSE_FIRST, REUSE_AFTER, NOREUSE
	 *
	 * @throw IndirectObjectsExhausted if all object numbers has been used.
	 * @return Reference which can be used to add new indirect object.
	 */
	virtual ::Ref reserveRef();
	
	/** Creates new xpdf indirect object.
	 * @param type Type of the object.
	 * @param ref Structure where to put object num and gen (if null, 
	 * nothing is set).
	 * 
	 * New reference is registered using reserveRef method. This is just 
	 * wrapper method to reserveRef with object initialization capability.
	 * 
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
	 *	object
	 * <li>Following values are not supported (it doesn't make sense to
	 *	do so - because they can't be value of indirect object)
	 * <ul>
	 *	<li>objRef
	 *	<li>objError
	 *	<li>objEOF
	 *	<li>objNone
	 * </ul>
	 * </ul>
	 * To change value of the object, use init{Type} method then call
	 * change method to register change and makes object visible by
	 * fetch method.
	 * <br>
	 * Returned object has to be deallocated by Object::free and gfree methods 
	 * (first one for value deallocation and second for Object instance itself)
	 * or by pdfobjects::xpdf::freeXpdfObject method.
	 *
	 * @return Object instance with given type or 0 if not able to create.
	 */
	virtual ::Object * createObject(::ObjType type, ::Ref * ref);

	/** Deallocates all internal structures.
	 *
	 * Cleanes up chnagedStorage and deallocates all its entries.
	 * <br>
	 * This method is responsible for deallocation of all internal structures
	 * specific for CXref.
	 */
	void cleanUp();
public:

	
	/** Initialize constructor.
	 * @param stream Stream with file data.
	 *
	 * Delegates to XRef constructor with same parameter.
	 *
	 * @throw MalformedFormatExeption if XRef creation fails (instance is
	 * unusable in such situation).
	 */
	CXref(BaseStream * stream):XRef(stream)
	{
		if(getErrorCode() !=errNone)
		{
			// xref is corrupted
			throw MalformedFormatExeption("XRef parsing problem errorCode="+getErrorCode());
		}
	}

	/** Initialize constructor with cache.
	 * @param stream Stream with file data.
	 * @param c Cache instance.
	 *
	 * Delegates to XRef constructor with the stream parameter and
	 * sets cache instance.
	 *
	 * @throw MalformedFormatExeption if XRef creation fails (instance is
	 * unusable in such situation).
	 */
	/* FIXME uncoment when cache is available
	CXref(BaseStream * stream, ObjectCache * c):XRef(stream), cache(c)
	{
		if(getErrorCode() !=errNone)
		{
			// xref is corrupted
			throw MalformedFormatExeption("XRef parsing problem errorCode="+getErrorCode);
		}
	}
	*/
	
	/** Destructor.
	 *
	 * Calls cleanUp for all internals deallocation and deletes stream.
	 */
	virtual ~CXref();
	
	/** Checks if given reference is known.
	 * @param ref Reference to check.
	 *
	 * Checks if reference is present in newStorage. If found, returns status
	 * stored in newStorage. If not found, searches XRef::entries array.
	 * 
	 * @see UNUSED_REF
	 * @see RESERVED_REF
	 * @see INITIALIZED_REF
	 * @return Current state of given reference.
	 */
	virtual RefState knowsRef(::Ref& ref);

	/** Checks if given reference is known.
	 * @param ref Reference to check.
	 *
	 * Calls knowsRef(::Ref) method. This is just for easy to use wrapper for 
	 * non xpdf code.
	 *
	 * @see knowsRef(::Ref)
	 *
	 * @return reference current state.
	 */
	virtual RefState knowsRef(IndiRef ref)
	{
		::Ref xpdfRef={ref.num, ref.gen};
		return knowsRef(xpdfRef);
	}

	/** Checks whether obj1 can replace obj2.
	 * @param obj1 Original object.
	 * @param obj2 Replace object.
	 *
	 * obj2 can replace obj1 only iff at least 1 condition is true:
	 * <ul>
	 * <li>obj1 is direct and obj2 has same type (if indirect then
	 *	dereferenced value)
	 * <li>obj2 is indirect and obj2 has same type (if indirect then
	 *	derefereced value)
	 * </ul>
	 *
	 * This means that in indirect value can replace direct one only
	 * if both have same value type.
	 *
	 * @return true if obj2 can replace obj1, false otherwise.
	 */
	virtual bool typeSafe(::Object * obj1, ::Object * obj2);

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
	// @throw NotImplementedException if object cloning fails.
	virtual ::Object *getDocInfo(::Object *obj);
	
	// Returns the document's Info dictionary (dereferenced
	// if indirect value
	// @throw NotImplementedException if object cloning fails.
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
	 * don't affect internally maintained values (e.g. it can and should be 
	 * deallocated by caller). 
	 * To register a change use change method.
	 * <br>
	 * This method provide transparent access to changed objects throught
	 * XRef (xpdf class) interface.
	 *
	 * @throw NotImplementedException if object cloning fails.
	 * @return Pointer with initialized object given as parameter, if not
	 * found obj is set to objNull.
	 */
	virtual ::Object * fetch(int num, int gen, ::Object *obj);
};

} // end of pdfobjects namespace

#endif // _CXREF_H_
