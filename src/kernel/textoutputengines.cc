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
#include "kernel/iproperty.h"
#include "kernel/carray.h"
#include "kernel/cobjectsimple.h"
#include "kernel/textoutputengines.h"
#include "kernel/stateupdater.h"

//=====================================================================================
namespace textoutput {
//=====================================================================================

using namespace std;
using namespace boost;
using namespace debug;
using namespace pdfobjects;
using namespace pdfobjects::utils;

//=====================================================================================
namespace {
//=====================================================================================

	//
	// Abbreviatons
	//
	typedef SimpleWordEngine::PdfOperatorPtr 		PdfOperatorPtr;
	typedef SimpleWordEngine::PageSimpleFragmentPtr PageSimpleFragmentPtr;
	typedef SimpleWordEngine::PageWordPtr 			PageWordPtr;
	typedef PageSimpleFragment::BBox				BBox;
	typedef SimpleWordEngine::GfxStatePtr			GfxStatePtr;

	using std::min;
	using std::max;

	//
	// Bordering constants
	// 	-- try fail constants
	//
	
	// If font size is too small (e.g. 1) use this instead of BORDER_X_DIV
	const static double BORDER_X_DISTANCE = 3;
	// Acceptable character spacing within one word when multiplied by font size
	const static double BORDER_X_DIV = 0.15;
	const static double BORDER_Y_DIV = 0.20;
	// Acceptable diff between word font and new fragment font to form one word
	const static double FONT_SIZE_DIFF = 3;
	// Max diff betweem baselines
	const static double LINE_Y_DIV = 0.2;

	// Name delimeters
	const static string FONT8BIT_CHARNAME_HEADER = "<specialchar>";
	const static string FONT8BIT_CHARNAME_FOOTER = "</specialchar>";
	const static string UNKNOWN_CHAR = "<unknownchar />";


	//
	// Is text showing operator
	//
	bool 
	text_op (const PdfOperatorPtr& op)
		{ return isPdfOp (op, string ("Tj"), string ("TJ"),string ("'"), string("\"")); }

	//
	// Get text from text operator
	//
	string 
	text_op_text (const PdfOperator& op, const GfxState& state)
	{
		assert (!isPdfOp(op, string("TJ")));

		// Operator text -- needn't be real ascii chars
		string text;
		assert (1 == op.getParametersCount());
		PdfOperator::Operands ops;
		op.getParameters (ops);
		assert (1 == ops.size());
		text = getStringFromIProperty (ops.front());
	
		//
		// Get real text from operators using xpdf (see the crazy code below)
		//
		const GfxFont* font = (const_cast<GfxState&>(state)).getFont();
			if (!font)
				return text;
		char* p = const_cast<char*> (text.c_str ());
		size_t len = text.size();
		int n = 0, uLen = 0;
		CharCode code;
		Unicode u;
		double d = 0.0;

		string result;
		while (len > 0) 
		{
			n = font->getNextChar (	p, 
									static_cast<int>(len), 
									&code,
									&u, 
									sizeof(Unicode), 
									&uLen,
									&d, &d, &d, &d);

			p += n;
			len -= n;
		
			// Put rather name tham the character
			const Gfx8BitFont* f = dynamic_cast<const Gfx8BitFont*> (font);
			if (f)
			{
				string tmp;
				const char* c = f->getCharName(code);
				if (c)
					tmp = c;
				if (1 < tmp.size())
					result += (FONT8BIT_CHARNAME_HEADER + tmp + FONT8BIT_CHARNAME_FOOTER);
				else if (0 == tmp.size())
					result +=  UNKNOWN_CHAR;
				else
					result += tmp;
			
			}else
				result += static_cast<char> (u&0xFF);
		}

		return result;
	}

	//
	// Stupid double abs
	//
	double
	abs (const double& a)
		{ return fabs (a); }

	//
	// Word part position comparator
	//
	bool
	word_part (const PageWord& w, const PageSimpleFragment& f)
	{
		// If word is empty
		assert (!w.empty());

		BBox b1 = w.bbox();
		BBox b2 = f._bbox;
		double wfsize = (*w.begin())->_state->getFontSize();
		double fsize = f._state->getFontSize();
		
		// todo
		kernelPrintDbg (DBG_DBG, 
			"\nbbox1: [" << b1 << "]" <<
			"\nbbox2: [" << b2 << "]" <<
			"\nWord font size: " << wfsize  << " font size: " << fsize << 
			"\nBORDER_Y_DISTANCE * wfsize = " << BORDER_Y_DIV * wfsize << "\nBORDER_X_DISTANCE * wfsize = " << BORDER_X_DIV * wfsize <<
			"\nText: "  << w.text() << "  ***  " << f._text);

		//
		// If fonts are too different it is a different word
		//
		if (abs (wfsize - fsize) > FONT_SIZE_DIFF)
		{
			//kernelPrintDbg (DBG_DBG, "NOT ONE WORD -- fonts too different.");
			return false;
		}

		//
		// This is almost magic 
		//  -- if rectangle borders are close then it's one word
		//

		// Y axis
		if (abs (min(b1.yleft,b1.yright) - min (b2.yleft, b2.yright)) > BORDER_Y_DIV * wfsize
				||	abs (max(b1.yleft,b1.yright) - max (b2.yleft, b2.yright)) > BORDER_Y_DIV * wfsize)
		{
			//kernelPrintDbg (DBG_DBG, "NOT ONE WORD -- y axis bad.");
			return false;
		}

		// X axis - but only the right part of w and left part of f
		if (abs (b1.xright - b2.xleft) > max (BORDER_X_DIV*wfsize, BORDER_X_DISTANCE))
		{
			//kernelPrintDbg (DBG_DBG, "NOT ONE WORD -- x axis bad.");
			return false;
		}


		// ONE word
		return true;
	}


	//
	// Transfrom complex pdf operators to simple ones
	//
	template<typename T>
	T 
	transform_op (PdfOperatorPtr op, const GfxState& state)
	{
		T t;
		GfxStatePtr s (const_cast<GfxState&> (state).copy(false));

		if (isPdfOp (op, "TJ"))
		{
			kernelPrintDbg (DBG_DBG, " BIG RECTANGLE: " << op->getBBox());
			assert (s->getFont());
			double fsize = s->getFontSize();
			
			PdfOperator::Operands ops;
			op->getParameters (ops);
			assert (1 == ops.size());
			boost::shared_ptr<CArray> array = IProperty::getSmartCObjectPtr<CArray> (ops.front());
			//
			// Loop through TJ operands either strings or nums
			//
			for (size_t i = 0; i < array->getPropertyCount(); ++i)
			{
				boost::shared_ptr<IProperty> ip = array->getProperty (i);
				if (isNumber (ip))
				{
					int wMode = s->getFont()->getWMode();
					double dx = getDoubleFromIProperty (ip) * 0.001 * abs(fsize);
	  				if (wMode)
						s->textShift(0, -dx);
					else
						s->textShift(-dx, 0);

				}else if (isString(ip))
				{
					// Create Tj with text and add it to output
					string txt = getStringFromIProperty (ip);
					PdfOperator::Operands opers;
					opers.push_back (boost::shared_ptr<CString> (new CString (txt)));
					boost::shared_ptr<PdfOperator> newop = createOperator ("Tj", opers);
					// Set bbox 
					BBox bbox;
					StateUpdater::printTextUpdate (s.get(), txt, &bbox);
					kernelPrintDbg (DBG_DBG, "\tSMALL RECTANGLE: [" << bbox << "] text:" << txt);
					newop->setBBox (bbox);
					
					t.push_back (make_pair (newop, s));
					s = GfxStatePtr (s->copy(false));
				}else
					throw MalformedContentStreamException ();
			} // for

		}else
		{
			t.push_back (make_pair (op, s));
		}

		return t;
	}

//=====================================================================================
} // namespace
//=====================================================================================


//
// Word engine
//
void
SimpleWordEngine::operator() (const PdfOperatorPtr op, const GfxState& gfx_state)
{
	static PageSimpleFragmentPtr sfrag (PageSimpleFragmentPtr(new PageSimpleFragment));
	assert (op);

	//
	// Transform difficult / complex operators to simple ones
	//
	typedef vector<pair<PdfOperatorPtr, GfxStatePtr> > TransformedOps;
	TransformedOps t_ops = transform_op<TransformedOps> (op, gfx_state);

	for (TransformedOps::iterator it = t_ops.begin(); it != t_ops.end(); ++it)
	{
		PdfOperatorPtr o = (*it).first;
		GfxStatePtr s = (*it).second;

		//
		// Text showing operator ends a page fragment
		//
		if (text_op (o))
		{
			//
			// Add all info needed -- bbox, text, state, font
			//
			sfrag->add (o->getBBox ());
			sfrag->add (o, text_op_text(*o,*s));
			// Copy and then set state
			sfrag->add (s);
			sfrag->add (res);
			// Set fragment font
			const GfxFont* font = s->getFont();
			if (font) 
			{
				sfrag->add (string (font->getTag()->getCString()));
				// Set invalid font (because of crazy xpdf) BUT correct font size in state object
				s->setFont (NULL, s->getFontSize());
			}
			
			// Store simple fragment and create a new one
			sfrags.push_back (sfrag);
			sfrag = PageSimpleFragmentPtr (new PageSimpleFragment);

		//
		// Other operators are just added to the fragment
		//
		}else
		{
			sfrag->add (o);
		}

	} // for
}

//
// Formatting simple fragments to fragments (words, etc.)
//
void
SimpleWordEngine::operator() ()
{
	// 
	// todo what if not after each
	//
	
	PageWordPtr w = PageWordPtr (new PageWord);
	// Simple collecting frags to words
	for (PageSimpleFragments::iterator it = sfrags.begin(); it != sfrags.end(); ++it)
	{
		//
		// If word is empty (first word) or fragment is pard of existing word
		// insert it into the word
		//
		if (w->empty() || word_part (*w, *(*it)))
		{
			w->push_back (*it);
			w->sort ();

		//
		// Otherwise create new word
		//
		}else
		{
			frags.push_back (w);
			w = PageWordPtr (new PageWord);
			w->push_back (*it);
		}
	}

	// Add the last word
	frags.push_back (w);
}


//
// Line engine
//

//
// Does word belongs to a line
//
SimpleLineEngine::LinePart::result 
SimpleLineEngine::LinePart::line_part (const PageLine& l, const PageFragment& f)
{
	BBox b1 = l.bbox();
	BBox b2 = f.bbox();
	// word height
	//double lsize = abs (b1.yleft - b1.yright);

	//
	// Check whether f is "below" current line -- next line
	//
	if (b1.yleft > b1.yright)
	{
		if (b1.yright > max (b2.yleft, b2.yright))
			return was_part;
	}else 
		if (b1.yleft < min (b2.yleft, b2.yright))
			return was_part;


	// Y axis -- if the intersection of y axis is not empty it is one line
	b2.xleft = b1.xleft;
	b2.xright = b1.xright;
	if (!BBox::isInitialized (libs::rectangle_intersect (b1,b2)))
		return not_part;
	
	return is_part;
}


//=====================================================================================
} // namespace textoutput
//=====================================================================================
