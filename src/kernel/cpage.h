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
const unsigned int PAGE_NUMBER_INVALID = 0xFFFFFFFF;

		
/**
 * CPage represents pdf page object. 
 *
 *
 */
class CPage : public CDict 
{
public:
	typedef	unsigned int PageNumber;
		
private:

	/** Page number of this page. */
	PageNumber	pageNumber;
		
		
public:
		
	/** Constructor. */
	CPage ();

public:
	
	/** Destructor. */
	~CPage () { printDbg (DBG_INFO, "Page destroyed."); };


public:
	
	/**
	 * Return page number of this page.
	 *
	 * @return Page number of this page.
	 */
	PageNumber getPageNumber () { return pageNumber; };

	/**
	 * Set page number.
	 *
	 * @param num Page number.
	 */
	void setPageNumber (PageNumber num) {assert (PAGE_NUMBER_INVALID == pageNumber); pageNumber = num;};

	

/**  Vrati plain text zadanej stranky (ziadne zazraky)*/
//  string text_export ();



/**  Vlozi na stranku existujuci objekt a vytvori novy
  identifikator vlozeneho objektu, ktory vrati.*/
//  CAny& insert_object (CAny,position);

/**  Vrati bounding box zadaneho objektu*/
//  bbox get_object_bbox (CAny&);

/**  Vrati pole vsetkych objektov na stranke.*/
//  array [CAny*] get_objects (family mask);

/**  Vrati pole bbox pre kazde pismeno od ord_pos (v plaintexte)
  s dlzkou len.*/
//  array [rectangle] get_textpos (ord_pos,len);

/**  Odstrani zadany objekt zo stranky.*/
//  void delete_object (CAny*);

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

/**  Vytvori novy textovy objekt, ktory obsahuje dany retazec a je umiestneny
  na pozicii position (pozn. vzhladom k comu sa urcuje pozicia)*/
//  CText create_text (string,position);

};



//=====================================================================================
} /* namespace pdfobjects */
//=====================================================================================


#endif // _CPAGE_H
