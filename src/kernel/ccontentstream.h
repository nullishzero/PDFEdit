// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80
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
		utilsPrintDbg (debug::DBG_DBG, "");
		
		while (!it.isEnd ())
		{
			adjustActualPosition (it.getCurrent(), state);
		
			// Create rectangle from actual position
			Rectangle rc (state.getCurX (), state.getPageHeight () - state.getCurY(), 0, 0);
			
			// DEBUG OUTPUT //
			std::string frst;
			it.getCurrent()->getOperatorName(frst);
			PdfOperator::Operands ops;
			it.getCurrent()->getParameters (ops);
			std::string strop;
			if (0 < ops.size())
				ops[0]->getStringRepresentation (strop);
			utilsPrintDbg (debug::DBG_DBG, rc << " " << frst << " " << strop);
			/////////////////

			if (cmp(rc))
				container.push_back (it.getCurrent());

			it = it.next ();
		}
		
		
		// DEBUG OUTPUT //
		utilsPrintDbg (debug::DBG_DBG, "");
		typename PdfOpStorage::const_iterator itt = container.begin();
		for (; itt != container.end(); ++itt)
		{
			std::string frst;
			(*itt)->getOperatorName(frst);
			PdfOperator::Operands ops;
			(*itt)->getParameters (ops);
			std::string strop;
			if (0 < ops.size())
				ops[0]->getStringRepresentation (strop);
			utilsPrintDbg (debug::DBG_DBG, frst << " " << strop);
		}
		utilsPrintDbg (debug::DBG_DBG, "");
		/////////////////


		
	};
};

//==========================================================
} // namespace operatorparser
//==========================================================


//
// Forward declaration
//
class CContentStream;	
typedef observer::IObserverHandler<CContentStream> CContentStreamObserver;

//==========================================================
// CContentStream
//==========================================================

/**
 * Content stream of a pdf content stream.
 *
 * It will hold parsed objects of the content stream. Change to the stream 
 * object representing the content stream will transparently update this object.
 *
 * This object represents only one of page content streams. Typically a page
 * consist of just one content stream.
 *
 * We know 2 types of pdf operators. Simple and complex. Content stream is a sequence of 
 * operators. Each complex operator can  have children. It is a tree structure, but in most cases
 * it will be just a sequence. 
 *
 * The first level sequence is stored in the CContentStream class.
 * 
 * The stream is processed sequentially and we can process it this way
 * using Iterator designe pattern implemented in these operators.
 *
 * Content stream does not derive from CStream because content stream can
 * consist of several streams.
 * The problem with this object not deriving from IProperty is that, it does not 
 * have an observer. Pdf operators are special objects, that just represent a
 * special type of CStream in a human form. 
 * \TODO make this comment sane
 */
class CContentStream : public noncopyable, public CContentStreamObserver 
{
public:
	typedef std::vector<boost::shared_ptr<PdfOperator> > Operators;

private:

	/** Content stream cobject. */
	boost::shared_ptr<CStream> contentstream;

	/** Parsed content stream operators. */
	Operators operators;

	
	//
	// Observer observing underlying stream
	//
private:	
	/**
	 * Content stream observer.
	 *
	 * If a stream is changed, reparse whole contentstream.
	 */
	struct CContentStreamObserver : public IProperty::Observer
	{
		//
		// Constructor
		//
		CContentStreamObserver (CContentStream* cc) : contentstream (cc)
			{assert (cc);}
		//
		// Observer interface
		//
		virtual void 
		notify (boost::shared_ptr<IProperty> newValue, boost::shared_ptr<const IProperty::ObserverContext> context) const throw();
		//
		//
		//
		virtual priority_t getPriority() const throw ()
			{return 0;};
		//
		// Destructor
		//
		virtual ~CContentStreamObserver () throw () {};

	private:
		CContentStream* contentstream;
	};

	/** CStream observer. */
	boost::shared_ptr<CContentStreamObserver> observer;

	
	//
	// Constructors
	//
public:

	/**
	 * Constructor. 
	 *
	 * @param stream CStream representing content stream or dictionary of more content streams.
	 */
	CContentStream (boost::shared_ptr<CStream> streams);


	//
	// Get methods
	//
public:	
	/**
	 * Get string representation.
	 *
	 * @param str String that will hold the output.
	 */
	template<typename Iter>
	void getStringRepresentation (std::string& str) const
	{
		kernelPrintDbg (debug::DBG_DBG, "");

		if (operators.empty ())
			return;

		// Clear string
		str.clear ();

		// Loop through every operator
		Iter it = PdfOperator::getIterator<Iter> (operators.front());
		while (!it.isEnd())
		{
			std::string tmp;
			it.getCurrent()->getStringRepresentation (tmp);
			str += tmp + " ";
			it.next();
		}
	}
	// Default function
	void getStringRepresentation (std::string& str) const
		{ getStringRepresentation<PdfOperator::Iterator> (str); }

		
	/**
	 * Get objects at specified position.
	 *
	 * @param operators Objects will be added to the back of the container.
	 */
	template<typename OpContainer, typename PdfOpPosComparator>
	void getOperatorsAtPosition (OpContainer& opContainer, const PdfOpPosComparator& cmp, GfxState& state) const
	{
		kernelPrintDbg (debug::DBG_DBG, "");
		if (operators.empty())
			return;
		
		operatorparser::PdfOperatorPositionParser<OpContainer, PdfOpPosComparator> posParser;
		posParser.getOpAtSpecificPosition 
				(PdfOperator::getIterator (operators.front()), opContainer, cmp, state);
	}

	/**
	 * Get pdf operators.
	 * 
	 * @param container Container that will hold all first level operators.
	 */
	template<typename T>
	void getPdfOperators (T& container) const
	{ 
		container.clear ();
		std::copy (operators.begin(), operators.end(), std::back_inserter (container));
	}
	
	//
	// Change methods
	//
public:
	/**
	 * Delete an operator.
	 *
	 * We have to remove it from iterator list and from composite if it is in any.
	 * We also have to be carefull when removing it from iterator list, because
	 * if the operator itself is a composite, we can not easily set its
	 * successor because we do not know it.
	 * 
	 * @param it PdfOperator iterator to delete.
	 */
	void deleteOperator (PdfOperator::Iterator it);
	

	//
	// Helper methods
	//
public:
	/**
	 * Change indicator.
	 * 
	 * @return True if the contentstream was changed, false otherwise.
	 */
	bool empty () const {return operators.empty ();};

	/**
	 * Parse.
	 */
	void reparse ();
	
	//
	// Destructor
	//
public:
	/** Destructor. */
	~CContentStream ()
	{
		kernelPrintDbg (debug::DBG_DBG, "destructing..");
	};
};



//==========================================================
} // namespace pdfobjects
//==========================================================


#endif // _CCONTENTSTREAM_H_
