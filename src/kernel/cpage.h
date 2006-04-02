/*
 * =====================================================================================
 *        Filename:  cpage.cc
 *     Description:  CPage.
 *         Created:  20/03/2006 11:46:14 AM CET
 * =====================================================================================
 *
 * \TODO
 * 		......	
 */

#ifndef _CPAGE_H
#define _CPAGE_H


// all basic includes
#include "static.h"

// CDict
#include "cobject.h"


//=====================================================================================
namespace pdfobjects {
//=====================================================================================


/** Invalid page number. */
const unsigned int PAGE_NUMBER_INVALID = std::numeric_limits<unsigned int>::max();
/** Coordinate. */
typedef double Coordinate;
/** Invalid coordinate. */
const double COORDINATE_INVALID = std::numeric_limits<Coordinate>::max();

/**
 * Rectangle structure. Defined as in pdf specification v1.5 (p. 133)
 */
typedef struct Rect
{
	Coordinate xleft;
	Coordinate yleft;
	Coordinate xright;
	Coordinate yright;

	// Constructor
	Rect ()	{xleft = yleft = xright = yright = COORDINATE_INVALID;}

} Rectangle;
		
		
/**
 * CPage represents pdf page object. 
 *
 *
 */
class CPage
{
public:
	typedef	unsigned int PageNumber;
		
private:

	/** Pdf dictionary representing a page. */
	boost::shared_ptr<CDict> dictionary;

	/** Page number of this page. */
	PageNumber	pageNumber;

		
public:
		
	/** Constructor. */
	CPage ();

	/** Constructor. */
	CPage (boost::shared_ptr<CDict> pageDict):dictionary(pageDict) {};

public:
	
	/** Destructor. */
	~CPage () { printDbg (debug::DBG_INFO, "Page destroyed."); };

public:
	/** Compares if the objects are identical. */
	bool operator== (const CPage& page)
	{
		return (this == &page) ? true : false;
	};
	

public:
	
	/**
	 * Get dictionary representing this CPage.
	 *
	 * @return Dictionary.
	 */
	boost::shared_ptr<CDict> getDictionary () const { return dictionary; };
	
	/**
	 * Return page number of this page.
	 *
	 * @return Page number of this page.
	 */
	PageNumber getPageNumber () const { return pageNumber; };

	/**
	 * Set page number.
	 *
	 * @param num Page number.
	 */
	void setPageNumber (PageNumber num) {assert (PAGE_NUMBER_INVALID == pageNumber); pageNumber = num;};

	

	/**  
	 * Returns plain text extracted from a page.
	 *
	 * @param text Container where the text will be saved.
	 */
 	void getText (std::string& /*text*/) const {};


	/**  
	 *
	 * Vlozi na stranku existujuci objekt a vytvori novy  identifikator vlozeneho objektu, ktory vrati.
	 * 
	 */
	// boost::shared_ptr<IProperty> insertObject (CAny,position);

	
	/**  
	 * Return media box of this page. It is a required item in page dictionary (spec p.119).
	 *
	 * @return Rectangle specifying the box.
	 */
	 Rectangle getMediabox () const {return Rectangle ();};


	/** 
	 * Returns all objects on a page.
	 *
	 * \TODO what here?
	 */
	 template<typename Container>
	 void getAllObjects (Container& container) const {}

	 
/**  Vrati pole bbox pre kazde pismeno od ord_pos (v plaintexte)
  s dlzkou len.*/
//  array [rectangle] get_textpos (ord_pos,len);

	/** 
	 * Remove !!object!! from page.
	 *
	 * \TODO object?
	 */
	//void delete_object (CAny*);

	 
/**  Vytvori novy graficky objekt so zadanou velkostou a poziciou.
  Graficky objekt je prazdny a pre vkladanie sa pouzivaju metody
  objektu graphic*/
//  CGraphic& create_graphic (bbox, position);

/**  Importuje graficky objekt (obrazok) do stranky na
  poziciu.*/
//  CGraphic& import_graphic (filename,position);

/**  Vytvori a vlozi na stranku novu anotaciu zadaneho typu. Typovo specificke
  nastavenie je mozne urobit pomocou (get/set)_property.*/
//  CAnnotation create_annotation (type,rectangle);

	/**  
	 * Create text !!object!! and put it at specified position.
	 *
	 * \TODO
	 */
//  CText create_text (string,position);

};



//=====================================================================================
} /* namespace pdfobjects */
//=====================================================================================


#endif // _CPAGE_H
