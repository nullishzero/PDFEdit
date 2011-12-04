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

#ifndef _CINLINEIMAGE_H
#define _CINLINEIMAGE_H


// all basic includes
#include "kernel/static.h"
#include "kernel/cstream.h"


//=====================================================================================
namespace pdfobjects {
//=====================================================================================



/**
 * According to pdf specification inline image is a special type of XObject that
 * can be present only in a content stream.
 *
 * It is a direct object and it can not be referenced from outside the
 * content stream. 
 *
 * The string representation of inline image is different that of a normal
 * stream. The advantage of inheriting from CStream is the transparent handling
 * of different string representation (to CStream) simply by overloading getStringRepresentation() method.
 *
 * \see CStream
 */
class CInlineImage : public CStream
{
	//
	// params
	//
private:
	double _width;
	double _height;
	
	//
	// Constructors
	//
public:
	/**
	 * Constructor. It will not be associated with a pdf.
	 *
	 * @param objDict Inline image dictionary.
	 * @param buffer Raw stream data.
	 */
	CInlineImage (::Object& objDict, const CStream::Buffer& buffer);


	/*
	 * Constructor. It will not be associated with a pdf.
	 *
	 * @param dict dictionary which will be associated with this image
	 * @param buffer Raw stream data which will be copied.
	 */
	CInlineImage (const CDict& dict, const CStream::Buffer& buf);

	/**
	 * Constructor. It will be associated with specified pdf.
	 * 
	 * @param p Pdf where it belongs.
	 * @param objDict Inline image dictionary.
	 * @param buffer Raw stream data.
	 * @param rf Indirect reference numbers.
	 */
	CInlineImage (boost::weak_ptr<CPdf> p, ::Object& objDict, const CStream::Buffer& buffer, const IndiRef& rf);

	/** Constructor. It will not be associated with a pdf. */
	CInlineImage () : CStream(false) {}

	
	//
	// Cloning
	//
protected:
	/** 
	 * Factory method.
	 * @return New instance of inline image.
	 */
	virtual CStream* _newInstance () const
		{
			CInlineImage * i = new CInlineImage;
			i->_height = _height;
			i->_width = _width;
			return i;
	}

	
	//
	//
	//
public:
	void initialize( const CStream::Buffer& buf );
	
	double width() const {
	  return _width;
	}
	double height() const {
	  return _height;
	}
	
	//
	// Get methods
	//
public:
	
	/**
	 * Returns string representation of this object.
	 *
	 * REMARK: String can contain also NOT printable characters like null
	 * character.
	 *
	 * @param str Output string.
	 */
	virtual void getStringRepresentation (std::string& str) const;

	/**
	 * This function is justfor catching programming errors, it does not make 
	 * any sense to make an xpdf object from a direct object.
	 *
     * @return Xpdf object(s).
     */
	virtual ::Object* _makeXpdfObject () const
	{ 
		assert (!"Failure. This operation is not permitted.");
		throw CObjInvalidOperation ();
	}

};



//=====================================================================================
} // namespace pdfobjects
//=====================================================================================


#endif // _CINLINEIMAGE_H
