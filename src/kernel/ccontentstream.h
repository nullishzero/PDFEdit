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
 * @param res Actual resources for lookup fonts.
 * @param rc Return rectangle for PdfOperator op.
 */
void adjustActualPosition (boost::shared_ptr<PdfOperator> op, GfxState& state, boost::shared_ptr<GfxResources> res, Rectangle* rc);
	
		
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
				 GfxState& state,
				 boost::shared_ptr<GfxResources> res)
	{
		utilsPrintDbg (debug::DBG_DBG, "");
		
		while (!it.isEnd ())
		{
			boost::shared_ptr<Rectangle> rc(new Rectangle());
			adjustActualPosition (it.getCurrent(), state, res, rc.get());
		
			// DEBUG OUTPUT //
			std::string frst;
			it.getCurrent()->getOperatorName(frst);
			PdfOperator::Operands ops;
			it.getCurrent()->getParameters (ops);
			std::string strop;
			if (0 < ops.size())
				ops[0]->getStringRepresentation (strop);
			utilsPrintDbg (debug::DBG_DBG, *rc.get() << " " << frst << " " << strop);
			/////////////////

			if (cmp(*rc.get()))
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
	typedef std::list<boost::shared_ptr<PdfOperator> > Operators;
	typedef PdfOperator::Iterator OperatorIterator;

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
	 * Get string representation. We traverse operators using specific
	 * iterators. 
	 *
	 * REMARK: If an iterator accepts composite objects and also its children, the 
	 * string representation will be incorrect. It will contain the string
	 * represenation of the composite including the child and also a separate
	 * string representation of the child.
	 *
	 * @param str String that will contain the output.
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
	
	/**
	 * Get string representation.
	 *
	 * @param str String that will contain the output.
	 */

	void getStringRepresentation (std::string& str) const
	{
		kernelPrintDbg (debug::DBG_DBG, "");

		if (operators.empty ())
			return;

		// Clear string
		str.clear ();

		// Loop through every operator
		for (Operators::const_iterator it = operators.begin(); it != operators.end(); ++it)
		{
			std::string tmp;
			(*it)->getStringRepresentation (tmp);
			str += tmp + " ";
		}	
	}

		
	/**
	 * Get objects at specified position.
	 *
	 * @param operators Objects will be added to the back of the container.
	 */
	template<typename OpContainer, typename PdfOpPosComparator>
	void getOperatorsAtPosition (OpContainer& opContainer, const PdfOpPosComparator& cmp, GfxState& state, boost::shared_ptr<GfxResources> res) const
	{
		kernelPrintDbg (debug::DBG_DBG, "");
		if (operators.empty())
			return;
		
		operatorparser::PdfOperatorPositionParser<OpContainer, PdfOpPosComparator> posParser;
		posParser.getOpAtSpecificPosition 
				(PdfOperator::getIterator (operators.front()), opContainer, cmp, state, res);
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
	 * @param indicateChange If true, change will be written to underlying
	 * stream.
	 */
	void deleteOperator (OperatorIterator it, bool indicateChange = true);

	/**
	 * Insert pdf operator at specified position.
	 *
	 * @param it Position after which the operator will be inserted.
	 * @param newOper Operator that will be inserted.
	 * @param indicateChange If true, change will be written to underlying
	 * stream.
	 */
	void insertOperator (OperatorIterator it, boost::shared_ptr<PdfOperator> newOper, bool indicateChange = true);
	
	/**
	 * Replace an operator.
	 *
	 * This is a problem due to the iterator list. In replace functions like
	 * changeColor, we put new oper in a new composite so we change its iterator
	 * previous and next items which means we can not use them in this function.
	 *
	 * @param it Position of the element that will be replaced.
	 * @param newOper Operator that will replace operator pointed by it.
	 * @param prev Previous iterator of newOper in iterator list
	 * @param next Next iterator of newOper in iterator list
	 * @param indicateChange If true, change will be written to underlying
	 */
	void replaceOperator (OperatorIterator it, 
						  boost::shared_ptr<PdfOperator> newOper, 
						  OperatorIterator itPrv,
						  OperatorIterator itNxt,
						  bool indicateChange = true);

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

	/**
	 * Save content stream to underlying cstream.
	 */
	void saveChange () 
		{ _objectChanged(); };

private:
	/**
	 * Save changes.
	 */
	void _objectChanged ();
	
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
// Operator creating functions
//==========================================================

// BT oper ET
boost::shared_ptr<PdfOperator> createTextOperator (boost::shared_ptr<PdfOperator> oper);
// (text) Td
boost::shared_ptr<PdfOperator> createText (const std::string text);
	

//==========================================================
// Operator changing functions
//==========================================================


boost::shared_ptr<PdfOperator> setPosition (boost::shared_ptr<PdfOperator> oper, const Point pt);
boost::shared_ptr<PdfOperator> setFontSize (boost::shared_ptr<PdfOperator> oper, size_t fontSize);
// ?? boost::shared_ptr<PdfOperator> setFont (boost::shared_ptr<PdfOperator> oper);
// ?? boost::shared_ptr<PdfOperator> setBackground (const Rectangle rc, RGB color);
 



/**
 * Set color of an operator in RGB format.
 *
 * Example: 1 0 0 is red
 *
 * REMARK: When changing an operator that is in a content stream, we can not use
 * Iterator list after calling this function. This function puts oper in a new
 * iterator list. Method replaceOperator has to be called immmediately after
 * this function.
 * 
 * @param r R in RGB
 * @param g G in RGB
 * @param b B in RGB
 *
 * @return Operator that contains changed original operator.
 */
boost::shared_ptr<PdfOperator>
operatorSetColor (boost::shared_ptr<PdfOperator> oper, double r, double g, double b);

//==========================================================
} // namespace pdfobjects
//==========================================================


#endif // _CCONTENTSTREAM_H_
