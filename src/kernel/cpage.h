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
static const int DEFAULT_ROTATE 	= 0;

static const double DEFAULT_PAGE_LX = 0;
static const double DEFAULT_PAGE_LY = 0;
static const double DEFAULT_PAGE_RX = 700;
static const double DEFAULT_PAGE_RY = 700;

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
private:
	const Rectangle rc_;
public:
	/** Consructor. */
	PdfOpCmpRc (const Rectangle& rc) : rc_(rc) {};
	
	/** Is in in a range. */
	bool operator() (const Rectangle& rc) const
	{
		return (rc_.contains (rc.xleft, rc.yleft));
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

//=====================================================================================

/**
 * CPage represents pdf page object. 
 *
 * Content stream is parsed on demand because it can be horribly slow.
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
	~CPage () { kernelPrintDbg (debug::DBG_INFO, "Page destroyed."); };

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
 	 * @param params Display parameters.
	 */
	template<typename OpContainer>
	void getObjectsAtPosition  (OpContainer& opContainer, 
								const Rectangle& rc,
								const DisplayParams params = DisplayParams())
	{	
		kernelPrintDbg (debug::DBG_DBG, " at rectangle (" << rc << ")");
		// Get the objects with specific comparator
		getObjectsAtPosition (opContainer, PdfOpCmpRc (rc), params);
	}
	/**
	 * Get objects at specified position. This call will be delegated to
	 * CContentStream class.
	 * 
	 * @param opContainer Operator container where operators in specified are
	 * 						wil be stored.
	 * @param pt 		Point around which we will be looking.
 	 * @param params Display parameters.
	 */
	template<typename OpContainer>
	void getObjectsAtPosition  (OpContainer& opContainer, 
								const Point& pt,
								const DisplayParams params = DisplayParams())
	{	
		kernelPrintDbg (debug::DBG_DBG, " at point (" << pt << ")");
		// Get the objects with specific comparator
		getObjectsAtPosition (opContainer, PdfOpCmpPt (pt), params);
	}
	/**
	 * Get objects at specified position. This call will be delegated to
	 * CContentStream class.
	 *
	 * @param opContainer Operator container where operators in specified are
	 * 						wil be stored.
	 * @param cmp 	Null if default kernel area comparator should be used otherwise points 
	 * 				 to an object which will decide whether an operator is "near" a point.
 	 * @param params Display parameters.
	 */

	template<typename OpContainer, typename PositionComparator>
	void getObjectsAtPosition ( OpContainer& opContainer, 
								PositionComparator cmp,
								const DisplayParams params = DisplayParams())
	{	
		// Create Media (Bounding) box
		boost::shared_ptr<PDFRectangle> rc (new PDFRectangle (params.pageRect.xleft, params.pageRect.yleft,
															  params.pageRect.xright, params.pageRect.yright));;
		// Create state
		GfxState state (params.hDpi, params.vDpi, rc.get(), params.rotate, params.upsideDown);
		
		// If not parsed
		if (contentstreams.empty())
			parseContentStream ();		
	
		// Get objects
		for (ContentStreams::iterator it = contentstreams.begin (); it != contentstreams.end(); ++it)
		{
			// Get the objects with specific comparator
			(*it)->getOperatorsAtPosition (opContainer, cmp, state);
		}
	}

	
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
