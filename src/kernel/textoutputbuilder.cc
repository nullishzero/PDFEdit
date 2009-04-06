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
#include "kernel/textoutputbuilder.h"

//=====================================================================================
namespace textoutput {
//=====================================================================================

using namespace std;
using namespace boost;
using namespace pdfobjects;

//=====================================================================================
namespace {
//=====================================================================================

	/**
	 * Map font type to name.
	 */
	string font_type (GfxFontType tp)
	{
		// FIXME handle all GfxFontType values
		switch (tp)
		{
			case  fontType1:
				return "Type1 (8bit)";
			case  fontType1C:
				return "Type1C (8bit)";
			case  fontType3:
				return "Type3 (8bit)";
			case  fontTrueType:
				return "TrueType (8bit)";
			case  fontCIDType0:
				return "CIDTrueType (CID)";
			case  fontCIDType0C:
				return "Type0C (CID)";
			case  fontCIDType2:
				return "Type2 (CID)";
			case fontUnknownType:
			default:
				return "UnknownType (8bit)";
		}
	}


	//
	// entities
	//
	string make_att (const string& attn, const string& att)
		{ return string (attn + string("=\"") + att + string ("\" ")); }
	template<typename T>
	string make_att (const string& attn, T t)
	{ 
		ostringstream str;
		str << t;
		return string (attn + string("=\"") + str.str() + string ("\" ")); 
	}

	string make_att (const PageLine::BBox& bbox)
	{ 
		ostringstream strb;
		strb << bbox;
		return make_att("bbox",strb.str());
	}

	//
	// attributes
	//
	string make_ent (const string& ent, const string& att)
		{ return string (string ("<") + ent + string(" ") + att + string (">")); }
	string make_ent (const string& ent, const string& att, const string& att1)
		{ return string (string ("<") + ent + string(" ") + att + att1 + string (">")); }
	string make_ent (const string& ent, const string& att, const string& att1, const string& att2)
		{ return string (string ("<") + ent + string(" ") + att + att1 + att2 + string (">")); }
	string make_ent (const string& ent, const string& att, const string& att1, const string& att2, const string& att3)
		{ return string (string ("<") + ent + string(" ") + att + att1 + att2 + att3 + string (">")); }

	//
	// General
	//
	namespace XML_GENERAL
	{
		const string header = "<?xml version='1.0' encoding='utf-8'?>\n<!-- jmisutka -->\n\n"
							  "<xmlpdf xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
							  "xsi:noNamespaceSchemaLocation=\"http://jm.ignac.org/pdfedit/schema/pdftoxml.xml\">\n";
		const string footer = "\n</xmlpdf>";
	}
	//
	// Page
	//
	namespace XML_PAGE
	{
		const string header (size_t pnum)
			{ return make_ent ("page", make_att("number",pnum)); }

		const string footer = "\n</page>";
	}


	//
	// Frag
	//
	namespace XML_FRAG
	{
		//
		// header & footer
		//
		const string header (const PageLine::BBox& b)
			{ return make_ent ("frag", make_att(b)); }
		const string footer = "</frag>";

		//
		// font
		//
		const string font_header (const PageSimpleFragment& f)
		{
			ostringstream oss;
			GfxFont* font = f._res->lookupFont (f._font_tag.c_str());

			if (font)
			{
				string tmp;
				ostringstream otmp;
				tmp += make_att ("basename", font->getName() ? font->getName()->getCString() : string ("unknown"));
				tmp += make_att ("origname", font->getOrigName() ? font->getOrigName()->getCString() : string ("unknown"));
				tmp += make_att ("embeddedfontname", font->getEmbeddedFontName() ? font->getEmbeddedFontName()->getCString() : string ("unknown"));
				tmp += make_att ("tag", font->getTag() ? font->getTag()->getCString() : string ("unknown"));
				tmp += make_att ("serif", font->isSerif() ? string ("true") : string ("false"));
				tmp += make_att ("symbolic", font->isSymbolic() ? string ("true") : string ("false"));
				tmp += make_att ("italic", font->isItalic() ? string ("true") : string ("false"));
				tmp += make_att ("bold", font->isBold() ? string ("true") : string ("false"));
				otmp << font->getAscent();
				tmp += make_att ("ascent", otmp.str());
				otmp.str("");
				otmp << font->getDescent();
				tmp += make_att ("descent", otmp.str());
				otmp.str("");
				tmp += make_att ("writemode", font->getWMode() ? string ("vertical") : string ("horizontal"));
				tmp += make_att ("fonttype", font_type (font->getType()));
				// create entity
				oss << make_ent ("font", tmp);

			}else
				oss << make_ent ("font", make_att("name", "UNKNOWN FONT"));

			return oss.str();
		}
		const string font_footer = "</font>";
	}

	//
	// Word
	//
	namespace XML_WORD
	{
		//
		// header & footer
		//
		const string header (const PageLine::BBox& b)
			{ return make_ent ("word", make_att(b)); }
		const string footer = "</word>";
		// delimeter
		const string delimeter = "\t";
	}
	//
	// Line
	//
	namespace XML_LINE
	{
		//
		// header & footer
		//
		const string header (const PageLine::BBox& b)
			{ return make_ent ("line", make_att(b)); }
		const string footer = "</line>";
		
		// new line
		const string delimeter = "\t";
	}
	//
	// Column
	//
	namespace XML_COLUMN
	{
		//
		// header & footer
		//
		string header (const PageLine::BBox& b)
			{ return make_ent ("column", make_att(b)); }
		const string footer = "</column>";
		const string newline = "\n\t";
	}



	//
	//
	//
	string 
	word2xml (const PageFragment& w, const string& newline)
	{
		ostringstream res;
		string nl (newline + XML_WORD::delimeter);

		// header
		res << XML_WORD::header (w.bbox());
		// real stuff
		for (PageFragment::Iterator it = w.begin(); it != w.end(); ++it)
		{
			res << nl << XML_FRAG::font_header (**it) << nl;
			res << XML_FRAG::header ((*it)->_bbox) << (*it)->_text << XML_FRAG::footer << nl;
			res << XML_FRAG::font_footer;
		}
		//footer
		res << newline << XML_WORD::footer;

		return res.str();
	}

	//
	//
	//
	string 
	line2xml (const PageLine& l, const string& newline)
	{
		ostringstream res;
		string nl (newline + XML_LINE::delimeter);

		// header
		res << XML_LINE::header (l.bbox());
		// real stuff
		for (PageLine::Iterator it = l.begin(); it != l.end(); ++it)
			res << nl << word2xml (**it, nl);
		//footer
		res << newline << XML_LINE::footer;

		return res.str();
	}

	//
	//
	//
	string 
	column2xml (const PageColumn& c)
	{
		ostringstream res;

		// header
		res << XML_COLUMN::header (c.bbox());
		// real stuff
		for (PageColumn::Iterator it = c.begin(); it != c.end(); ++it)
			res << XML_COLUMN::newline << line2xml (**it, XML_COLUMN::newline);
		// footer
		res << endl << XML_COLUMN::footer;

		return res.str();
	}


//=====================================================================================
} // namespace
//=====================================================================================

//
// OutputBuilder
//

//
// From words
//
void
XmlOutputBuilder::build (PageFragmentIterator, PageFragmentIterator)
{
	kernelPrintDbg (debug::DBG_DBG,"");
	
	//for (PageFragmentIterator it = it_s; it != it_e; ++it)
	//	cout << (*it)->text() << "\t" << flush;
}


//
// Xml output builder
//

//
// From columns
//
void
XmlOutputBuilder::build (PageColumnIterator it_s, PageColumnIterator it_e)
{
	// header
	_str += XML_PAGE::header (_pagepos);

	// stuff
	for (PageColumnIterator it = it_s; it != it_e; ++it)
		_str += string ("\n") + column2xml (**it);

	// footer
	_str += XML_PAGE::footer + string ("\n");
}

//
//
//
string
XmlOutputBuilder::xml (const XmlOutputBuilder& out)
{
	return XML_GENERAL::header + out.str() + XML_GENERAL::footer;
}

//=====================================================================================
} // namespace textoutput
//=====================================================================================
