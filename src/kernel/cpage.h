/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006, 2007, 2008  PDFedit team: Michal Hocko,
 *                                              Miroslav Jahoda,
 *                                              Jozef Misutka,
 *                                              Martin Petricek
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
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

// TextSearchParams 
#include "kernel/textsearchparams.h"
// DisplayParams
#include "kernel/displayparams.h"

// CDict
#include "kernel/cobject.h"
// CContentstream
#include "kernel/ccontentstream.h"
// CAnnotation
#include "kernel/cannotation.h"
// Text output
#include "kernel/textoutput.h" 
// Factories
#include "kernel/factories.h"
// State updater
#include "kernel/stateupdater.h"

//=====================================================================================
namespace pdfobjects {
//=====================================================================================



//=====================================================================================
// CPage
//=====================================================================================

//
// Forward declaration
//
class CPage;


//
// Typedefs
//
typedef observer::ObserverHandler<CPage> CPageObserverSubject;

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
class CPage : public noncopyable, public CPageObserverSubject
{
public:
	/** Container of content streams. */
	typedef std::vector<boost::shared_ptr<CContentStream> > ContentStreams;
	/** Type for annotation storage. */
	typedef std::vector<boost::shared_ptr<CAnnotation> > AnnotStorage;
	/** Position in content stream container. */
	typedef size_t CcPosition;
	
	/** Type of page observer context. */
	typedef observer::BasicChangeContext<CPage> BasicObserverContext;

private:

	/** Pdf dictionary representing a page. */
	boost::shared_ptr<CDict> dictionary;

	/** Class representing content stream. */
	ContentStreams contentstreams;

	/** Actual display parameters. */
	DisplayParams lastParams;

	/** Is page valid. */
	bool valid;
	
	/** Keeps all annotations from this page.
	 *
	 * This structure is synchronized with page dictionary Annots field with
	 * observer.
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

	/** 
	 * Consolidates annotStorage field according given change.
	 * Works in two steps. First handles oldValue and second newValue. At first
	 * checkes oldValue type and if it is reference, dereference indirect
	 * objects and if it is annotation dictionary, it will invalidate 
	 * associated CAnnotation and removes it from annotStorage. 
	 * <br>
	 * In second step, checks newValue type and if it is reference to
	 * dictionary, it will create new CAnnotation instance and adds it to
	 * annotStorage. 
	 * 
	 * @param oldValue Removed reference from annotStorage.
	 * @param newValue Added reference to the annotStorage.
	 */
	void consolidateAnnotsStorage(boost::shared_ptr<IProperty> & oldValue, boost::shared_ptr<IProperty> & newValue);
	
	/** Observer for Annots property.
	 * This observer is registered on page dictionary and if Annots property is
	 * a reference also to this property. Any change which leads to change of 
	 * Annots array (either add, remove or change) is handled here.  Note that
	 * it doesn't handle array content change.
	 */
	class AnnotsPropWatchDog: public IIPropertyObserver
	{
		/** Page owner of this observer.
		 */
		CPage* page;

	public:
		/** Initialization constructor.
		 * Sets page field according parameter.
		 *
		 * @param _page CPage instance.
		 */
		AnnotsPropWatchDog(CPage * _page):page(_page)
		{
			// given parameter must be non NULL
			// this is used only internaly by CPage, so assert is enough for
			// checking
			assert(_page);
		}

		/** Empty destructor.
		 */
		virtual ~AnnotsPropWatchDog() throw(){}
		
		/** Observer handler.
		 * 
		 * Checks given context type:
		 * <ul>
		 * <li>BasicChangeContext means that Annots property in page dictionary
		 * is reference and its value has changed.
		 * <li>ComplexChangeContext means that page dictionary has changed. So
		 * checks property id and if it not Annots, immediatelly returns,
		 * because this change doesn't affect annotations. Otherwise checks
		 * original value type. If it is reference, unregisters this observer
		 * from it. If newValue is reference, registers observer to it.
		 * </ul>
		 * In any case:
		 * <ul>
		 * <li>Tries to get array from oldValue and unregister observers from
		 * it (uses page-&gt;unregisterAnnotsObservers).
		 * <li>Invalidates and removes all annotations from 
		 * page-&gt;annotStorage.
		 * <li>collects all current annotations (uses collectAnnotations).
		 * <li>Tries to get current Annots array and registers observers to it
		 * (uses page-&gt;registerAnnotsObservers)
		 * </ul>
		 *
		 * @param newValue New value of changed property.
		 * @param context Context of the change.
		 */
		virtual void notify (boost::shared_ptr<IProperty> newValue, 
							 boost::shared_ptr<const IProperty::ObserverContext> context) const throw();

		/** Returns observer priority.
		 */
		virtual priority_t getPriority()const throw()
		{
			// TODO some constant
			return 0;
		}
	};
	
	/** 
	 * Observer for Annots array synchronization.
	 * This observer is registered on Annots array property and all its
	 * reference typed elements. It handles change in Annots array content -
	 * this means either element is added, removed or replaced, or any of its
	 * reference elements changes its value.
	 */
	class AnnotsArrayWatchDog: public IIPropertyObserver
	{
		/** Page owner of this observer.
		 */
		CPage* page;

	public:
		typedef observer::BasicChangeContext<IProperty> BasicObserverContext;
		typedef CDict::CDictComplexObserverContext ComplextObserverContext;
			
		/** Initialization constructor.
		 * Sets page field according parameter.
		 *
		 * @param _page CPage instance.
		 */
		AnnotsArrayWatchDog(CPage * _page):page(_page)
		{
			// given parameter must be non NULL
			// this is used only internaly by CPage, so assert is enough for
			// checking
			assert(_page);
		}

		/** Empty destructor.
		 */
		virtual ~AnnotsArrayWatchDog() throw(){}
		
		/** Observer handler.
		 * 
		 * Checks given context type:
		 * <ul>
		 * <li>BasicObserverContext means that Annots array reference element 
		 * has changed its value.
		 * <li>ComplexChangeContext means that Annots array content has changed.
		 * If original value is reference, then unregisters this obsever from
		 * it. If newValue is reference registers this observer to it.
		 * <li>Different context is not supported and so method immediatelly
		 * returns.
		 * </ul>
		 * In both situations calls consolidateAnnotsStorage with original and
		 * new value parameters.
		 *
		 * @param newValue New value of changed property.
		 * @param context Context of the change.
		 */
		virtual void notify (boost::shared_ptr<IProperty> newValue, 
							 boost::shared_ptr<const IProperty::ObserverContext> context) const throw();

		/** Returns observer priority.
		 */
		virtual priority_t getPriority()const throw()
		{
			// TODO some constant
			return 0;
		}
	};

	/** Watchdog for Annots property.
	 * @see AnnotsPropWatchDog
	 */
	boost::shared_ptr<AnnotsPropWatchDog> annotsPropWatchDog;

	/** Watchdog for Annotation array.
	 * @see AnnotsArrayWatchDog
	 */
	boost::shared_ptr<AnnotsArrayWatchDog> annotsArrayWatchDog;

	/** Registers observers for annotations synchronization.
	 * Checks type of given property and if it is reference, registers
	 * annotsPropWatchDog observer to it and dereferences indirect object. If 
	 * annots or dereferenced object is array, registers annotsArrayWatchDog 
	 * observer to it and all its reference type elements.
	 *
	 * @param annots Annots property.
	 */
	void registerAnnotsObservers(boost::shared_ptr<IProperty> & annots);

	/** Unregisters obsevers from given Annots property.
	 * This method works reversely to registerAnnotsObservers (observers are
	 * unregistered but rest is same).
	 * 
	 * @param annots Annots property.
	 */
	void unregisterAnnotsObservers(boost::shared_ptr<IProperty> & annots);


	//
	// CStream observer
	//
private:
	/** 
	 * Observer implementation for content stream synchronization.
	 */
	class ContentsWatchDog: public IIPropertyObserver
	{
		/** 
		 * Owner of this observer.
		 */
		CPage* page;

	public:
		/** Initialization constructor.
		 * Sets page field according parameter.
		 * 
		 * @param _page CPage instance.
		 */
		ContentsWatchDog (CPage* _page) : page(_page)
			{ assert(_page); }

		/** Empty destructor.  */
		virtual ~ContentsWatchDog() throw() {}
		
		/** 
		 * Observer handler.
		 * 
		 * @param 
		 * @param context Context of the change.
		 */
		virtual void notify (boost::shared_ptr<IProperty>, 
							 boost::shared_ptr<const IProperty::ObserverContext>) const throw();

		/** Returns observer priority. */
		virtual priority_t getPriority() const throw()
			{ return 0;	}
	};
	
	/**
	 * Contents observer.
	 */
	boost::shared_ptr<ContentsWatchDog> contentsWatchDog;

protected:
	/**
	 * Register observer on all cstreams that content stream consists of.
	 */
	void registerContentsObserver () const;

	/**
	 * Unregister observer from all cstreams that this object consists of.
	 *
	 * This function is called when saving consten stream consisting of
	 * more streams. If we do not unregister observers, we would be notified
	 * that a stream has changed after the first save (when the content stream
	 * is invalid) and our observer would want to reparse an invalid stream.
	 */
	void unregisterContentsObserver () const;

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
	{
		return (this == &page) ? true : false;
	}

	
	//
	// Invalidate page
	//
public:
	/**
	 * Inform all obsevers that this page is not valid.
	 */
	void invalidate ()
	{ 
		assert (valid);
		// Unregister contents observer
		unregisterContentsObserver ();
		// Unregister annots
		UNREGISTER_SHAREDPTR_OBSERVER(dictionary, annotsPropWatchDog);
		// unregisters annotation observers - if annotation array present in
		// page dictionary
		if(dictionary->containsProperty("Annots"))
		{
			boost::shared_ptr<IProperty> annotsDict=dictionary->getProperty("Annots");
			unregisterAnnotsObservers(annotsDict);
		}

		_objectChanged (true); 
		valid = false;
	}
	
	//
	// Get methods
	//	
public:
	
	/**
	 * Get the dictionary representing this object.
	 *
	 * @return Dictionary.
	 */
	boost::shared_ptr<CDict> getDictionary () const { return dictionary; }
	
	
	/**
	 * Get pdf operators at specified position.
	 * This call will be delegated to content stream object.
	 *
	 * @param opContainer Operator container where operators in specified are
	 * 						wil be stored.
	 * @param rc 		Rectangle around which we will be looking.
	 */
	template<typename OpContainer>
	void getObjectsAtPosition  (OpContainer& opContainer, const libs::Rectangle& rc)
	{	
		kernelPrintDbg (debug::DBG_DBG, " at libs::Rectangle (" << rc << ")");
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
		assert (valid);

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
	 * @param encoding Encoding format.
	 * @param rc Rectangle from which to extract the text.
	 */
 	void getText (std::string& text, const std::string* encoding = NULL, const libs::Rectangle* rc = NULL)  const;

	/**
	 * Get text source of a page.
	 */
	template<typename WordEngine, 
			 typename LineEngine, 
			 typename ColumnEngine 
			 >
 	void convert (textoutput::OutputBuilder& out)
	{
		typedef textoutput::PageTextSource<WordEngine, LineEngine, ColumnEngine> TextSource;
		kernelPrintDbg (debug::DBG_INFO, ""); 

		// If not parsed
		if (contentstreams.empty())
			parseContentStream ();		

		// Create gfx resource and state
		boost::shared_ptr<GfxResources> gfxres;
		boost::shared_ptr<GfxState> gfxstate;
		createXpdfDisplayParams (gfxres, gfxstate);
		assert (gfxres && gfxstate);

		// Create page text class with parametrized parts
		TextSource text_source;

		// Get text from all content streams
		for (ContentStreams::iterator it = contentstreams.begin(); it != contentstreams.end(); ++it)
		{
			// Get operators and build text representation if not empty
			CContentStream::Operators ops;
			(*it)->getPdfOperators (ops);
			if (!ops.empty())
			{
				PdfOperator::Iterator itt = PdfOperator::getIterator (ops.front());
				StateUpdater::updatePdfOperators<TextSource&> (itt, gfxres, *gfxstate, text_source);
			}
		}

		// Create lines, columns...
		text_source.format ();
		// Build the output
		if (hasValidPdf(dictionary))
			text_source.output (out, getPagePosition());
		else
			text_source.output (out, 0);
	}


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
		assert (valid);
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
	 * have to add standard system font or manually a font object.
	 *
	 * @param cont Output container of font id and basename pairs (FontList
	 * container type should be prefered).
	 */
	template<typename Container>
	void getFontIdsAndNames (Container& cont) const;
	
	/** Type for list of fonts. */
	typedef std::vector<std::pair<std::string, std::string> > FontList;

	/** Looks for a font with the given name.
	 * @param container Container of fonts (filled with getFontIdsAndNames).
	 * @param name Name of the font.
	 * @return iterator to the container (container.end() if not found).
	 */
	FontList::const_iterator findFont(const FontList &containter,
			const std::string & name)
	{
		for(FontList::const_iterator i=containter.begin();
				i!=containter.end(); ++i)
			if(i->first == name)
				return i;
		return containter.end();
	}

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
	std::string addSystemType1Font (const std::string& fontname, 
			bool winansienc = true);


	//
	// Helper methods
	//
public:	
	/**
	 * Get page position.
	 */
	size_t getPagePosition () const;

	/**
	 * Set display params.
	 */
	void setDisplayParams (const DisplayParams& dp)
	{ 
		lastParams = dp; 
		// set rotate to positive integer
		lastParams.rotate -= ((int)(lastParams.rotate / 360) -1) * 360;
		// set rotate to range [ 0, 360 )
		lastParams.rotate -= ((int)lastParams.rotate / 360) * 360;
		// Use mediabox
		if (lastParams.useMediaBox)
			lastParams.pageRect = getMediabox ();
		// Change bbox etc...
		reparseContentStream ();
	}

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
	void displayPage (::OutputDev& out, boost::shared_ptr<CDict> dict = boost::shared_ptr<CDict> (), int x = -1, int y = -1, int w = -1, int h = -1) const;

	/**
	 * Parse content stream. 
	 * Content stream is an optional property. When found it is parsed,
	 * nothing is done otherwise.
	 *
	 * @return True if content stream was found and was parsed, false otherwise.
	 */
	bool parseContentStream ();

	/**
	 * Reparse content stream using actual display parameters. 
	 */
	void reparseContentStream ();

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
	template<typename Container> void addContentStreamToFront (const Container& cont);
	
	
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
	template<typename Container> void addContentStreamToBack (const Container& cont);

	/**
	 * Remove content stream. 
	 * This function removes all objects from "Contents" entry which form specified contentstream.
	 *
	 * @param csnum Number of content stream to remove.
	 */
	void removeContentStream (size_t csnum);

	//
	// Page translation 
	//
public:
	/**
	 * Set transform matrix of a page. This operator will be preceding first cm
	 * operator (see pdf specification), if not found it will be the first operator.
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
	 libs::Rectangle getMediabox () const;

	 
	/**  
	 * Set media box of this page. 
	 * 
	 * @param rc Rectangle specifying the page metrics.
	 */
	 void setMediabox (const libs::Rectangle& rc);

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
					  const TextSearchParams& params = TextSearchParams()) const;

	 //
	 // Helper functions
	 //
public:
	 /**
	  * Return shared pointer to the content stream.
	  * @param cc Raw ccontentstream pointer.
	  */
	  boost::shared_ptr<CContentStream> 
	  getContentStream (CContentStream* cc) 
	  {
		  for (ContentStreams::iterator it = contentstreams.begin(); it != contentstreams.end(); ++it)
			  if ((*it).get() == cc)
				  return *it;
		  
		  assert (!"Contentstream not found");
		  throw CObjInvalidOperation ();
	  }
	 
private:
	 /**
	  * Create xpdf's state and resource parameters.
	  *
	  * @param res Gfx resource parameter.
	  * @param state Gfx state parameter.
	  */
	 void createXpdfDisplayParams (boost::shared_ptr<GfxResources>& res, boost::shared_ptr<GfxState>& state);

	
private:
	/**
	 * Save changes and indicate that the object has changed by calling all
	 * observers.
	 *
	 * @param invalid If true indicate that this page has been invalidated.
	 */
	void _objectChanged (bool invalid = false);


	//
	// PdfEdit changes 
	//
public:
	/**
	 * Get n-th change.
	 * Higher change means older change.
	 */
	boost::shared_ptr<CContentStream> getChange (size_t nthchange = 0) const;

	/**
	 * Get our changes sorted.
	 * The first change is the last change. If there are no changes
	 * container is empty.
	 */
	template<typename Container>
	void getChanges (Container& cont) const;

	/**
	 * Get count of our changes.
	 */
	size_t getChangeCount () const;

	/**
	 * Draw nth change on an output device with last used display parameters.
	 *
	 * @param out Output device.
	 * @param cont Container of content streams to display
	 */
	template<typename Container>
	void displayChange (::OutputDev& out, const Container& cont) const;

	void displayChange (::OutputDev& out, const std::vector<size_t> cs) const
	{
		ContentStreams css;
		for (std::vector<size_t>::const_iterator it = cs.begin(); it != cs.end(); ++it)
		{
			if (static_cast<size_t>(*it) >= contentstreams.size())
				throw CObjInvalidOperation ();
			css.push_back (contentstreams[*it]);
		}
		displayChange (out, css);
	}

	/**
	 * Move contentstream up one level. Which means it will be repainted by less objects.
	 */
	void moveAbove (boost::shared_ptr<const CContentStream> ct);
	void moveAbove (CcPosition pos)
	{ 
		if (pos >= contentstreams.size())
			throw OutOfRange();
		moveAbove (contentstreams[pos]); 
	}

	/**
	 * Move contentstream below one level. Which means it will be repainted by more objects.
	 */
	void moveBelow (boost::shared_ptr<const CContentStream> ct);
	void moveBelow (CcPosition pos)
	{ 
		if (pos >= contentstreams.size())
			throw OutOfRange();
		moveBelow (contentstreams[pos]); 
	}

	
};


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


/** Sets unitialized inheritable page attributes.
 * @param pageDict Page dictionary reference where to set values.
 *
 * Gets InheritedPageAttr structure for given pageDict (uses
 * fillInheritedPageAttr helper function) and sets all fields which are not
 * present in given dictionary to found values.
 */
void setInheritablePageAttr(boost::shared_ptr<CDict> & pageDict);



//=====================================================================================
} // namespace pdfobjects
//=====================================================================================


#endif // _CPAGE_H
