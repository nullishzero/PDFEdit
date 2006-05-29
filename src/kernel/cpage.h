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


//=====================================================================================
// Display parameters (loose xpdf paramters put into a simple structure)
// 	--  default values are in cpage.cc because we do not want to have global variables.
//=====================================================================================

/** 
 * Graphical state parameters. 
 *
 * These parameters are needed by xpdf.
 */
typedef struct DisplayParams
{
	/** Paramaters */
	double 		hDpi;		/*< Horizontal DPI. */
	double 		vDpi; 		/*< Vertical DPI. 	*/
	Rectangle 	pageRect;	/*< Page rectangle. */
	int 		rotate;		/*< Page rotation. 	*/
	GBool		useMediaBox;/*< Use page media box. */
	GBool		crop;		/*< Crop the page. 	*/
	GBool		upsideDown;	/*< Upside down. 	*/
	
	/** Constructor. */
	DisplayParams ();

} DisplayParams;


//=====================================================================================
// Text search parameters (loose xpdf paramters put into a simple structure)
// 	--  default values are in cpage.cc because we do not want to have global variables.
//=====================================================================================

/** Text search parameters. */
typedef struct TextSearchParams
{
	/** Paramaters */
	GBool startAtTop;		/*< Start searching from the top.    */
	double xStart; 			/*< Start searching from x position. */
	double yStart; 			/*< Start searching from y position. */
	double xEnd; 			/*< Stop searching from x position.  */
	double yEnd; 			/*< Stop searching from y position.  */

	/** Constructor. */
	TextSearchParams ();

} TextSearchParams;


//=====================================================================================
// Comparators Point/Rectangle
//=====================================================================================

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
	 * min (xleft-our, xright-our) >= min (xleft, xright)
	 * max (xleft-our, xright-our) <= max (xleft, xright)
	 * min (yleft-our, yright-our) >= min (yleft, yright)
	 * max (yleft-our, yright-our) <= max (yleft, yright)
	 */
	bool operator() (const Rectangle& rc) const
	{
		if ( std::min(rc_.xleft,rc_.xright) >= std::min(rc.xleft, rc.xright) ) {
			return false;
		}
		if ( std::max(rc_.xleft,rc_.xright) <= std::max(rc.xleft, rc.xright) ) {
			return false;
		}

		if ( std::min(rc_.yleft,rc_.yright) >= std::min(rc.yleft, rc.yright) ) {
			return false;
		}
		if ( std::max(rc_.yleft,rc_.yright) <= std::max(rc.yleft, rc.yright) ) {
			return false;
		}

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

	/** Last used display parameters. */
	DisplayParams lastParams;
	
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
	// template<typename Container>
	// void getAllObjects (Container& container) const {}


	
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
	void displayPage (::OutputDev& out, const DisplayParams params) {
		// Sett last used display paameters
		lastParams = params;
		if (params.useMediaBox)
			lastParams.pageRect = getMediabox();

		// display page
		displayPage (out);
	}

	
	/**
	 * Draw page on an output device with last Display parameters.
	 *
	 * @param out Output device.
 	 * @param params Display parameters.
	 */
	void displayPage (::OutputDev& out) const;

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


	 //
	 // Text search/find interface
	 //
public:
	 /**
	  * Find text on a page.
	  */
	// Rectangle findText (std::string text, const TextSearchParams& params = TextSearchParams()) const {};
	 
	 /**
	  * Find all occurences on a page.
	  *
	  * It uses xpdf TextOutputDevice function for finding text.
	  *
	  * @param text Search text.
	  * @param recs Result rectangles of found text.
	  * @param param Search params.
	  *
	  * @return Number of occurences found.
	  */
	 template<typename RectangleContainer>
	 size_t findText (std::string text, 
					  RectangleContainer& recs, 
					  const TextSearchParams& params = TextSearchParams()) const;
	 
	 
	//========================= not implemented yet
	/**  
	 *
	 * Vlozi na stranku existujuci objekt a vytvori novy  identifikator vlozeneho objektu, ktory vrati.
	 * 
	 */
	// boost::shared_ptr<IProperty> insertObject (CAny,position);

	 
/**  Vytvori novy graficky objekt so zadanou velkostou a poziciou.
  Graficky objekt je prazdny a pre vkladanie sa pouzivaju metody
  objektu graphic*/
//  CGraphic& create_graphic (bbox, position);

/**  Importuje graficky objekt (obrazok) do stranky na
  poziciu.*/
//  CGraphic& import_graphic (filename,position);

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
