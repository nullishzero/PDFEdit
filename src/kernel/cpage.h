/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006-2009  PDFedit team: Michal Hocko,
 *                                        Jozef Misutka,
 *                                        Martin Petricek
 *                   Former team members: Miroslav Jahoda
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (in doc/LICENSE.GPL); if not, write to the 
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 * MA  02111-1307  USA
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit
 */
// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80

#ifndef _CPAGE_H
#define _CPAGE_H

// all basic includes
#include "kernel/static.h"

#include "kernel/textsearchparams.h"// TextSearchParams 
#include "kernel/displayparams.h"	// DisplayParams
#include "kernel/cobject.h"			// CDict
#include "kernel/cpagecontents.h"	// 
#include "kernel/cpagechanges.h"	// 
#include "kernel/cpageannots.h"		// 
#include "kernel/cpagefonts.h"

//=====================================================================================
namespace pdfobjects {
//=====================================================================================


//=====================================================================================
// CPage
//=====================================================================================

// Forward declaration
class CPage;
class CPageDisplay;


/**
 * This object represents page object from pdf specification v1.5. Pdf page object is a dictionary
 * reachable from page tree structure with several required properties. 
 * It is responsible just for one single page.
 *
 * Every pdf page contains all information required for displaying the page
 * (e.g. page metrics, page contents etc.) Page properties can be inherited from
 * its parent in the page tree. The first encountered during page tree traversal
 * is used. This feature can
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
 * neither to sentences nor words. Letters of a word can occur randomly in the content stream
 * because the position of a letter (text) is absolute. (e.g. it is very likely
 * that a word "humor" will be split into "hu" "m" "or" because of the "m"
 * beeing wider than other letters.) This makes searching and exporting page text a problem. 
 * We use xpdf code to perform both actions. Xpdf parses a page to lines and
 * words with a rough approuch when a more letters are claimed as one word when
 * they are close enough. This algorithm is working ok for normal pdf files, but
 * if the pdf creator would like to disable text exporting it could produce such
 * sequence of pdfoperators, that hardly any program could export text correctly.
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
 * use resources defined in another page. It can use only inherited resources
 * from a parent in the page tree. Which means we can not simply change fonts
 * on a page to match another page, use images from another page etc.
 */
class CPage : public noncopyable, public observer::ObserverHandler<CPage>
{
	// Typedefs
public:
	typedef CPageFonts::FontList FontList;
	typedef CPageAnnots::Annotations Annotations;
	/** Type of page observer context. */
	typedef observer::BasicChangeContext<CPage> BasicObserverContext;
	typedef std::vector<boost::shared_ptr<ICPageModule> > Modules;

	// Friends
public:
	friend class CPageContents;
	friend class CPageDisplay;
	friend class CPageFonts;
	friend class CPageChanges;
	//friend class CPageAnnots;

	// Variables
private:
	/** Pdf dictionary representing a page. */
	boost::shared_ptr<CDict> _dict;
	
	/** Is page valid.
	 * Modifications of page object are not allowed and no observers
	 * can be registered on this object if the flag is false.
	 */
	bool _valid;

	// Modules
	Modules _modules;
	// Specific modules
	/** Object managing Contents entry. */
	boost::shared_ptr<CPageContents> _contents;
	/** Object managing Contents entry. */
	boost::shared_ptr<CPageDisplay> _display;
	/** Object managing Contents entry. */
	boost::shared_ptr<CPageFonts> _fonts;
	/** Object managing changes. */
	boost::shared_ptr<CPageChanges> _changes;
	/** Object managing annotations. */
	boost::shared_ptr<CPageAnnots> _annots;


	//
	// Constructor
	//
public:
		
	/** 
	 * Constructor. 
	 * 
	 * @param pageDict Dictionary representing pdf page.
	 */
	CPage (boost::shared_ptr<CDict>& pageDict);

	//
	// Destructor
	//
public:
	
	/** Destructor. */
	~CPage ();
	

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
		{ return (this == &page) ? true : false; }

	
	//
	// Module Getters & Setters
	//	
private:
	
	/** Returns the contents module.*/
	boost::shared_ptr<CPageContents> contents () const 
		{ return _contents; }
	/** Returns the display module.*/
	boost::shared_ptr<CPageDisplay> display () const 
		{ return _display; }
	/** Returns the fonts module.*/
	boost::shared_ptr<CPageFonts> fonts () const 
		{ return _fonts; }
	/** Returns the changes module.*/
	boost::shared_ptr<CPageChanges> changes () const 
		{ return _changes; }
	/** Returns the annotation module.*/
	boost::shared_ptr<CPageAnnots> annotations () const 
		{ return _annots; }
	

	//
	// Invalidate page
	//
public:

	/**
	 * Inform all obsevers that this page is not valid.
	 */
	void invalidate ();
	

	//
	// Setters & Getters
	//	
public:
	
	/** Returns the dictionary representing this object.*/
	boost::shared_ptr<CDict> getDictionary () const 
		{ return _dict; }

	/** Returns page position. */
	size_t getPagePosition () const;


	//
	// Annotations
	//
public:

	/** 
	 * Fills given container with all page's annotations.
	 * 
	 * Copies _annotations content to given container (which is cleared at
	 * first).
	 * <br>
	 * Given container must support clear and insert operations and store
	 * shared_ptr&lt;CAnnotation$gt; elements. 
	 * 
	 * @param container Container which is filled in.
	 */
	template<typename T>
	void getAllAnnotations(T& container)const
		{ _annots->getAll (container); }

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
	 * As a result _annotations is updated. New indirect object representing
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
	void addAnnotation(boost::shared_ptr<CAnnotation> annot)
		{ _annots->add (annot); }


	/** 
	 * Removes given annotation from page.
	 *
	 * Tries to find given annotation and if found, removes it.<br />
	 *
	 * As a result, the removed annotation is invalidated and not accessible. User 
	 * has to call getAllAnnotations method to get current state (the same way as 
	 * in addAnnotation case).
	 *
	 * @param annot Annotation to remove.
	 * @return true if annotation was removed.
	 */
	bool delAnnotation(boost::shared_ptr<CAnnotation> annot)
		{ return _annots->del (annot); }
		

	//
	// Contents module delegation
	//	
public:

	/** Returns shared pointer to the specified content stream. */
	boost::shared_ptr<CContentStream> getContentStream (CContentStream* cc) 
		{ return _contents->getContentStream (cc); }

	/** Fills container with contents streams. */
	template<typename Container> 
	void getContentStreams (Container& container)
		{ _contents->getContentStreams (container); }


	/** Get pdf operators at position specified by rectangle. @see getObjectsAtPosition() */
	template<typename OpContainer>
	void getObjectsAtPosition  (OpContainer& opContainer, const libs::Rectangle& rc)
		{ getObjectsAtPosition (opContainer, PdfOpCmpRc(rc)); }

	/** Get pdf operators at position specified by point. @see getObjectsAtPosition() */
	template<typename OpContainer>
	void getObjectsAtPosition  (OpContainer& opContainer, const Point& pt)
		{ getObjectsAtPosition (opContainer, PdfOpCmpPt(pt)); }
	
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
			_check_validity();
		_contents->getObjectsAtPosition (opContainer, cmp);
	}


	//	
	// Conversion methods
	//
public:
	
	/**
	 * Get text source of a page.
	 */
	template<typename WordEngine,typename LineEngine,typename ColumnEngine>
 	void convert (textoutput::OutputBuilder& out)
		{ _contents->convert<WordEngine, LineEngine, ColumnEngine> (out); }


	//
	// Font module delegation
	//
public:
	/**
	 * Get all font ids and base names that are in the resource dictionary of a page.
	 *
	 * The resource can be inherited from a parent in the page tree dictionary.
	 * Base names should be human readable or at least standard system fonts
	 * defined in the pdf specification. We
	 * must choose from these items to make a font change valid. Otherwise, we
	 * have to add standard system font or manually a font object.
	 *
	 * @param cont Output container of font id and basename pairs (FontList
	 * container type should be prefered).
	 */
	void getFontIdsAndNames (FontList& cont) const;
	
	/**
	 * Add new simple type 1 font item to the page resource dictionary. 
	 *
	 * The id of this font is arbitrary but it has to be unique.
	 * It will be generated as PDFEDIT_F#, where # is the lowest 
	 * free number so that name is unique.
	 *
	 * We supposed that the font name is a standard system font avaliable 
	 * to all viewers.
	 *
	 * @param fontname Name of the font to add.
	 * @param winansienc Set encoding to standard WinAnsiEnconding.
	 *
	 * @return The font ID of the added font.
	 */
	std::string addSystemType1Font (const std::string& fontname, bool winansienc = true);


	//
	// Display module delegation
	//
public:

	/** 
	 * Returns rotation in degrees.
	 */
	int getRotation () const;

	/** 
	 * Sets rotation in degrees. 
	 */
	void setRotation (int rot);

	/**  
	 * Return media box of this page. 
	 *
	 * It is a required item in page dictionary (spec p.119) but can be
	 * inherited from a parent in the page tree.
	 *
	 * @return Rectangle specifying the box.
	 */
	libs::Rectangle getMediabox () const;
	
	/** Seta media box of this page. */
	void setMediabox (const libs::Rectangle& rc);

	/**
	 * Set transform matrix of a page. This operator will be preceding first cm
	 * operator (see pdf specification), if not found it will be the first operator.
	 *
	 * @param tm Six number representing transform matrix.
	 */
	void setTransformMatrix (double tm[6]);

	/**
	 * Set display params.
	 */
	void setDisplayParams (const DisplayParams& dp);

	/**
	 * Draw page on an output device.
	 *
	 * We use xpdf code to draw a page. It uses insane global parameters and
	 * many local parameters.
	 *
	 * @param out Output device.
 	 * @param params Display parameters.
	 */
	void displayPage (::OutputDev& out, const DisplayParams& params, int x = -1, int y = -1, int w = -1, int h = -1);
	
	/**
	 * Draw page on an output device.
	 * Use old display params.
	 */
	void displayPage (::OutputDev& out, int x = -1, int y = -1, int w = -1, int h = -1);

	/**
	 * Draw page on an output device with last used display parameters.
	 *
	 * @param out Output device.
	 * @param dict If not null, page is created from dict otherwise
	 * this page dictionary is used. But still some information is gathered from this page dictionary.
	 */
	void displayPage (::OutputDev& out, 
					  boost::shared_ptr<CDict> dict = boost::shared_ptr<CDict> (), 
					  int x = -1, int y = -1, int w = -1, int h = -1) const;


	//
	// CPageContents module delegation
	//
public:

	/**
	 * Add new content stream to the front. This function adds new entry in the "Contents"
	 * property of a page. The container of provided operators must form a valid
	 * contentstream.
	 * This function should be used when supplied operators
	 * should be handled at the beginning end e.g. should be drawn first which means
	 * they will appear the "below" other object.
	 *
	 * This function can be used to separate our changes from original content stream.
	 *
	 * @param cont Container of operators to add.
	 */
	template<typename Container> 
	void addContentStreamToFront (const Container& cont)
		{ _contents->addToFront (cont); }
		
	/**
	 * Add new content stream to the back. This function adds new entry in the "Contents"
	 * property of a page. The container of provided operators must form a valid
	 * contentstream. 
	 * This function should be used when supplied operators
	 * should be handled at the end e.g. should be drawn at the end which means
	 * they will appear "above" other objects.
	 *
	 * This function can be used to separate our changes from original content stream.
	 *
	 * @param cont Container of operators to add.
	 */
	template<typename Container> 
	void addContentStreamToBack (const Container& cont)
		{ _contents->addToBack (cont); }

	/**
	 * Remove content stream. 
	 * This function removes all objects from "Contents" entry which form specified contentstream.
	 *
	 * @param csnum Number of content stream to remove.
	 */
	void removeContentStream (size_t csnum)
		{ _contents->remove (csnum); }


	/**  
	 * Returns plain text extracted from a page using xpdf code.
	 * 
	 * @param text Output string  where the text will be saved.
	 * @param encoding Encoding format.
	 * @param rc Rectangle from which to extract the text.
	 */
	void getText (std::string& text, const std::string* encoding = NULL, const libs::Rectangle* rc = NULL) const
		{ _contents->getText (text, encoding, rc); }
 
	 /**
	  * Find all occurences of a text on this page.
	  *
	  * It uses xpdf TextOutputDevice to get the bounding box of found text.
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
					  const TextSearchParams& params = TextSearchParams()) const
		{ return _contents->findText (text, recs, params);	}

	/**
	 * Move contentstream up one level. Which means it will be repainted by less objects.
	 */
	void moveAbove (boost::shared_ptr<const CContentStream> ct)
		{ _contents->moveAbove (ct); }
	void moveAbove (size_t pos)
		{ _contents->moveAbove (pos); }

	/**
	 * Move contentstream below one level. Which means it will be repainted by more objects.
	 */
	void moveBelow (boost::shared_ptr<const CContentStream> ct)
		{ _contents->moveBelow (ct); }
	void moveBelow (size_t pos)
		{ _contents->moveBelow (pos); }


	//
	// PdfEdit changes 
	//
public:
	/**
	 * Get n-th change.
	 * Higher change means older change.
	 */
	boost::shared_ptr<CContentStream> getChange (size_t nthchange = 0) const
		{ return _changes->getChange (nthchange); }

	/**
	 * Get our changes sorted.
	 * The first change is the last change. If there are no changes
	 * container is empty.
	 */
	template<typename Container> 
	void getChanges (Container& cont) const
		{ _changes->getChanges (cont); }

	/**
	 * Get count of our changes.
	 */
	size_t getChangeCount () const
		{ return _changes->getChangeCount (); }

	/**
	 * Draw nth change on an output device with last used display parameters.
	 *
	 * @param out Output device.
	 * @param cont Container of content streams to display
	 */
	template<typename Container>
	void displayChange (::OutputDev& out, const Container& cont) const
		{ _changes->displayChange (out, cont); }
	void displayChange (::OutputDev& out, const std::vector<size_t> cs) const
		{ _changes->displayChange (out, cs); }


	//
	// 
	//
public:

	/**
	 * Replaces text in the whole page.
	 */
	void replaceText (const std::string& what, const std::string& with)
	{
			_check_validity();
		_contents->replaceText (what, with);
	}

	/**
	 * Adds text to specified position.
	 */
	void addText (const std::string& what, 
				  const libs::Point& where, 
				  const std::string& font_id)
	{
			_check_validity();
		_contents->addText (what, where, font_id);
	}

	void addInlineImage (const CStream::Buffer& what,
  						 const libs::Point& dim,
						 const libs::Point& where)
	{
			_check_validity();
		_contents->addInlineImage (what, dim, where);
	}
	 //
	 // Helper functions
	 //
private:

	// Save changes and indicate that the object has changed by calling all observers.
	// If invalid is true indicate that this page has been invalidated.
	void _objectChanged (bool invalid = false);

	// check for valid object
	inline bool _check_validity (const char* err = NULL) const
	{
		if (!_valid || !hasValidPdf(_dict) || !hasValidRef(_dict))
		{	
			if (err)
				kernelPrintDbg (debug::DBG_ERR, err);
			throw CObjInvalidObject ();
		}
		return true;
	}
public:
	/** Returns page object valid flag value.
	 * @return true if page object is valid for modifications, false 
	 * otherwise.
	 */
	bool isValid()const
	{
		return _valid;
	}

}; // class CPage




//=====================================================================================
// Helper functions
//=====================================================================================

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
