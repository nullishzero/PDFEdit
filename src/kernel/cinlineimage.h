/*                                                                              
 * PDFedit - free program for PDF document manipulation.                        
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko, 
 *                                              Miroslav Jahoda,       
 *                                              Jozef Misutka, 
 *                                              Martin Petricek                                             
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit                                                                      
 */ 
// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80
/*
 * =====================================================================================
 *        Filename:  cinlineimage.h
 *     Description:  
 *         Created:  01/05/2006 11:46:14 AM CET
 *          Author: jmisutka
 * =====================================================================================
 */

#ifndef _CINLINEIMAGE_H
#define _CINLINEIMAGE_H


// all basic includes
#include "static.h"

// 
#include "cobject.h"


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

	/**
	 * Constructor. It will be associated with specified pdf.
	 * 
	 * @param p Pdf where it belongs.
	 * @param objDict Inline image dictionary.
	 * @param buffer Raw stream data.
	 * @param rf Indirect reference numbers.
	 */
	CInlineImage (CPdf& p, ::Object& objDict, const CStream::Buffer& buffer, const IndiRef& rf);

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
		{ return new CInlineImage; }

	
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
