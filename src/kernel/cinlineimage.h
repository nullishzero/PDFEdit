// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80
/*
 * =====================================================================================
 *        Filename:  cinlineimage.h
 *     Description:  
 *         Created:  01/05/2006 11:46:14 AM CET
 *          Author: jmisutka
 * =====================================================================================
 */

#ifndef _CPAGE_H
#define _CPAGE_H


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
 * content. 
 *
 * It can inherit from CStream and therefore it has several advantages. 
 * 
 * The string representation of inline image is different that of a normal
 * stream. The advantage of inheriting from CStream is that transparently handles
 * different string representation simply by overloading getStringRepresentation method.
 * Otherwise it could be a problem.
 */
class CInlineImage : public CStream
{
	//
	// Constructors
	//
public:
	/**
	 * Constructor. 
	 *
	 * It does not belong to any pdf.
	 *
	 * @param objDict Inline image dictionary.
	 * @param buffer Raw stream data.
	 */
	CInlineImage (::Object& objDict, const CStream::Buffer& buffer);

	/**
	 * Constructor.
	 *
	 * This inline image is in a pdf.
	 * 
	 * @param p Pdf where it belongs.
	 * @param objDict Inline image dictionary.
	 * @param buffer Raw stream data.
	 * @param rf Indirect reference numbers.
	 */
	CInlineImage (CPdf& p, ::Object& objDict, const CStream::Buffer& buffer, const IndiRef& rf);

	/** Default constructor. */
	CInlineImage () {};

	
	//
	// Cloning
	//
protected:
	/** 
	 * Factory method.
	 * 
	 * @return New instance of inline image.
	 */
	virtual CStream* _newInstance () const
		{ return new CInlineImage; };

	
	//
	// Get methods
	//
public:
	
	/**
	 * Returns string representation of this object.
	 *
	 * REMARK: String can contain also NOT printable characters like '\0'.
	 *
	 * @param str Output string.
	 */
	virtual void getStringRepresentation (std::string& str) const;

	/**
     * Creates xpdf object from this object. 
	 *
	 * This function is just to for catching programming errors, it does not make 
	 * any sense to make an xpdf object from a direct object.
	 *
     * @return Xpdf object(s).
     */
	virtual ::Object* _makeXpdfObject () const
	{ 
		assert (!"Failure. This operation is not permitted.");
		throw CObjInvalidOperation ();
	}; 

	
	
};



//=====================================================================================
} /* namespace pdfobjects */
//=====================================================================================


#endif // _CPAGE_H
