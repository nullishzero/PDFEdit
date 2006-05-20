// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80
/*
 * =====================================================================================
 *        Filename:  cinlineimage.h
 *     Description:  
 *         Created:  01/05/2006 11:46:14 AM CET
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


//=====================================================================================

/**
 * Inline image is a special XObject. It can not be referenced from outside the
 * content stream. It is a direct object. And therefore it can inherit from
 * CStream.
 *
 * It has several advantegas. One of them is, that it transparently handles
 * different string representation than normal stream, simply by overloading
 * getStringRepresentation method.
 */
class CInlineImage : public CStream
{
	//
	// Constructors
	//
public:
	/**
	 *
	 */
	CInlineImage (::Object& objDict, const CStream::Buffer& buffer);
	CInlineImage (CPdf& p, ::Object& objDict, const CStream::Buffer& buffer, const IndiRef& rf);

	/**
	 * Default constructor. CStream default constructor is called.
	 */
	CInlineImage () {};

	
	//
	// Cloning
	//
protected:
	virtual CStream* _newInstance () const
		{ return new CInlineImage; };

	
	//
	// Get methods
	//
public:
	
	/**
	 * Returns string representation of actual object.
	 *
	 * REMARK: String can contain also NOT printable characters.
	 *
	 * @param str String representation.
	 */
	virtual void getStringRepresentation (std::string& str) const;

	/**
     * Create xpdf object.
	 *
	 * This function does not make any sense with only direct object.
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
