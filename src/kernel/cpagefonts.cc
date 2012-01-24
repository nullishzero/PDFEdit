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

// static
#include "kernel/static.h"

#include "kernel/cpagefonts.h"

#include "kernel/cobject.h"
#include "kernel/cobject.h"
#include "kernel/cpage.h"
#include "kernel/cpageattributes.h"
#include "kernel/ccontentstream.h"
#include "kernel/cpagedisplay.h"


// =====================================================================================
namespace pdfobjects {
// =====================================================================================

using namespace boost;
using namespace utils;

// Static initialization
const char* CPageFonts::PDFEDIT_FONTID = "PDFEDIT_F";


	/** 
	 * Looks for a font with the given name.
	 * @param container Container of fonts (filled with getFontIdsAndNames).
	 * @param name Name of the font.
	 * @return iterator to the container (container.end() if not found).
	 */
	CPageFonts::FontList::const_iterator 
	findFont (const CPageFonts::FontList& containter, const std::string& name)
	{
		for (CPageFonts::FontList::const_iterator i=containter.begin(); i!=containter.end(); ++i)
			if(i->first == name)
				return i;
		return containter.end();
	}


//
//
//
std::string
CPageFonts::addSystemType1Font (const std::string& fontname, bool winansienc)
{
	// Create font dictionary
	// << 
	//    /Type /Font
	//    /Subtype /Type1
	//    /BaseFont / ...
	// >>
	boost::shared_ptr<CDict> font (new CDict ());
	boost::shared_ptr<CName> name (new CName (Specification::Font::TYPE));
	font->addProperty (Specification::Dict::TYPE, *name);
	name->setValue (Specification::Font::TYPE1);
	font->addProperty (Specification::Font::SUBTYPE, *name);
	name->setValue (fontname);
	font->addProperty (Specification::Font::BASEFONT, *name);
	
	// For accents
	if (winansienc)
	{
		name->setValue (Specification::Font::WINANSIENCODING);
		font->addProperty (Specification::Font::ENCODING, *name);
	}

	// Resources is an inheritable property, must be present
	if (!_page->getDictionary()->containsProperty (Specification::Page::RESOURCES))
	{
		CPageAttributes::InheritedAttributes atr;
		CPageAttributes::fillInherited (_page->getDictionary(),atr);
		_page->getDictionary()->addProperty (Specification::Page::RESOURCES, *(atr._resources));
	}
	
	// Get Resources
	boost::shared_ptr<CDict> res = _page->getDictionary()->getProperty<CDict>(Specification::Page::RESOURCES);
	
	if (!res->containsProperty (Specification::Font::TYPE))
	{	
		boost::shared_ptr<CDict> fontdict (new CDict ());
		res->addProperty (Specification::Font::TYPE, *fontdict);
	}
	
	// Get "Fonts"
	boost::shared_ptr<CDict> fonts = res->getProperty<CDict>(Specification::Font::TYPE);

	// Get all avaliable fonts
	CPageFonts::FontList fs;
	getFontIdsAndNames (fs);

	// Try PDFEDIT_FONTID{1,2,3,..}, etc., until we find one that's not in 
	// use
	std::ostringstream newfontname;
	int i = 1;
	do {
		newfontname.str("");
		newfontname << PDFEDIT_FONTID << i++;
	} while (findFont(fs, newfontname.str()) != fs.end());

	// Add it
	fonts->addProperty (newfontname.str(), *font);
	
	//
	// Create state and resources and update our contentstreams
	//
	boost::shared_ptr<GfxResources> gfxres;
	boost::shared_ptr<GfxState> gfxstate;
	_page->display()->createXpdfDisplayParams (gfxres, gfxstate);

	typedef std::vector<boost::shared_ptr<CContentStream> > CCs;
	CCs ccs;
	_page->getContentStreams (ccs);
	for (CCs::iterator it = ccs.begin(); it != ccs.end(); ++it)
		(*it)->setGfxParams (gfxstate, gfxres);

	return newfontname.str();
}

//
void 
CPageFonts::getFontIdsAndNames (FontList& cont) const
{
	// Clear container
	cont.clear ();
	
	CPageAttributes::InheritedAttributes atr;
	CPageAttributes::fillInherited (_page->getDictionary(), atr);
	boost::shared_ptr<CDict> res = atr._resources;
	
	try 
	{
		boost::shared_ptr<CDict> fonts = res->getProperty<CDict>(Specification::Font::TYPE);
		typedef std::vector<std::string> FontNames;
		FontNames fontnames;
		// Get all font names (e.g. R14, R15, F19...)
		fonts->getAllPropertyNames (fontnames);
		// Get all base names (Symbol, csr12, ...)
		for (FontNames::iterator it = fontnames.begin(); it != fontnames.end(); ++it)
		{
			boost::shared_ptr<CDict> font = fonts->getProperty<CDict>(*it);
			try {
				std::string fontbasename;
				
				if (font->containsProperty (Specification::Font::BASEFONT)) // Type{1,2} font
					fontbasename = utils::getNameFromDict (font, Specification::Font::BASEFONT);
				else									// TrueType font
					fontbasename = utils::getNameFromDict (font, Specification::Font::SUBTYPE);
				cont.push_back (std::make_pair (*it, fontbasename));

			}catch (ElementNotFoundException&)
			{}
		}

	}catch (ElementNotFoundException&)
	{
		kernelPrintDbg (debug::DBG_INFO, "No resource dictionary.");
	}
}

// =====================================================================================
} // namespace pdfobjects
// =====================================================================================
