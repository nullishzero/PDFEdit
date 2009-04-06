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

#ifndef _TEXTOUTPUTENTITIES_H_
#define _TEXTOUTPUTENTITIES_H_

// basic types
#include "kernel/static.h"

// pdf operators
#include "kernel/pdfoperators.h"


//=====================================================================================
namespace textoutput {
//=====================================================================================


//=====================================================================================
// The simplest entity on a page
//=====================================================================================

/**
 * This is very ugly code, BUT we are forced to this by xpdf.
 * This can be changed frequently.
 */
struct PageSimpleFragment
{
	typedef pdfobjects::PdfOperator::BBox BBox;

	typedef boost::shared_ptr<pdfobjects::PdfOperator> PdfOperatorPtr;
	typedef std::vector<PdfOperatorPtr> PdfOperators;
	typedef boost::shared_ptr<GfxState> GfxStatePtr;
	typedef boost::shared_ptr<GfxResources> GfxResourcePtr;
	typedef boost::shared_ptr<GfxFont>  GfxFontPtr;
	typedef std::string Text;

	// BBoxes
	BBox 	 		_bbox;
	// Operators
	PdfOperators 	_ops;
	// Object general properties
	GfxStatePtr  	_state;
	GfxResourcePtr 	_res;
	Text		 	_font_tag;
	Text		 	_text;

	//
	// Interface
	//
public:
	/** Specify style. */
	void add (GfxStatePtr state);
	void add (GfxResourcePtr res);
	void add (PdfOperatorPtr op);
	void add (BBox bbox);
	void add (PdfOperatorPtr op, Text text);
	void add (const Text& font_tag);

};


//=====================================================================================
// Page fragment
//=====================================================================================

//
// Page fragment base class
//

/**
 * Page text fragment.
 * Can be either word, math formula etc.
 *
 * It is supposed that the actual structure of fragment characteristics can be changed 
 * frequently, also specialized text output engines can have their special
 * characteristics an because of this PageFragment only points to the structure.
 */

struct PageFragment
{
typedef libs::Rectangle							  BBox;

typedef PageSimpleFragment::PdfOperatorPtr	  PdfOperatorPtr;
typedef PageSimpleFragment::GfxStatePtr 	  GfxStatePtr;
typedef PageSimpleFragment::GfxResourcePtr 	  GfxResourcePtr;
typedef	boost::shared_ptr<PageSimpleFragment> PageSimpleFragmentPtr;
typedef	std::vector<PageSimpleFragmentPtr> 	  PageSimpleFragments;
typedef	PageSimpleFragments::const_iterator	  Iterator;

protected:
	BBox _bbox;	/**< Aproximate bbox. */
	PageSimpleFragments _sfrags;/**< Fragment styles. */

	//
	// Interface
	//
public:
	/** Add sfrag to the end and adjust bbox accordingly. */
	virtual void push_back (PageSimpleFragmentPtr sfrag);
	/** Sort lines. */	
	virtual void sort ();


	//
	// Helper functions
	//
public:
	
	/** Return first simple fragment iterator. */
	Iterator begin () const
		{ return _sfrags.begin (); }
	/** Return last simple fragment iterator. */
	Iterator end () const
		{ return _sfrags.end (); }
	/** Is container empty. */
	bool empty() const
		{ return _sfrags.empty(); }

	/** Return apporximate bbox. */
	BBox bbox () const
		{ return _bbox; }

	//
	// DEBUG
	//
	std::string text () const
	{
		std::string t ("(");
		for (Iterator it = _sfrags.begin(); it != _sfrags.end(); ++it)
			t += (*it)->_text;
		if (1 < _sfrags.size())
		{
			std::ostringstream oss;
			oss << "*" << _sfrags.size() << "*";
			t += oss.str();
		}
		t += ")";
		return t;
	}


	//
	// Dtor
	//
public:
	virtual ~PageFragment () {}

};


//=====================================================================================
// Page word
//=====================================================================================

/**
 * Page fragment representing one word.
 *
 * Basic style used.
 */

class PageWord : public PageFragment
{
	// Interface
public:
};


//=====================================================================================
// Page formula
//=====================================================================================

/**
 * Page fragment representing one math formula.
 */

class PageFormula : public PageFragment
{
	// Interface
public:
};



//=====================================================================================
// Page line
//=====================================================================================

/** 
 * Class representing one sentence on a page. 
 */
struct PageLine
{
typedef libs::Rectangle BBox;
typedef boost::shared_ptr<PageFragment> PageFragmentPtr;
typedef std::vector<PageFragmentPtr>	PageFragments;
typedef	PageFragments::const_iterator	Iterator;

protected:
	BBox			_bbox;	/**< Approximate bbox. */
	PageFragments 	_words;	/**< Container of all words from one line. */

	//
	// Interface
	//
public:

	/** Insert word into this line. Insert it in sorted words. */
	virtual void push_back (PageFragmentPtr f);
	/** Sort lines. */	
	virtual void sort ();


	//
	// Helper functions
	//
public:
	
	/** Return first simple fragment iterator. */
	Iterator begin () const
		{ return _words.begin (); }
	/** Return last simple fragment iterator. */
	Iterator end () const
		{ return _words.end (); }
	/** Is container empty. */
	bool empty() const
		{ return _words.empty(); }

	/** Return apporximate bbox. */
	BBox bbox () const
		{ return _bbox; }

	//
	// Dtor
	//
public:
	virtual ~PageLine () {}


	//
	// DEBUG
	//
	std::string text () const
	{
		std::string t ("[");
		for (Iterator it = _words.begin(); it != _words.end(); ++it)
			t = t + (*it)->text() + " ";
		//std::ostringstream oss;
		//oss << "(" << _words.size() << ")";
		//t += oss.str();
		t += "]";
		return t;
	}

};


//=====================================================================================
// Page column
//=====================================================================================

/** 
 * Class representing one column on a page. 
 */
struct PageColumn
{
typedef libs::Rectangle 						BBox;
typedef boost::shared_ptr<PageLine>		PageLinePtr;
typedef std::vector<PageLinePtr>		PageLines;
typedef	PageLines::const_iterator		Iterator;

protected:
	BBox		_bbox;	/**< Approximate bbox. */
	PageLines	_lines;	/**< Container of all words from one line. */

	//
	// Interface
	//
public:

	/** Insert line into this column. */
	virtual void push_back (PageLinePtr l);
	/** Sort lines. */	
	virtual void sort ();

	//
	// Helper functions
	//
public:
	
	/** Return first simple fragment iterator. */
	Iterator begin () const
		{ return _lines.begin (); }
	/** Return last simple fragment iterator. */
	Iterator end () const
		{ return _lines.end (); }
	/** Is container empty. */
	bool empty() const
		{ return _lines.empty(); }

	/** Return apporximate bbox. */
	BBox bbox () const
		{ return _bbox; }

	//
	// Dtor
	//
public:
	virtual ~PageColumn () {}



	//
	// DEBUG
	//
	std::string text () const
	{
		std::string t;
		for (Iterator it = _lines.begin(); it != _lines.end(); ++it)
		{
			t += "\nNEW LINE: ";
			t += (*it)->text();
		}
		return t;
	}

};


//=====================================================================================
} // namespace textouput
//=====================================================================================

#endif // _TEXTOUTPUTENTITIES_H_
