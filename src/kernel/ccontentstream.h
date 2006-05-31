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
class CContentStream;	
typedef observer::IObserverHandler<CContentStream> CContentStreamObserverSubject;
typedef observer::IObserver<IProperty> IIPropertyObserver;

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
class CContentStream : public noncopyable, public CContentStreamObserverSubject
{
public:
	typedef std::list<boost::shared_ptr<PdfOperator> > Operators;
	typedef std::list<boost::shared_ptr<CStream> > CStreams;
	typedef PdfOperator::Iterator OperatorIterator;
	
private:

	/** Content stream cobject. */
	CStreams cstreams;

	/** Parsed content stream operators. */
	Operators operators;

	/** Gfx state needed for bbox. */
	boost::shared_ptr<GfxState> gfxstate;

	/** Gfx resources needed for bbox. */
	boost::shared_ptr<GfxResources> gfxres;

	//
	// Observer observing underlying cstreams and operands
	//
private:	
	/**
	 * Content stream observer.
	 *
	 * If a stream changes, reparse whole contentstream.
	 *
	 * It can happen that the stream is parsed also after page's Contents entry
	 * has been modified in a way that this content stream no longer exists. 
	 */
	struct CStreamObserver : public IIPropertyObserver
	{
		//
		// Constructor
		//
		CStreamObserver (CContentStream* cc) : contentstream (cc)
			{assert (cc);}
		//
		// Observer interface
		//
		virtual void notify (boost::shared_ptr<IProperty> newValue, 
							 boost::shared_ptr<const IProperty::ObserverContext> context) const throw();
		virtual priority_t getPriority() const throw ()	{return 0;};
		//
		// Destructor
		//
		virtual ~CStreamObserver () throw () {};

	private:
		CContentStream* contentstream;
	};
	/**
	 * Operand stream observer.
	 *
	 * If an operand is changed, save the stream.
	 */
	struct OperandObserver : public IIPropertyObserver
	{
		//
		// Constructor
		//
		OperandObserver (CContentStream* cc) : contentstream (cc)
			{assert (cc);}
		//
		// Observer interface
		//
		virtual void notify (boost::shared_ptr<IProperty> newValue, 
							 boost::shared_ptr<const IProperty::ObserverContext>) const throw();
		virtual priority_t getPriority() const throw ()	{return 0;};
		//
		// Destructor
		//
		virtual ~OperandObserver () throw () {};

	private:
		CContentStream* contentstream;
	};

	/** CStream observer. */
	boost::shared_ptr<CStreamObserver> cstreamobserver;
	/** Operand observer. */
	boost::shared_ptr<OperandObserver> operandobserver;

	
	//
	// Constructors
	//
public:

	/**
	 * Constructor. 
	 *
	 * @param stream CStream representing content stream or dictionary of more content streams.
	 */
	CContentStream (CStreams& strs, 
					boost::shared_ptr<GfxState> state, 
					boost::shared_ptr<GfxResources> res);


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
		utilsPrintDbg (debug::DBG_DBG, "");

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
	 * @param opContainer Container that we fill with operators that are in an enclosing area.
	 * @param cmp Comparator that will decide if an operator is close enough.
	 */
	template<typename OpContainer, typename PdfOpPosComparator>
	void getOperatorsAtPosition (OpContainer& opContainer, const PdfOpPosComparator& cmp) const
	{
		utilsPrintDbg (debug::DBG_DBG, "");
		if (operators.empty())
			return;
			
		ChangeableOperatorIterator it = PdfOperator::getIterator<ChangeableOperatorIterator> (operators.front());
		while (!it.isEnd())
		{
			if (cmp(it.getCurrent()->getBBox()))
				opContainer.push_back (boost::shared_ptr<PdfOperator> (it.getCurrent()));

			// debug
			std::string tmp;
			it.getCurrent()->getOperatorName (tmp);
			utilsPrintDbg (debug::DBG_DBG, tmp << " " << it.getCurrent()->getBBox());
			//
			assert (!it.isEnd());
			it.next();
		}

		// DEBUG OUTPUT
		utilsPrintDbg (debug::DBG_DBG, "--- SELECTED THESE OPERATORS --- ");
		for (typename OpContainer::const_iterator it = opContainer.begin (); it != opContainer.end(); ++it)
		{
			PdfOperator::Operands ops;
			(*it)->getParameters (ops);
			std::string strop;
			if (0 < ops.size())
				ops[0]->getStringRepresentation (strop);
			std::string tmp;
			(*it)->getOperatorName (tmp);
			utilsPrintDbg (debug::DBG_DBG, tmp << "(" << strop << "): " << (*it)->getBBox());
		}
		utilsPrintDbg (debug::DBG_DBG, "--------------------------------- ");
		// 
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
	
	void deleteOperator (boost::shared_ptr<PdfOperator> oper, bool indicateChange = true)
		{ deleteOperator (PdfOperator::getIterator<OperatorIterator> (oper), indicateChange); };

	/**
	 * Insert pdf operator at specified position.
	 *
	 * @param it Position after which the operator will be inserted.
	 * @param newOper Operator that will be inserted.
	 * @param indicateChange If true, change will be written to underlying
	 * stream.
	 */
	void insertOperator (OperatorIterator it, boost::shared_ptr<PdfOperator> newOper, bool indicateChange = true);
	
	void insertOperator (boost::shared_ptr<PdfOperator> oper, boost::shared_ptr<PdfOperator> newOper, bool indicateChange = true)
		{ insertOperator (PdfOperator::getIterator<OperatorIterator> (oper), newOper, indicateChange); };
	
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

	void replaceOperator (boost::shared_ptr<PdfOperator> oper, 
						  boost::shared_ptr<PdfOperator> newOper, 
						  OperatorIterator itPrv,
						  OperatorIterator itNxt,
						  bool indicateChange = true)
		{ replaceOperator (PdfOperator::getIterator<OperatorIterator> (oper), newOper, itPrv, itNxt, indicateChange); };

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
	 * Reparse pdf operators.
	 *
	 * @param state GfxState if changed.
	 * @param res GfxResources if changed.
	 * @param bboxOnly If true only bboxes are set.
	 */
	void reparse (boost::shared_ptr<GfxState> state = boost::shared_ptr<GfxState> (), 
				  boost::shared_ptr<GfxResources> res = boost::shared_ptr<GfxResources> (),
				  bool bboxOnly = false);

	/**
	 * Save content stream to underlying cstream(s).
	 */
	void saveChange () 
		{ _objectChanged(); };

private:
	/**
	 * Save changes.
	 */
	void _objectChanged ();

	//
	// Observers
	//
protected:
	/**
	 * Register observers.
	 */
	void registerCStreamObservers () const;

	/**
	 * Unregister observers.
	 *
	 * This is an important function when saving consten stream consisting of
	 * more streams.
	 */
	void unregisterCStreamObservers () const;
	
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


//boost::shared_ptr<PdfOperator> setPosition (boost::shared_ptr<PdfOperator> oper, const Point pt);
//boost::shared_ptr<PdfOperator> setFontSize (boost::shared_ptr<PdfOperator> oper, size_t fontSize);
// ?? boost::shared_ptr<PdfOperator> setFont (boost::shared_ptr<PdfOperator> oper);
// ?? boost::shared_ptr<PdfOperator> setBackground (const Rectangle rc, RGB color);
 
//
// All changing operators have one drawback. They have to know if they change
// "nonstroking" operations or "stroking" operations.
//

/**
 * Finds a non stroking operator if any.
 *
 * @param oper Operator.
 *
 * @return True if a non stroking operator found, false otherwise.
 */
bool containsNonStrokingOperator (boost::shared_ptr<PdfOperator> oper);

/**
 * Finds stroking operator if any.
 *
 * @param oper Operator.
 *
 * @return True if a stroking operator found, false otherwise.
 */
bool containsStrokingOperator (boost::shared_ptr<PdfOperator> oper);


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
