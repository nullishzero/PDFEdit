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
 * We know 2 types of pdf operators. Simple and complex. Content stream is a sequence of 
 * operators. Each complex operator can  have children. It is a tree structure, but in most cases
 * it will be just a sequence. 
 *
 * The first level sequence is stored in the CContentStream class.
 * 
 * The stream is processed sequentially and we can process it this way
 * using Iterator designe pattern implemented in these operators.
 */
class CContentStream
{
public:
	typedef std::vector<boost::shared_ptr<PdfOperator> > Operators;
	typedef std::vector<boost::shared_ptr<IProperty> > ContentStreams;

private:

	/** Content stream cobject. */
	ContentStreams contentstreams;

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
	CContentStream (ContentStreams& streams, Object* obj = NULL);

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
