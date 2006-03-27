/*
 * =====================================================================================
 *        Filename:  ccontentstream.h
 *         Created:  03/24/2006 10:34:03 PM CET
 *          Author:  jmisutka ()
 * =====================================================================================
 */

#ifndef _CCONTENTSTREAM_H_
#define _CCONTENTSTREAM_H_

// static includes
#include "static.h"

// 
#include "iproperty.h"
// iterator
#include "pdfoperators.h"


//==========================================================
namespace pdfobjects {
//==========================================================


/**
 * Content stream of a pdf content stream.
 *
 * It will hold parsed objects of the content stream. Change to the stream 
 * object representing the content stream will not affect this object only changes
 * made by this object's methods.
 *
 */
class CContentStream
{
public:
	typedef std::vector<boost::shared_ptr<PdfOperator> > Operators;

private:

	/** Content stream cobject. */
	boost::shared_ptr<IProperty> contentstream;

	/** Parsed content stream operators. */
	Operators operators;

public:

	/**
	 * Constructor. 
	 *
	 * Object is not freed.
	 *
	 * @param stream CStream representing content stream or dictionary of more content streams.
	 * @param obj Object representing content stream.
	 */
	CContentStream (boost::shared_ptr<IProperty> stream, Object* obj = NULL);

	
	/**
	 * Get string representation.
	 *
	 * @param str String that will hold the output.
	 */
	void getStringRepresentation (std::string& str) const;
			
	
	
};




//==========================================================
} // namespace pdfobjects
//==========================================================


#endif // _CCONTENTSTREAM_H_
