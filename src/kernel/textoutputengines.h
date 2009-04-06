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

#ifndef _TEXTOUTPUTENGINES_H_
#define _TEXTOUTPUTENGINES_H_

// basic types
#include "kernel/static.h"
#include <vector>

// output builder
#include "kernel/textoutputentities.h"


//=====================================================================================
namespace textoutput {
//=====================================================================================


//=====================================================================================
// Simple word engine
//=====================================================================================

/**
 * This is a simple word engine.
 *
 * Can be extended very easily or better replaced.
 */
struct SimpleWordEngine
{
typedef PageFragment::GfxStatePtr 		GfxStatePtr;
typedef PageFragment::GfxResourcePtr	GfxResourcePtr;

typedef PageFragment::PdfOperatorPtr 		PdfOperatorPtr;
typedef PageFragment::PageSimpleFragments 	PageSimpleFragments;
typedef PageFragment::PageSimpleFragmentPtr PageSimpleFragmentPtr;

typedef boost::shared_ptr<PageFragment> PageFragmentPtr;
typedef std::vector<PageFragmentPtr>  	PageFragments;
typedef PageFragments::const_iterator 	Iterator;

typedef boost::shared_ptr<PageWord>	PageWordPtr;


protected:
	PageSimpleFragments sfrags;	/**< List of all simple fragments on a page. */
	PageFragments frags;		/**< List of all fragments on a page. */
	GfxResourcePtr res;			/**< Resources containing fonts, etc. */

	//
	// Page source functor
	//
public:
	/** Init fragments. */
	void operator() (GfxResourcePtr gfx_res)
		{ res = gfx_res; }
	
	/** Create fragments. */
	void operator() (PdfOperatorPtr op, const GfxState& gfx_state);

	/** Create words. */
	void operator() ();

	//
	// Page fragment container
	//
public:
	/** Return the beginning of word list. */
	Iterator begin () const
		{ return frags.begin(); }

	/** Return the end of word list. */
	Iterator end () const
		{ return frags.end(); }

};


//=====================================================================================
// Simple line engine
//=====================================================================================

/**
 * Simple line engine grouping words in one line.
 */

struct SimpleLineEngine
{
typedef boost::shared_ptr<PageLine>	PageLinePtr;
typedef std::vector<PageLinePtr>	PageLines;
typedef PageLines::const_iterator 	Iterator;
typedef SimpleWordEngine::PageFragmentPtr PageFragmentPtr;

private:
	PageLines _lines;	/**< Container of lines. */

	//
	// Class deciding whether a word belongs to a line
	//
	struct LinePart
	{
		typedef short unsigned int result;
		static const result is_part  = 0;
		static const result was_part = 1;
		static const result not_part = 2;
		/** Decide whether word is part of this line. */
		static result line_part (const PageLine& l, const PageFragment& f);
	};

	//
	// Page source functor
	//
public:
	/** Add this operator to words. */
	template<typename WordEngine>
	void operator() (const WordEngine& w)
	{
		//
		// Loop through all words and group them into lines
		//
		for (typename WordEngine::Iterator itw = w.begin(); itw != w.end(); ++itw)
		{
			PageLines::iterator itl;
			LinePart::result 	is_part = LinePart::not_part;

			// Find appropriate place
			for (itl = _lines.begin(); itl != _lines.end(); ++itl)
			{
				is_part = LinePart::line_part (*(*itl), *(*itw));
				if (LinePart::not_part != is_part)
					break;
			}
			
			// Insert itw into exsting line
			if (LinePart::is_part == is_part)
			{
				(*itl)->push_back (*itw);
		
			// Create new line before existing one
			}else if (LinePart::was_part == is_part)
			{
				itl = _lines.insert (itl, PageLinePtr (new PageLine));
				(*itl)->push_back (*itw);

			//	Not part make new line at the end
			}else if (LinePart::not_part == is_part)
			{
				_lines.push_back (PageLinePtr (new PageLine));
				_lines.back()->push_back (*itw);
			}

		}// for (WordEngine::Iterator itl = w.begin(); itl != w.end(); ++itl)
	
		//
		// Sort words in lines
		//
		for (PageLines::iterator it = _lines.begin(); it != _lines.end(); ++it)
			(*it)->sort ();
	}

	
	//
	// Page line container
	//
public:
	/** Return the beginning of line list. */
	Iterator begin () const
		{ return _lines.begin(); }

	/** Return the end of line list. */
	Iterator end () const
		{ return _lines.end(); }

};


//=====================================================================================
// Simple column engine
//=====================================================================================

/**
 * Simple column engine grouping lines in one columns.
 */

struct SimpleColumnEngine
{
typedef boost::shared_ptr<PageColumn> PageColumnPtr;
typedef std::vector<PageColumnPtr> 	  PageColumns;
typedef PageColumns::const_iterator	  Iterator;

private:
	PageColumns _cols;

	//
	// Page source functor
	//
public:
	/** Add this operator to words. */
	template<typename LineEngine>
	void operator() (const LineEngine& l)
	{
		// todo
		_cols.push_back (PageColumnPtr (new PageColumn));
		for (typename LineEngine::Iterator it = l.begin(); it != l.end(); ++it)
			_cols.back()->push_back (*it);
		_cols.back()->sort ();
	}


	//
	// Page column container
	//
public:
	/** Return the beginning of column list. */
	Iterator begin () const
		{ return _cols.begin(); }

	/** Return the end of column list. */
	Iterator end () const
		{ return _cols.end(); }

};


//=====================================================================================
} // namespace textouput
//=====================================================================================

#endif // _TEXTOUTPUTENGINES_H_
