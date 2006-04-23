// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80
/*
 * =====================================================================================
 *        Filename:  cpage.cc
 *     Description:  CPage.
 *         Created:  20/03/2006 11:46:14 AM CET
 * =====================================================================================
 */

#ifndef _CPAGE_H
#define _CPAGE_H


// all basic includes
#include "static.h"

// CDict
#include "cobject.h"
// CContentstream
#include "ccontentstream.h"


//=====================================================================================
namespace pdfobjects {
//=====================================================================================


/**
 * Comparator that will define area around specified point.
 */
struct PdfOpCmpRc
{
private:
	const Rectangle rc_;
public:
	/** Consructor. */
	PdfOpCmpRc (const Rectangle& rc) : rc_(rc) {};
	
	/** Is in in a range. */
	bool operator() (const Rectangle&) const
	{
		return true;
	}
};

struct PdfOpCmpPt
{
private:
	const Point pt_;
public:
	/** Consructor. */
	PdfOpCmpPt (const Point& pt) : pt_(pt) {};
	
	/** Is in in a range. */
	bool operator() (const Rectangle&) const
	{
		return true;
	}
};

//
// Constatnts needed by GfxState
//
// xpdf doesn't know the keyword CONST....
//
static double DEFAULT_HDPI 	= 72;
static double DEFAULT_VDPI 	= 72;
static int DEFAULT_ROTATE 	= 0;

static double DEFAULT_PAGE_LX = 0;
static double DEFAULT_PAGE_LY = 0;
static double DEFAULT_PAGE_RX = 700;
static double DEFAULT_PAGE_RY = 700;



//=====================================================================================

/**
 * CPage represents pdf page object. 
 *
 *
 */
class CPage
{
private:

	/** Pdf dictionary representing a page. */
	boost::shared_ptr<CDict> dictionary;

	/** Class representing content stream. */
	boost::shared_ptr<CContentStream> contentstream;

public:
		
	/** Constructor. */
	CPage () {};

	/** 
	 * Constructor. 
	 * If the dicionary contains not empty content stream, it is parsed.
	 * 
	 * @param pageDict Dictionary representing pdf page.
	 */
	CPage (boost::shared_ptr<CDict>& pageDict);

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
	 * Get objects at specified position. This call will be delegated to
	 * CContentStream class.
	 *
	 * @param rc Rectangle around which we will be looking.
	 * @param cmp 		Null if default kernel area comparator should be used
	 * 						otherwise points to an object which will decide whether an operator 
	 * 						is "near" a point.
	 * 	@param hDpi		Horizontal dpi.
	 * 	@param vDpi		Vertival dpi.
	 * 	@param pageRect Page bounding box.
	 * 	@param rotata	Rotation.
	 */
	template<typename OpContainer>
	void getObjectsAtPosition  (OpContainer& opContainer, 
								const Rectangle& rc,
								double hDpi = DEFAULT_HDPI, 
								double vDpi = DEFAULT_VDPI, 
								const Rectangle pageRect = Rectangle (DEFAULT_PAGE_LX, DEFAULT_PAGE_LY,
																	  DEFAULT_PAGE_RX, DEFAULT_PAGE_RY), 
								int rotate = DEFAULT_ROTATE) const
	{	
		printDbg (debug::DBG_DBG, " at rectangle (" << rc << ")");
		// Get the objects with specific comparator
		getObjectsAtPosition (opContainer, PdfOpCmpRc (rc), hDpi, vDpi, pageRect, rotate);
	}
	/**
	 * Get objects at specified position. This call will be delegated to
	 * CContentStream class.
	 *
	 * @param pt 		Point around which we will be looking.
	 * @param cmp Null if default kernel area comparator should be used
	 * 				otherwise points to an object which will decide whether an operator 
	 * 				is "near" a point.
	 * 	@param hDpi		Horizontal dpi.
	 * 	@param vDpi		Vertival dpi.
	 * 	@param pageRect Page bounding box.
	 * 	@param rotata	Rotation.
	 */
	template<typename OpContainer>
	void getObjectsAtPosition  (OpContainer& opContainer, 
								const Point& pt,
								double hDpi = DEFAULT_HDPI, 
								double vDpi = DEFAULT_VDPI, 
								const Rectangle pageRect = Rectangle (DEFAULT_PAGE_LX, DEFAULT_PAGE_LY,
																	  DEFAULT_PAGE_RX, DEFAULT_PAGE_RY), 
								int rotate = DEFAULT_ROTATE) const
	{	
		printDbg (debug::DBG_DBG, " at point (" << pt << ")");
		// Get the objects with specific comparator
		getObjectsAtPosition (opContainer, PdfOpCmpPt (pt), hDpi, vDpi, pageRect, rotate);
	}
	
	template<typename OpContainer, typename PositionComparator>
	void getObjectsAtPosition ( OpContainer& opContainer, PositionComparator cmp, 
								double hDpi, double vDpi, const Rectangle& pageRect, int rotate ) const
	{	
		// Create Media (Bounding) box
		boost::shared_ptr<PDFRectangle> rc (new PDFRectangle (pageRect.xleft, pageRect.yleft,
															  pageRect.xright, pageRect.yright));;
		// Create state
		GfxState state (hDpi, vDpi, rc.get(), rotate, gFalse);
		// Get the objects with specific comparator
		contentstream->getOperatorsAtPosition (opContainer, cmp, state);
	}

	/**
	 * Draw page on an output device.
	 *
	 * @param out Output device.
 	 * @param hDpi Horizontal dpi.
	 * @param vDpi Vertival dpi.
	 * @param rotata Rotation.
	 */
	void displayPage (	::OutputDev& out, 
						double hDpi = DEFAULT_HDPI, 
						double vDPI = DEFAULT_VDPI,
						int rotate = DEFAULT_ROTATE
						) const;
	
	/** 
	 * Get contents stream.
	 *
	 * @return Content stream.
	 */
	boost::shared_ptr<CContentStream> getContentStream () const { return contentstream; };



	/**
	 * Parse content stream. 
	 * Content stream is optional property. When found it is parsed,
	 * nothing is done otherwise.
	 *
	 * @return True if content stream was found and was parsed, false otherwise.
	 */
	bool parseContentStream ();


//========================= not implemented yet
	
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
	 Rectangle getMediabox () const;

	 
	/**  
	 * Set media box of this page. 
	 * 
	 * @param rc Rectangle specifying the box.
	 */
	 void setMediabox (const Rectangle& rc);


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
