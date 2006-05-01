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
 * CPage represents pdf page object. 
 *
 * Content stream is parsed on demand because it can be horribly slow.
 */
class CInlineImage : public CStream
{

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
	 * REMARK: Caller is responsible for deallocating the xpdf object.
     *
     * @return Xpdf object(s).
     */
	virtual ::Object* _makeXpdfObject () const; 

	
};



//=====================================================================================
} /* namespace pdfobjects */
//=====================================================================================


#endif // _CPAGE_H
