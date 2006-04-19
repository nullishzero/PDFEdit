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

// DEBUG output needs it
#include "iproperty.h"

//
#include "cobject.h"

// PdfOperator
#include "pdfoperators.h"

//==========================================================
namespace pdfobjects {
//==========================================================

//
// Forward declaration
//
class IProperty;

//==========================================================
namespace operatorparser  {
//==========================================================

/** 
 * Adjust actual position. 
 *
 * @param op Actual operator.
 * @param state Actual state that we should update.
 */
void adjustActualPosition (boost::shared_ptr<PdfOperator> op, GfxState& state);
	
		
/**
 * PdfOperators parser.
 * 
 */
template<typename PdfOpStorage, typename PdfOpPosComparator>
class PdfOperatorPositionParser
{
public:

	/**
	 * Stores all operators at specific position.
	 *
	 * @param it Iterator that will be used to traverse all operators in
	 * 				specific order.
	 * @param container Container used to store all operators that are in a
	 * 				specificied area.
	 * @param comp Comparator that will decide whether an operator is in the
	 * 				specified area or not.
	 */
	void
	getOpAtSpecificPosition (PdfOperator::Iterator it, 
							PdfOpStorage& container, 
							const PdfOpPosComparator& cmp, 
							GfxState& state)
	{
		printDbg (debug::DBG_DBG, "");
		
		while (!it.isEnd ())
		{
			adjustActualPosition (it.getCurrent().lock(), state);
		
			// Create rectangle from actual position
			Rectangle rc (state.getCurX (), state.getPageHeight () - state.getCurY(), 0, 0);
			
			// DEBUG OUTPUT //
			std::string frst;
			it.getCurrent().lock()->getOperatorName(frst);
			PdfOperator::Operands ops;
			it.getCurrent().lock()->getParameters (ops);
			std::string strop;
			if (0 < ops.size())
				ops[0]->getStringRepresentation (strop);
			printDbg (debug::DBG_DBG, rc << " " << frst << " " << strop);
			/////////////////

			if (cmp(rc))
				container.push_back (boost::shared_ptr<PdfOperator> (it.getCurrent()));

			it = it.next ();
		}
	};
};

//==========================================================
} // namespace operatorparser
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
	typedef std::vector<boost::shared_ptr<CStream> > ContentStreams;

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
	CContentStream (boost::shared_ptr<CStream> stream, Object* obj = NULL);

	
	/**
	 * Get string representation.
	 *
	 * @param str String that will hold the output.
	 */
	void getStringRepresentation (std::string& str) const;
			
	/**
	 * Get objects at specified position.
	 *
	 * @param operators Container that will hold all objects at position.
	 */
	template<typename OpContainer, typename PdfOpPosComparator>
	void getOperatorsAtPosition (OpContainer& opContainer, const PdfOpPosComparator& cmp, GfxState& state) const
	{
		printDbg (debug::DBG_DBG, "");
		if (operators.empty())
			return;
		
		operatorparser::PdfOperatorPositionParser<OpContainer, PdfOpPosComparator> posParser;
		posParser.getOpAtSpecificPosition 
				(PdfOperator::getIterator (operators.front()), opContainer, cmp, state);
	}
	
	
	/** Destructor. */
	~CContentStream ()
	{
		printDbg (debug::DBG_DBG, "destructing..");
	};
};



//==========================================================
} // namespace pdfobjects
//==========================================================


#endif // _CCONTENTSTREAM_H_
