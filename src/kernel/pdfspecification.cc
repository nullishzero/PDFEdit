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

using namespace std;

// =====================================================================================
namespace pdfobjects {
// =====================================================================================

	// Null
	const std::string Specification::CNULL_NULL ("null");
	// Bool		
	const std::string Specification::CBOOL_TRUE  ("true");
	const std::string Specification::CBOOL_FALSE ("false");
	// Name
	const std::string Specification::CNAME_PREFIX ("/");
	// String
	const std::string Specification::CSTRING_PREFIX ("(");
	const std::string Specification::CSTRING_SUFFIX (")");
	const std::string Specification::CHEXSTRING_PREFIX ("<");
	const std::string Specification::CHEXSTRING_SUFFIX (">");
	// Ref
	const std::string Specification::CREF_MIDDLE (" ");
	const std::string Specification::CREF_SUFFIX (" R");

	// Array
	const std::string Specification::CARRAY_PREFIX ("[");
	const std::string Specification::CARRAY_MIDDLE (" ");
	const std::string Specification::CARRAY_SUFFIX (" ]");
	// Dict
	const std::string Specification::CDICT_PREFIX ("<<");
	const std::string Specification::CDICT_MIDDLE ("\n/");
	const std::string Specification::CDICT_BETWEEN_NAMES (" ");
	const std::string Specification::CDICT_SUFFIX ("\n>>");
	// Stream
	const std::string Specification::CSTREAM_HEADER ("\nstream\n");
	const std::string Specification::CSTREAM_FOOTER ("\nendstream");
	// Indirect
	const std::string Specification::INDIRECT_HEADER ("obj ");
	const std::string Specification::INDIRECT_FOOTER ("\nendobj");

	// Object Error
	const std::string Specification::OBJERROR ("\n");


	// 
	// Dict
	//
	const std::string Specification::Dict::TYPE ("Type");

	// 
	// Page
	//
	const std::string Specification::Page::TYPE		("Page");
	const std::string Specification::Page::PARENT   ("Parent");
	const std::string Specification::Page::CONTENTS ("Contents");
	const std::string Specification::Page::RESOURCES("Resources");
	const std::string Specification::Page::MEDIABOX ("MediaBox");
	const std::string Specification::Page::CROPBOX  ("CropBox");
	const std::string Specification::Page::TRIMBOX  ("TrimBox");
	const std::string Specification::Page::ROTATE   ("Rotate");
	const std::string Specification::Page::ANNOTS   ("Annots");

	//
	// Font
	//
	const std::string Specification::Font::TYPE				("Font");
	const std::string Specification::Font::TYPE1			("Type1");
	const std::string Specification::Font::SUBTYPE			("Subtype");
	const std::string Specification::Font::BASEFONT			("BaseFont");
	const std::string Specification::Font::ENCODING			("Encoding");
	const std::string Specification::Font::WINANSIENCODING	("WinAnsiEncoding");


// =====================================================================================
} // namespace pdfobjects
// =====================================================================================
