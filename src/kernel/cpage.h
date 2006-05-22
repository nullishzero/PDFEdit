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

//
// Constatnts needed by GfxState
// xpdf doesn't know the keyword CONST....
//
static const double DEFAULT_HDPI 	= 72;
static const double DEFAULT_VDPI 	= 72;
static const int DEFAULT_ROTATE 	= 0;		// no rotate

static const double DEFAULT_PAGE_LX = 0;
static const double DEFAULT_PAGE_LY = 0;
static const double DEFAULT_PAGE_RX = 612;		// width of A4 on a device with 72 horizontal dpi
static const double DEFAULT_PAGE_RY = 792;		// height of A4 on a device with 72 vertical dpi

/** Graphical state parameters. */
typedef struct DisplayParams
{
	/** Paramaters */
	double 		hDpi;		/*< Horizontal DPI. */
	double 		vDpi; 		/*< Vertical DPI. */
	Rectangle 	pageRect;	/*< Page rectangle. */
	int 		rotate;		/*< Page rotation. */
	GBool		useMediaBox;/*< Use page media box. */
	GBool		crop;		/*< Crop the page. */
	GBool		upsideDown;	/*< Upside down. */
	
	/** Constructor. */
	DisplayParams () : hDpi (DEFAULT_HDPI), 
					   vDpi (DEFAULT_VDPI),
					   pageRect (Rectangle (DEFAULT_PAGE_LX, DEFAULT_PAGE_LY, 
								   			DEFAULT_PAGE_RX, DEFAULT_PAGE_RY)),
					   rotate (DEFAULT_ROTATE),
					   useMediaBox (gTrue),
					   crop (gFalse),
					   upsideDown (gFalse) {};
} DisplayParams;


/** 
 * Comparator that will define area around specified point. 
 */
struct PdfOpCmpRc
{
	/** Consructor. */
	PdfOpCmpRc (const Rectangle& rc) : rc_(rc) {};
	
	/** 
	 * Is in in a range. 
	 *
	 * Our rectangle does NOT contain another rectangle if
	 * max (xleft-our, xright-our) < min (xleft, xright)
	 * min (xleft-our, xright-our) > max (xleft, xright)
	 * max (yleft-our, yright-our) < min (yleft, yright)
	 * min (yleft-our, yright-our) < max (yleft, yright)
	 */
	bool operator() (const Rectangle& rc) const
	{
		if ( std::max(rc_.xleft,rc_.xright) < std::min(rc.xleft, rc.xright) )
			return false;
		if ( std::min(rc_.xleft,rc_.xright) > std::max(rc.xleft, rc.xright) )
			return false;

		if ( std::max(rc_.yleft,rc_.yright) < std::min(rc.yleft, rc.yright) )
			return false;
		if ( std::min(rc_.yleft,rc_.yright) > std::max(rc.yleft, rc.yright) )
			return false;

		return true;
	}

private:
	const Rectangle rc_;
};


/** 
 * Point comparator.
 */
struct PdfOpCmpPt
{
	/** Consructor. */
	PdfOpCmpPt (const Point& pt) : pt_(pt) {};
	
	/** Is in in a range. */
	bool operator() (const Rectangle& rc) const
	{
		return (rc.contains (pt_.x, pt_.y));
	}

private:
	const Point pt_;
};



//=====================================================================================
// CPage
//=====================================================================================

/**
 * CPage represents pdf page object. 
 *
 * Content stream is parsed on demand because otherwise it would be very slow.
 */
class CPage
{
public:
	typedef std::vector<boost::shared_ptr<CContentStream> > ContentStreams;
	
private:

	/** Pdf dictionary representing a page. */
	boost::shared_ptr<CDict> dictionary;

	/** Class representing content stream. */
	ContentStreams contentstreams;

	
	//
	// Constructors
	//
public:
		
	/** 
	 * Constructor. 
	 * If the dicionary contains not empty content stream, it is parsed.
	 * 
	 * @param pageDict Dictionary representing pdf page.
	 */
	CPage (boost::shared_ptr<CDict>& pageDict);


	//
	// Destructor
	//
public:
	
	/** Destructor. */
	~CPage () { kernelPrintDbg (debug::DBG_INFO, "Page destroyed."); };

	//
	// Comparable interface
	//
public:
	/** Compares if the objects are identical. */
	bool operator== (const CPage& page)
	{
		return (this == &page) ? true : false;
	};
	

	//
	// Get methods
	//	
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
	 * @param opContainer Operator container where operators in specified are
	 * 						wil be stored.
	 * @param rc 		Rectangle around which we will be looking.
	 */
	template<typename OpContainer>
	void getObjectsAtPosition  (OpContainer& opContainer, const Rectangle& rc)
	{	
		kernelPrintDbg (debug::DBG_DBG, " at rectangle (" << rc << ")");
		// Get the objects with specific comparator
		getObjectsAtPosition (opContainer, PdfOpCmpRc (rc));
	}
	
	
	/**
	 * Get objects at specified position. This call will be delegated to
	 * CContentStream class.
	 * 
	 * @param opContainer Operator container where operators in specified are
	 * 						wil be stored.
	 * @param pt 		Point around which we will be looking.
	 */
	template<typename OpContainer>
	void getObjectsAtPosition  (OpContainer& opContainer, const Point& pt)
	{	
		kernelPrintDbg (debug::DBG_DBG, " at point (" << pt << ")");
		// Get the objects with specific comparator
		getObjectsAtPosition (opContainer, PdfOpCmpPt (pt));
	}

	
	/**
	 * Get objects at specified position. This call will be delegated to
	 * CContentStream class.
	 *
	 * @param opContainer Operator container where operators in specified are wil be stored.
	 * @param cmp 	Null if default kernel area comparator should be used otherwise points 
	 * 				 to an object which will decide whether an operator is "near" a point.
	 */
	template<typename OpContainer, typename PositionComparator>
	void getObjectsAtPosition (OpContainer& opContainer, PositionComparator cmp)
	{	
		kernelPrintDbg (debug::DBG_DBG, "");
		
		// Are we in valid pdf
		assert (hasValidPdf (dictionary));
		assert (hasValidRef (dictionary));
		if (!hasValidPdf(dictionary) || !hasValidRef(dictionary))
			throw CObjInvalidObject ();

		// If not parsed
		if (contentstreams.empty())
			parseContentStream ();		
	
		// Get the objects with specific comparator
		for (ContentStreams::iterator it = contentstreams.begin (); it != contentstreams.end(); ++it)
			(*it)->getOperatorsAtPosition (opContainer, cmp);
	}

	/** 
	 * Get contents streams.
	 *
	 * @return Content stream.
	 */
	template<typename Container>
	void
	getContentStreams (Container& container)
	{
		kernelPrintDbg (debug::DBG_DBG, "");

		// If not parsed
		if (contentstreams.empty())
			parseContentStream ();		

		container.clear();
		std::copy (contentstreams.begin(), contentstreams.end(), std::back_inserter(container));
	}

	
	/**  
	 * Returns plain text extracted from a page.
	 * This method uses xpdf TextOutputDevice that outputs page to text device.
	 *
	 * @param text Container where the text will be saved.
	 */
 	void getText (std::string& text) const;

	
	/** 
	 * Returns all objects on a page.
	 *
	 * \TODO what here?
	 */
	 template<typename Container>
	 void getAllObjects (Container& container) const {}


	
	//
	// Helper methods
	//
public:	
	/**
	 * Draw page on an output device.
	 *
	 * @param out Output device.
 	 * @param params Display parameters.
	 */
	void displayPage (::OutputDev& out, const DisplayParams params = DisplayParams ()) const;
	

	/**
	 * Parse content stream. 
	 * Content stream is optional property. When found it is parsed,
	 * nothing is done otherwise.
	 *
	 * @return True if content stream was found and was parsed, false otherwise.
	 */
	bool parseContentStream ();

	
	//
	// Media box interface
	//
public:
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


	 
	 
	//========================= not implemented yet
	/**  
	 *
	 * Vlozi na stranku existujuci objekt a vytvori novy  identifikator vlozeneho objektu, ktory vrati.
	 * 
	 */
	// boost::shared_ptr<IProperty> insertObject (CAny,position);

	 
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
