/*
 * =====================================================================================
 *        Filename:  cpdf.h
 *     Description: CPdf class definition.
 *         Created:  01/28/2006 03:48:14 AM CET
 *          Author:  jmisutka (), 
 *         Changes: 2006/1/28 added mapping support
 *         			2006/1/30 tested objToString () -- ok
 * =====================================================================================
 */


#ifndef __CPDF_H__
#define __CPDF_H__

#include <map>
#include <set>

/**
 * Forward declarations
 */
class Object;


// =====================================================================================
namespace pdfobjects {

/**
 * Forward declarations
 */
class IProperty;


//
// Mapping between (x)pdf Object <--> IProperty
//
namespace
{
	/**
	 * Comparator class for two mapped items.
	 */
	class ObjComparator
	{
	public:
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

}
		
		

/**
 * CPdf special object.
 * 
 * TODO: many things. Right now mapping is finished because Raw CObject needs it...
 */
class CPdf : public CDict
{
		
private:
	/** Mapping between Object <--> IProperty*. It is necessary when releasing objects. */
	ObjectMapping objMap;
	/** Mapping between Id + Gen <--> IProperty*. It is necessary when accessing indirect objects. */
	IndirectMapping indMap;
	/** CXref class which is (x)pdf xref with enhanced functionality.*/
	//CXref*	xref;

public:
	/**
	 * Constructor
	 */
	CPdf ();
//  CPdf (filename,mode);


public:
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
	 * @param o	 (x)pdf object.
	 * @param ip IProperty that will be mapped to Object o.
	 */
	void setObjectMapping (const Object* o, const IProperty* ip);
	void setIndMapping (const IndiRef& pair, const IProperty* ip);

	/**
	 * Deletes relation between (x)pdf object and IProperty*. 
	 *
	 * @param o	 (x)pdf object.
	 * @param ip IProperty that will be mapped to Object o.
	 */
	void delObjectMapping (const Object* o);
	void delIndMapping (const IndiRef& pair);

	
public:
	/** Return xref associated with this pdf. */
	//CXref* getXref() {return xref;};


public:
 /** Vytvori vnutornu reprezentaciu pdf (pre pristup k objektom sa bude
  uplatnovat mode - READ_ONLY, READ_WRITE).*/
//  int open (filename,mode);

  //
  // Destruktor
  //
//  virtual ~CPdf();

/**  !open.*/
//  void close ();

/**  Do suboru ulozi aktualnu verziu (reviziu) dokumentu. Neoptimalizuje
  vyskyt nepotrebnych objektov.*/
//  void save (filename);

/**  Vrati identifikator strany zadanej poradim od uplne prvej
  stranky.*/
//  CPage& get_page (int cislo_stranky);

/**  Vrati referenciu na stranku zadanej menom (I,IV,a...).*/
//  CPage& get_page (string meno_stranky);

/**  Vrati uplne prvu stranku.*/
//  CPage& get_page_first ();

/**  Vrati uplne poslednu stranku.*/
//  CPage& get_page_last ();

/**  Vrati nasledujuci stranku.*/
//  CPage& get_page_next (CPage&);

/**  Vrati predchadzajucu stranku. Zo zadaneho identifikatora, sa bude
  dat zistit, ku ktoremu pdf sa dana stranka viaze.*/
//  CPage& get_page_prev (CPage&)

/**  Vlozi stranku do zadaneho pdf na zadane miesto, vrati novy
  identifikator vlozenej stranky.*/
//  CPage& insert_page (CPage&, int page_position)

/**  Odstrani stranku z pdf. (v novej verzii nebude zobrazena)*/
//  void delete_page (CPage*);

/**  Vrati pocet stranok.*/
//  int page_count ();

/** Vrati vsetky outline najvyssej urovne. Ich potomkov vieme
  ziskat s pomocou get_outline_children.*/
//  array [COutline*] get_outlines ();

/**  Vytvori novu prazdnu stranku, asociovanu so zadanym pdf. Umiestni ju
  na ord_pos (od zaciatku dokumentu). Name urcuje "cislo" stranky.*/
//  CPage& create_page (int ord_pos, string name);

/**  Vytvori novy outline a vlozi ho do hierarchie. Toplevel outline je mozne
  riesit ako vhodnym zvolenim niektoreho parametra (napr. parent == NULL )
  Name urcuje meno outline, target miesto, kam sa dostaneme po klinkuti na nu.
  Target bude struktura obsahujici: stranku (CPage &),
   pozici na strance (souradnice) a zoom.
           (+ asi par preddefinovanych konstant zoomu na "fit to width" apod..)*/
//  COutline& create_outline (COutline& parent, int position, string name,target);

/**  Z pdf odstrani objekty, na ktore nie su ziadne referencie.
  Mode urcuje sposob priebehu operacie (zakomprimuje ret., ...).*/
//  void optimize (mode);

/**  Prehlasi danu reviziu za poslednu (teda nastavi mode na READ_WRITE),
  zahodi vsetky zmeny v poslednej revizii.*/
//  void set_lastver ();

/**  Vrati meno predchadzajucej revizie a nastavi ju ako aktualnu.
  Nastavi editovaci mod na READ_ONLY*/
//  string set_prevver ();

/**  Vrati meno nasledujucej revizie a nastavi ju ako aktualnu. Pre poslednu
  reviziu nastavi mode na READ_WRITE.*/
//  string set_nextver ();

/**  Vrati meno aktualnej revizie.*/
//  string get_namever ();

/**  Vrati mode danej revizie.*/
//  mode get_modever ();

};











		
} // namespace pdfobjects

#endif // __CPDF_H__
