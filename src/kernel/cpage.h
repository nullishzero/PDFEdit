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
// CAnnotation
#include "cannotation.h"


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
 * These parameters are used by xpdf and can be changed. These parameters are
 * important when updating bounding boxex of content stream operators,
 * displaying page etc.
 */
typedef struct DisplayParams
{
	/** Paramaters */
	double 		hDpi;		/**< Horizontal DPI. */
	double 		vDpi; 		/**< Vertical DPI. 	*/
	Rectangle 	pageRect;	/**< Page rectangle. */
	int 		rotate;		/**< Page rotation. 	*/
	GBool		useMediaBox;/**< Use page media box. */
	GBool		crop;		/**< Crop the page. 	*/
	GBool		upsideDown;	/**< Upside down. 	*/
	
	/** Constructor. Default values are set. */
	DisplayParams () : 
		hDpi (DEFAULT_HDPI), vDpi (DEFAULT_VDPI),
		pageRect (Rectangle (DEFAULT_PAGE_LX, DEFAULT_PAGE_LY, DEFAULT_PAGE_RX, DEFAULT_PAGE_RY)),
		rotate (DEFAULT_ROTATE), useMediaBox (gTrue), crop (gFalse), upsideDown (gTrue) 
		{}



	/** Equality operator. */
	bool operator== (const DisplayParams& dp) const
	{
		return (hDpi == dp.hDpi && vDpi == dp.vDpi &&
				pageRect == dp.pageRect && rotate == dp.rotate &&
				useMediaBox == dp.useMediaBox && crop == dp.crop &&
				upsideDown == dp.upsideDown);
	}

	//
	// Default values
	// -- small hack to declare them as ints, to be able to init
	// them here (if double, we could not init them here because of the non
	// integral type compilator error))
	// 
	static const int DEFAULT_HDPI 	= 72;		/**< Default horizontal dpi. */
	static const int DEFAULT_VDPI 	= 72;		/**< Default vertical dpi. */
	static const int DEFAULT_ROTATE	= 0;		/**< No rotatation. */

	static const int DEFAULT_PAGE_LX = 0;		/**< Default x position of left upper corner. */
	static const int DEFAULT_PAGE_LY = 0;		/**< Default y position of right upper corner. */
	static const int DEFAULT_PAGE_RX = 612;		/**< Default A4 width on a device with 72 horizontal dpi. */
	static const int DEFAULT_PAGE_RY = 792;		/**< Default A4 height on a device with 72 vertical dpi. */

} DisplayParams;


//=====================================================================================
// Text search parameters (loose xpdf paramters put into a simple structure)
// 	--  default values are in cpage.cc because we do not want to pollute global space.
//=====================================================================================

/** 
 * Text search parameters. 
 *
 * These parameters are used by xpdf when serching a text string.
 */
typedef struct TextSearchParams
{
	/** Paramaters */
	GBool startAtTop;		/**< Start searching from the top.    */
	double xStart; 			/**< Start searching from x position. */
	double yStart; 			/**< Start searching from y position. */
	double xEnd; 			/**< Stop searching from x position.  */
	double yEnd; 			/**< Stop searching from y position.  */

	/** Constructor. Default values are set. */
	TextSearchParams () : 
		startAtTop (DEFAULT_START_AT_TOP),
		xStart (DEFAULT_X_START), yStart (DEFAULT_Y_START), xEnd (DEFAULT_X_END), yEnd (DEFAULT_Y_END)
	{}

	//
	// Default values  
	// -- small hack to declare them as ints, to be able to init
	// them here (if double, we could not init them here because of the non
	// integral type compilator error))
	//
	static const GBool DEFAULT_START_AT_TOP 	= gTrue;	/**< Start at top. */

	static const int DEFAULT_X_START = 0;	/**< Default x position of left upper corner. */
	static const int DEFAULT_Y_START = 0;	/**< Default y position of left upper corner. */
	static const int DEFAULT_X_END = 0;		/**< Default x position of right upper corner. */
	static const int DEFAULT_Y_END = 0;		/**< Default y position of right upper corner. */


} TextSearchParams;


//=====================================================================================
// Comparators Point/Rectangle
//=====================================================================================

/** 
 * Comparator that we can use to find out if another rectangle intersects
 * rectangle specified by this comparator.
 */
struct PdfOpCmpRc
{
	/** 
	 * Consructor. 
	 *
	 * @param rc Rectangle used when comparing.
	 */
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
	const Rectangle rc_;	/**< Rectangle to be compared. */
};


/** 
 * Comparator that we can use to find out if a rectange contains point specified
 * by this comparator.
 */
struct PdfOpCmpPt
{
	/** 
	 * Consructor. 
	 * 
	 * @param pt Point that we use when comparing.
	 */
	PdfOpCmpPt (const Point& pt) : pt_(pt) {};
	
	/** 
	 * Is point in a rectangle. 
	 * 
	 * @param rc Rectangle.
	 */
	bool operator() (const Rectangle& rc) const
	{
		return (rc.contains (pt_.x, pt_.y));
	}

private:
	const Point pt_;	/**< Point to be compared. */
};

/** Sets unitialized inheritable page attributes.
 * @param pageDict Page dictionary reference where to set values.
 *
 * Gets InheritedPageAttr structure for given pageDict (uses
 * fillInheritedPageAttr helper function) and sets all fields which are not
 * present in given dictionary to found values.
 */
void setInheritablePageAttr(boost::shared_ptr<CDict> & pageDict);

//=====================================================================================
// CPage
//=====================================================================================

//
//
//
typedef observer::IObserverHandler<CPage> CPageObserverSubject;

/**
 * This object represents one pdf page object. PDF page object is a dictionary
 * reachable from page tree structure with several required properties. 
 * It is responsible just for one single page.
 *
 * Every pdf page contains all information required for displaying the page
 * (e.g. page metrics, page contents etc.) Page properties can be inherited from
 * its parent in the page tree. Only the most specific is used. This feature can
 * cause problems because it is no well defined what does it mean to change a
 * property that is inherited (it is not present in the page dictionary but in a
 * parent)
 *
 * We display a page using xpdf code. The argument to this function is an output
 * device which can draw graphical objects. The contents of a page is specified
 * by a "Contents" entry in the page dictionary. If empty the page is blank.
 * 
 * CPage is a subject that can be observed. This is important when a change
 * leads to content stream reparsing (e.g. deleting an entry from "Contents"
 * property in the page dictionary)
 *
 * Content stream consists of a sequence of operators which should be processed
 * sequentially. The operators define what is really on a page. The pdf
 * specification is too general about pdf operators and that is why working with
 * operators is difficult. According to pdf specification text is split
 * neither to sentences nor words. ALso the letters can occur in the content stream
 * at random because the position is specified absolutely. (e.g. it is very likely
 * that a word "humor" will be split into "hu" "m" "or" because of the "m"
 * beeing wider than other letters.) That makes searching and exporting the text a problem. 
 * We use xpdf code to perform both actions. Xpdf parses a page to lines and
 * words with a rough approuch when a more letters are claimed as one word when
 * they are close enough. This algorithm is working ok for normal pdf files, but
 * if the pdf creator would like to disable text exporting it could produce such
 * set of pdfoperators, that nobody could tell if they form a word or not.
 *
 * Pdf operators are in one or more streams. Problem with this
 * approach is that these operators can be split
 * into streams at almost arbitrary place.
 *
 * Processing pdf operators can be very expensive so they are parsed only on demand. Each operator 
 * can be placed in a bounding box. These bounding boxes are used when searching
 * the page for a text, selecting objects, drawing the page. 
 *
 * Each page content stream is a selfcontained entity that can not
 * use resources defined in another page. It can only use inherited resources
 * from a parent in the page tree. Which means we can not simply change fonts
 * on a page to match another page, use images from another page etc.
 */
class CPage : public noncopyable, public CPageObserverSubject
{
public:
	typedef std::vector<boost::shared_ptr<CContentStream> > ContentStreams;

	/** type for annotation storage. */
	typedef std::vector<boost::shared_ptr<CAnnotation> > AnnotStorage;

private:

	/** Pdf dictionary representing a page. */
	boost::shared_ptr<CDict> dictionary;

	/** Class representing content stream. */
	ContentStreams contentstreams;

	/** Actual display parameters. */
	DisplayParams lastParams;
	
	/** Keeps all annotations from this page.
	 *
	 * This structure is synchronized with page dictionary Annots field with
	 * observer (TODO).
	 */
	AnnotStorage annotStorage;

	//
	// Constructors
	//
public:
		
	/** 
	 * Constructor. 
	 * 
	 * @param pageDict Dictionary representing pdf page.
	 */
	CPage (boost::shared_ptr<CDict>& pageDict);

	
	//
	// Annotation observer
	//
private:
	/** Observer implementation for annotation synchronization.
	 */
	class AnnotsWatchDog: public observer::IObserver<IProperty>
	{
		/** Page owner of this observer.
		 */
		CPage * page;

	public:
		/** Initialization constructor.
		 * @param _page CPage instance.
		 *
		 * Sets page field according parameter.
		 */
		AnnotsWatchDog(CPage * _page):page(_page)
		{
			// given parameter must be non NULL
			// this is used only internaly by CPage, so assert is enough for
			// checking
			assert(_page);
		}

		/** Empty destructor.
		 */
		virtual ~AnnotsWatchDog() throw(){}; 
		
		/** Observer handler.
		 * @param newValue New value of changed property.
		 * @param context Context of the change.
		 *
		 *
		 */
		virtual void notify (boost::shared_ptr<IProperty> newValue, 
							 boost::shared_ptr<const IProperty::ObserverContext> context) const throw();

		/** Reurns observer priority.
		 */
		virtual observer::IObserver<IProperty>::priority_t getPriority()const throw()
		{
			// TODO some constant
			return 0;
		}
	};

	/** Annotations watch dog observer.
	 *
	 * This instance is used to handle changes in Annots array of the page.
	 */
	boost::shared_ptr<AnnotsWatchDog> annotsWatchDog;

	/** Registers AnnotsWatchDog observer.
	 *
	 * Obsever is registered to Annots array (if present) and all its 
	 * (reference) elements. Method is called in constructor.
	 */
	void registerAnnotsWatchDog();
	
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
	/** 
	 * Equality operator. 
	 *
	 * @param page Another page object.
	 */
	bool operator== (const CPage& page)
	{
		return (this == &page) ? true : false;
	};
	

	//
	// Get methods
	//	
public:
	
	/**
	 * Get the dictionary representing this object.
	 *
	 * @return Dictionary.
	 */
	boost::shared_ptr<CDict> getDictionary () const { return dictionary; };
	
	
	/**
	 * Get pdf operators at specified position.
	 * This call will be delegated to content stream object.
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
	 * Get pdf operators at specified position. 
	 * This call will be delegated to content stream object.
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
	 * Get pdf operators at specified position. 
	 * This call will be delegated to content stream object.
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
	 * @param container Output container of all contentstreams.
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
	 * Returns plain text extracted from a page using xpdf code.
	 * 
	 * This method uses xpdf TextOutputDevice that outputs a page to a text device.
	 * Text in a pdf is stored neither word by word nor letter by letter. It is not
	 * easy not decide whether two letters form a word. Xpdf uses insane
	 * algorithm that works most of the time.
	 *
	 * @param text Output string  where the text will be saved.
	 */
 	void getText (std::string& text) const;

	//
	// Annotations
	//
public:
	/** 
	 * Fills given container with all page's annotations.
	 * 
	 * Copies annotStorage content to given container (which is cleared at
	 * first).
	 * <br>
	 * Given container must support clear and insert operations and store
	 * shared_ptr&lt;CAnnotation$gt; elements. 
	 * 
	 * @param container Container which is filled in.
	 */
	template<typename T>
	void getAllAnnotations(T  & container)const
	{
		container.clear();	
		container.insert(container.begin(), annotStorage.begin(), annotStorage.end());
	}

	/** 
	 * Adds new annotation to this page.
	 * 
	 * Inserts deep copy of given annotation and stores its reference to Annots
	 * array in page dictionary (if this doesn't exist, it is created). 
	 * User has to call getAllAnnotations to get current annotations state (we 
	 * don't have identifier for annotations - there are some mechanisms how to 
	 * do it according pdf specification, but there is no explicit identifier).
	 * <br>
	 * Given annotation may come from different CPdf or may belong to nowhere.
	 * <br>
	 * As a result annotStorage is updated. New indirect object representing
	 * annotation dictionary is added to same pdf (dictionary is same as given
	 * one except P field is updated to contain correct reference to this page).
	 * <br>
	 * Note that this page must belong to pdf and has to have valid indirect
	 * reference. This is neccessary, because annotation is indirect object page
	 * keeps reference to it. Reference without pdf doesn't make sense.
	 *
	 * @param annot Annotation to add.
	 * @throw CObjInvalidObject if this page doesn't have valid pdf or indirect
	 * reference.
	 * @throw ElementBadTypeException if Annots field from page dictionary is
	 * not an array (or reference with array indirect target).
	 */ 
	void addAnnotation(boost::shared_ptr<CAnnotation> annot);

	/** Removes given annotation from page.
	 * @param annot Annotation to remove.
	 *
	 * Tries to find given annotation in annotStorage and if found, removes
	 * reference from Annots array.
	 * <br>
	 * As a result, removed annotation is invalidated and not accessible. User 
	 * has to call getAllAnnotations method to get current state (same way as 
	 * in addAnnotation case).
	 *
	 * @return true if annotation was removed.
	 */
	bool delAnnotation(boost::shared_ptr<CAnnotation> annot);


	//
	// Font 
	//
public:
	/**
	 * Get all font ids and base names that are in the resource dictionary of a page.
	 *
	 * The resource can be inherited from a parent in the page tree dictionary.
	 * Base names should be human readable or at least standard system fonts
	 * defined in the pdf specification. We
	 * must choose from these items to make a font change valid. Otherwise, we
	 * have to add a standard system font or manually a font object.
	 *
	 * @param cont Output container of font id and basename pairs.
	 */
	template<typename Container>
	void getFontIdsAndNames (Container& cont) const
	{
		// Clear container
		cont.clear ();
		
		try {
			
			// \todo Resources is an inheritable property
			boost::shared_ptr<CDict> res = utils::getCDictFromDict (dictionary, "Resources");
			boost::shared_ptr<CDict> fonts = utils::getCDictFromDict (res, "Font");
			typedef std::vector<std::string> FontNames;
			FontNames fontnames;
			// Get all font names (e.g. R14, R15, F19...)
			fonts->getAllPropertyNames (fontnames);
			// Get all base names (Symbol, csr12, ...)
			for (FontNames::iterator it = fontnames.begin(); it != fontnames.end(); ++it)
			{
				boost::shared_ptr<CDict> font = utils::getCDictFromDict (fonts, *it);
				try {
					std::string fontbasename;
					
					if (font->containsProperty ("BaseFont")) // Type{1,2} font
						fontbasename = utils::getNameFromDict (font, "BaseFont");
					else									// TrueType font
						fontbasename = utils::getNameFromDict (font, "SubType");
					cont.push_back (std::make_pair (*it, fontbasename));

				}catch (ElementNotFoundException&)
				{}
			}

		}catch (ElementNotFoundException&)
		{
			kernelPrintDbg (debug::DBG_ERR, "No resource dictionary.");
		}
	}
	
	/**
	 * Add new simple type 1 font item to the page resource dictionary. 
	 *
	 * The id of this font is arbitrary but it has to be unique. Itwill be generated as folows: 
	 * PdfEditor for the first item, PdfEditorr for the second, PdfEditorrr for
	 * the third etc.
	 *
	 * We supposed that the font name is a standard system font avaliable to all viewers.
	 *
	 * @param fontname Output container of pairs of (Id,Name).
	 */
	void addSystemType1Font (const std::string& fontname);


	//
	// Helper methods
	//
public:	
	/**
	 * Draw page on an output device.
	 *
	 * We use xpdf code to draw a page. It uses insane global parameters and
	 * many local paramters.
	 *
	 * @param out Output device.
 	 * @param params Display parameters.
	 */
	void displayPage (::OutputDev& out, const DisplayParams params); 
	
	/**
	 * Draw page on an output device with last used display parameters.
	 *
	 * @param out Output device.
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

	/**
	 * Reparse content stream using actual display paramters. 
	 */
	void reparseContentStream ();


	/**
	 * Add new content stream. This function adds new entry in the "Contents"
	 * property of a page. The container of provided operators must form a valid
	 * contentstream.
	 *
	 * This function can be used to separate our complex objects from other
	 * content streams co a set of objects can be easily recognized.
	 *
	 * @param container Container of operators to add.
	 */
	template<typename Container>
	void addContentStream (const Container& cont)
	{
		assert (hasValidPdf(dictionary));
		assert (hasValidRef(dictionary));
		if (!hasValidPdf(dictionary) || !hasValidRef(dictionary))
		{
			throw CObjInvalidObject ();
		}

		// If not parsed
		if (contentstreams.empty())
			parseContentStream ();		
		
		CPdf* pdf = dictionary->getPdf();
		IndiRef ref = dictionary->getIndiRef();
		
		// Create stream with one default property Length
		boost::shared_ptr<CStream> newstr (new CStream());
		
		//
		// Get string representation of new content stream
		//
		typename Container::const_iterator it = cont.begin();
		std::string str;
		for (; it != cont.end(); ++it)
		{
			std::string tmpop;
			(*it)->getStringRepresentation (tmpop);
			str += tmpop;
		}
		kernelPrintDbg (debug::DBG_DBG, str);
	
		// Set the stream
		newstr->setBuffer (str);

		//
		// Change the "Contents" entry 
		//
		// Set ref and indiref reserve free indiref for the new object
		IndiRef newref = pdf->addIndirectProperty (newstr);
		newstr = IProperty::getSmartCObjectPtr<CStream> (pdf->getIndirectProperty (newref));
		assert (newstr);

		//
		// Make valid array of stream references, add new to the beginning
		// otherwise we do not know gfx state 
		// 
		if (dictionary->containsProperty ("Contents"))
		{
			boost::shared_ptr<IProperty> contents = utils::getReferencedObject(dictionary->getProperty("Contents"));
			if (isStream(contents))
			{ // Exactly one stream
				
				CArray streamrefs;
				
				// Add new one
				CRef newref (newstr->getIndiRef());
				streamrefs.addProperty (newref);

				// Add current one
				CRef tmp (contents->getIndiRef());
				streamrefs.addProperty (tmp);
	
				// Set new array to the "Contents" entry
				dictionary->setProperty ("Contents", streamrefs);
		
			}else
			{ // Array
				
				assert (isArray(contents));
				if (!isArray(contents))
					throw CObjInvalidObject ();

				// Insert new one before current ones
				CRef newref (newstr->getIndiRef());
				IProperty::getSmartCObjectPtr<CArray> (contents)->addProperty (0,newref);
			}

		}else
		{
			// Make valid array of stream references
			CArray streamrefs;
	
			// Add new one
			CRef newref (newstr->getIndiRef());
			streamrefs.addProperty (newref);
				
			// Add the new array
			dictionary->addProperty ("Contents", streamrefs);
		}
		
		
		// Parse new stream to content stream and add it to the streams
		CContentStream::CStreams streams;
		streams.push_back (newstr);
		boost::shared_ptr<GfxResources> res;
		boost::shared_ptr<GfxState> state;
		createXpdfDisplayParams (res, state);
		ContentStreams _tmp;
		_tmp.push_back(boost::shared_ptr<CContentStream> (new CContentStream(streams,state,res)));
		std::copy (contentstreams.begin(), contentstreams.end(), std::back_inserter(_tmp));
		contentstreams = _tmp;

		// Indicate change
		_objectChanged ();
	}

	
	//
	// Page translation 
	//
public:
	/**
	 * Set tranform matrix of a page. This operator will be preceding first cm
	 * operator, if not found it will be the first operator.
	 *
	 * @param tm Six number representing transform matrix.
	 */
	void setTransformMatrix (double tm[6]);
	
	
	//
	// Media box 
	//
public:
	/**  
	 * Return media box of this page. 
	 *
	 * It is a required item in page dictionary (spec p.119) but can be
	 * inherited from a parent in the page tree.
	 *
	 * @return Rectangle specifying the box.
	 */
	 Rectangle getMediabox () const;

	 
	/**  
	 * Set media box of this page. 
	 * 
	 * @param rc Rectangle specifying the page metrics.
	 */
	 void setMediabox (const Rectangle& rc);

	 //
	 // Rotation
	 //
public:
	 /**
	  * Get rotation.
	  *
	  * @return Rotation degree measurment.
	  */
	 int getRotation () const;
	
	 /**
	  * Set rotation.
	  *
	  * @param rot Set rotation degree measurment.
	  */
	 void setRotation (int rot);
 
	 //
	 // Text search/find 
	 //
public:
	 /**
	  * Find all occurences of a text on this page.
	  *
	  * It uses xpdf TextOutputDevice function to get the bounding box of a
	  * found text.
	  *
	  * @param text Text to find.
	  * @param recs Output container of rectangles of all occurences of the text.
	  * @param params Search parameters.
	  *
	  * @return Number of occurences found.
	  */
	 template<typename RectangleContainer>
	 size_t findText (std::string text, 
					  RectangleContainer& recs, 
					  const TextSearchParams& params = TextSearchParams()) const;
	
private:
	 /**
	  * Create xpdf's state and res paramters.
	  *
	  * @param res Gfx resource paramter.
	  * @param state Gfx state parameter.
	  */
	 void createXpdfDisplayParams (boost::shared_ptr<GfxResources>& res, boost::shared_ptr<GfxState>& state);

	
private:
	/**
	 * Save changes and indicate that the object has changed by calling all
	 * observers.
	 */
	void _objectChanged ();

};


//
// Helper functions
//

/**
 * Check whether iproperty claimed to be a page is conforming to the pdf specification.
 * (p.118)
 *
 * @param ip IProperty.
 */
bool isPage (boost::shared_ptr<IProperty> ip);


//=====================================================================================
} // namespace pdfobjects
//=====================================================================================


#endif // _CPAGE_H
