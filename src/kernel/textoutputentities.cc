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

#include "kernel/static.h"
#include "kernel/textoutputentities.h"

//=====================================================================================
namespace textoutput {
//=====================================================================================

using namespace std;
using namespace boost;
using namespace debug;


//=====================================================================================
namespace {
//=====================================================================================

	// Forward declarations
	typedef PageSimpleFragment::BBox BBox;

	/** Merge two bbox into one big. */
	BBox
	bbox_merge (BBox _b1, BBox _b2)
	{
		return libs::rectangle_merge (_b1, _b2);
	}

	// Forward declarations
	typedef PageFragment::PageSimpleFragmentPtr PageSimpleFragmentPtr;

	/** Similar frag. */
	bool 
	similar_frag (const PageSimpleFragment& f1, const PageSimpleFragment& f2)
	{
		return (f1._font_tag == f2._font_tag); // && ...
	}

//=====================================================================================
} // namespace
//=====================================================================================


//=====================================================================================
// PageSimpleFragment
//=====================================================================================

//
// 
//
void
PageSimpleFragment::add (PdfOperatorPtr op)
{
	// Add operator to ops
	_ops.push_back (op);
}
void
PageSimpleFragment::add (PdfOperatorPtr op, Text text)
{
	assert (_text.empty());
	_text = text;
	// Add operator to ops
	_ops.push_back (op);
}
void
PageSimpleFragment::add (GfxStatePtr state)
{
	// Add operator to ops
	_state = state;
}
void
PageSimpleFragment::add (GfxResourcePtr res)
{
	// Add operator to ops
	_res = res;
}
void
PageSimpleFragment::add (BBox bbox)
{
	//kernelPrintDbg (DBG_DBG, "rectangle: " << bbox);
	// Add operator to ops
	_bbox = bbox;
}
void
PageSimpleFragment::add (const Text& font_tag )
{
	// Add operator to ops
	_font_tag = font_tag;
}


//=====================================================================================
// PageFragment
//=====================================================================================

//
//
//
void
PageFragment::push_back (PageSimpleFragmentPtr sfrag)
{
	//
	// Handle first time 
	//
	if (_sfrags.empty())
	{
		_sfrags.push_back (sfrag);
		_bbox = sfrag->_bbox;
		return;
	}

	//
	// If font matches bbox bottom line then merge into one text
	//
	if (similar_frag (*(_sfrags.back()), *sfrag))
	{
		_sfrags.back()->_text += sfrag->_text;	

	}else
	{
		kernelPrintDbg (DBG_DBG, "FRAGS ARE NEAR BUT FONT DOES NOT MATCH!");
		// Add fragment to the end of this word
		_sfrags.push_back (sfrag);
	}

	// Merge bbox of current word with added part
	_bbox = bbox_merge (_bbox, sfrag->_bbox);
}

//
//
//
void
PageFragment::sort ()
{
	// todo
}

//=====================================================================================
// PageLine 
//=====================================================================================

//
//
//
void
PageLine::push_back (PageFragmentPtr f)
{
	_words.push_back (f);
	_bbox = bbox_merge (BBox::isInitialized(_bbox) ? _bbox : f->bbox(), f->bbox ());
}

//
//
//
void
PageLine::sort ()
{
	// todo
}

//=====================================================================================
// PageLine 
//=====================================================================================

//
//
//
void
PageColumn::push_back (PageLinePtr l)
{
	_lines.push_back (l);
	_bbox = bbox_merge (BBox::isInitialized(_bbox) ? _bbox : l->bbox(), l->bbox ());
}

//
//
//
void
PageColumn::sort ()
{
	// todo
}

//=====================================================================================
} // namespace textoutput
//=====================================================================================
