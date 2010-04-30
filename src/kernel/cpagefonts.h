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

#ifndef _CPAGEFONTS_H
#define _CPAGEFONTS_H

// all basic includes
#include "kernel/static.h"
#include "kernel/cpagemodule.h"


//=====================================================================================
namespace pdfobjects {
//=====================================================================================

// Forward declarations
class CPage;

//=====================================================================================
// CPageFonts
//=====================================================================================

/**
 * Class representing page fonts.
 */
class CPageFonts : public ICPageModule
{
	// Typedefs
public:
	typedef std::list<std::string> SystemFontList;
	/** Type for list of fonts. */
	typedef std::vector<std::pair<std::string, std::string> > FontList;

	// Constants
private:
	static const char* PDFEDIT_FONTID;


	// Variables
private:
	/** Pdf dictionary representing a page. */
	CPage* _page;

	// Ctor & Dtor
public:
	CPageFonts (CPage* page) : _page (page) {}
	~CPageFonts () 
		{ _page = NULL; }
		
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

	/**
	 * Get all system fonts which should be supported by all pdf viewers.
	 */
	static SystemFontList getSystemFonts () 
	{
		SystemFontList flist;
		for (size_t i = 0; i < nBuiltinFonts; ++i)
			flist.push_back (builtinFonts[i].name);
		return flist;
	};

}; // class CPage


//=====================================================================================
} // namespace pdfobjects
//=====================================================================================


#endif // _CPAGEFONTS_H
