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
//
#include "kernel/cinlineimage.h"

// =====================================================================================
namespace pdfobjects {
// =====================================================================================

using namespace std;
using namespace boost;
using namespace utils;

// =====================================================================================
namespace {
// =====================================================================================

	//
	// Begin tag and end tag are added by composite pdfoperator
	//
	/** Prefix of string representation of this object. */
	const string CINLINEIMAGE_BEGIN = "";
	/** Middle string of string representation of this object. */
	const string CINLINEIMAGE_MIDDLE = "ID";
	/** Suffix of string representation of this object. */
	const string CINLINEIMAGE_END = "";

	/**
	 * Decode or ASCII85Decode as one of its filters, the ID operator should be followed
	 * by a single white-space character; the next character after that is interpreted as
	 * the first byte of image data.
	 *
	 * The problem is, that xpdf does not read this character so we do not get it (or at least when
	 * the character is a null character).
	 * 
	 * Some pdf uses null character so we use it too.
	 *
	 * REMARK: We have to use char, because
	 * 
	 * \code
	 * string nul = "\0";
	 * char cnul = '\0';
	 *
	 * str = "123";
	 * str += nul; // "123" size: 3 
	 * str += cnul // "123\0" size: 4
	 * \endcode
	 */	
	const char CINLINEIMAGE_MIDDLE_CHAR_AFTER_ID = '\0';
	
// =====================================================================================
} // namespace
// =====================================================================================

//
// Constructors
//

//
//
//
CInlineImage::CInlineImage (::Object& oDict, const CStream::Buffer& buf) : CStream (false)
{
	kernelPrintDbg (debug::DBG_DBG, "");
	assert (objDict == oDict.getType());

	// Init dictionary, we do not have access to dictionary container
	utils::dictFromXpdfObj (this->dictionary, oDict);
	assert (0 < this->dictionary.getPropertyCount ());

	initialize( buf );
}

//
//
//
CInlineImage::CInlineImage (const CDict& dict, const CStream::Buffer& buf) : CStream (dict)
{
	kernelPrintDbg (debug::DBG_DBG, "");

	initialize( buf );
}

//
//
//
CInlineImage::CInlineImage (boost::weak_ptr<CPdf> p, ::Object& oDict, const CStream::Buffer& buf, const IndiRef& rf) : CStream ()
{
	kernelPrintDbg (debug::DBG_DBG, "");
		assert (objDict == oDict.getType());

	// Init dictionary, we do not have access to dictionary container
	utils::dictFromXpdfObj (this->dictionary, oDict);
	assert (0 < this->dictionary.getPropertyCount ());
	
	// Set pdf and ref
	setPdf (p);
	setIndiRef (rf);
	
	initialize( buf );
}

void CInlineImage::initialize( const CStream::Buffer& buf ) 
{
	_width = _height = 0;
	try {
	  _width = utils::getDoubleFromDict( dictionary, "Width" );
	}catch(CObjectException&){}
	try {
	  _width = utils::getDoubleFromDict( dictionary, "W" );
	}catch(CObjectException&){}
	try {
	  _height = utils::getDoubleFromDict( dictionary, "Height" );
	}catch(CObjectException&){}
	try {
	  _height = utils::getDoubleFromDict( dictionary, "H" );
	}catch(CObjectException&){}
	
	// Set buffer, do not use setRawBuffer because CStream would be ... copied
	std::copy (buf.begin(), buf.end(), std::back_inserter (this->buffer));
}

//
// Get methods
//

//
//
//
void
CInlineImage::getStringRepresentation (std::string& str) const
{
	str += CINLINEIMAGE_BEGIN;
	typedef std::vector<std::string> Names;
	Names names;
	CStream::dictionary.getAllPropertyNames (names);
	for (Names::const_iterator it = names.begin(); it != names.end(); ++it)
	{
		std::string tmp;
		CStream::dictionary.getProperty (*it)->getStringRepresentation (tmp);
		str += string ("/") + *it + string (" ") + tmp + string ("\n");
	}
	
	str += CINLINEIMAGE_MIDDLE;
	str += CINLINEIMAGE_MIDDLE_CHAR_AFTER_ID;
	
	for (Buffer::const_iterator it = buffer.begin(); it != buffer.end(); ++it)
		str +=  static_cast<std::string::value_type> (*it);
	str += CINLINEIMAGE_END;
}

// =====================================================================================
} // namespace pdfobjects
// =====================================================================================
