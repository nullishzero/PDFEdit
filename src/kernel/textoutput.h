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

#ifndef _TEXTOUTPUT_H_
#define _TEXTOUTPUT_H_

// basic types
#include "kernel/static.h"

// output builder
#include "kernel/textoutputbuilder.h"

// text output entities
#include "kernel/textoutputentities.h"


//=====================================================================================
namespace textoutput {
//=====================================================================================

//=====================================================================================
// Page source
//=====================================================================================

//
// Page source
//

/**
 * Text representation of a page.
 *
 * This class can be parametrized by page entity engines. Word engine is
 * responsible for creating complex structures (words,...) from simple fragments.
 *
 *
 * Word engine must contain three functions
 *  * operator() (shared_ptr<GfxResources>) -- to init resources (it is called just once because resources do not change)
 *  * operator() (PdfOperatorPtr, BBox, const GfxState&) -- called after
 *  each end every operator
 *  * operator() () -- responsible for ordering and reformating words (optimizing)
 *
 */

template<typename WordEngine, 
		 typename LineEngine, 
		 typename ColumnEngine 
		 >
class PageTextSource
{
typedef SimpleWordEngine::PdfOperatorPtr PdfOperatorPtr;
typedef libs::Rectangle BBox;

private:
	WordEngine 	 wordengine;
	LineEngine 	 lineengine;
	ColumnEngine columnengine;

	//
	// Basic functions
	//
public:
	/** Init stateupdater functor. */
	void operator() (boost::shared_ptr<GfxResources> gfx_res)
		{ wordengine (gfx_res); }

	/** Stateupdater functor. */
	void operator() (const PdfOperatorPtr op, 
					 BBox rc,
					 const GfxState& gfx_state) 
	{
		assert (rc == op->getBBox());

		// Add the operator to one of existing fragments or add new one
		wordengine (op, gfx_state);
	}

	/** Create lines, columns. */
	void format ()
	{
		// Order fragments to words
		wordengine ();
		// Create sentences from words
		lineengine (wordengine);
		// Create columns from sentences
		columnengine (lineengine);
	}

	/** Output builder. */
	void output (OutputBuilder& out, size_t pagepos) const
	{
		// start page
		out.start_page (pagepos);
		// Try building output from columns (tree like structure)
		out.build (columnengine.begin(), columnengine.end());
		// Try building output from words (list structure)
		out.build (wordengine.begin(), wordengine.end());
		// end page
		out.end_page ();
	}

};

//=====================================================================================
} // namespace textouput
//=====================================================================================

#endif // _TEXTOUTPUT_H_
