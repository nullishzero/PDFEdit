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
 * Content stream class representing pdf content stream.
 *
 * This object represents operators and operands of a content stream. Each content stream consists of
 * one or more indirect streams. These streams are listed in "Contents" entry of page dictionary. 
 *
 * Each operator represents an operation that alters the current graphic state.
 * These operators are processed sequentially. We cannot parse only one stream
 * when the content stream consists of more streams, because the split point is
 * almost arbitrary. Every content stream class represents a valid content which
 * can consist of one or more streams.
 * 
 * During initialization of this object, observers are registred on all
 * underlying streams, which means a change to the stream object(s) representing 
 * the content stream will transparently update this object. This feature could be
 * tricky. (E.g. Content stream consists of more streams and we need
 * to save the content stream, which means changing streams one after another.
 * After a change the content stream becomes invalid.)
 * 
 * Operators form a tree-like structure consisting of Simple objecst and Composite objects. 
 * \todo reference to general documentation [pdfoperators].
 * Only first level operators are stored.
 * 
 * The pdf feature that a content stream can consist of several streams means we
 * can not derive from CStream object. Due to this limitation we do not have 
 * Observer interface so we need to implement it.
 *
 * Mainly this object is responsible for all visible objects on a page. If a
 * content stream is empty, the page is empty. Changing operators can be very
 * destructive (e.g. text matrix, font type, deleting one operator of a pair,
 * ...)
 *
 * The requirement of processing operators sequentially has lead to the decision
 * that these operators will be in an iterator queue meainig we can process them
 * seqeuntially with the advantage of Iterator design pattern.
 * \todo reference to general documentation [pdfoperators].
 * Another characteristis of content stream operators is that they build a tree
 * like structure which has lead to another decision that these operators will
 * be in a tree like queue. It means operators are designed as a Composite
 * design pattern. This enables (e.g. gui to represent content stream in a human
 * readable from)
 */
class CContentStream : public noncopyable, public CContentStreamObserverSubject
{
public:
	typedef std::list<boost::shared_ptr<PdfOperator> > Operators;
	typedef std::list<boost::shared_ptr<CStream> > CStreams;
	typedef PdfOperator::Iterator OperatorIterator;
	
private:

	/** Underlying cstream objects. */
	CStreams cstreams;

	/** Parsed first level content stream operators. */
	Operators operators;

	/** Graphical state. */
	boost::shared_ptr<GfxState> gfxstate;

	/** Graphical resources. */
	boost::shared_ptr<GfxResources> gfxres;

	//
	// Observer observing underlying cstreams and operands
	//
private:	
	/**
	 * Content stream observer.
	 *
	 * After a cstream changes reparse the contentstream.
	 *
	 * It can happen that the stream is parsed also after page's Contents entry
	 * has been modified in a way that this content stream no longer exists. It
	 * depends on the position of this observer in cstream observer list.
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
	 * If an operand is changed, save the stream notifying all observers.
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

	/** Observer observing underlying cstreams. */
	boost::shared_ptr<CStreamObserver> cstreamobserver;
	/** Observer observing operands of all operators. */
	boost::shared_ptr<OperandObserver> operandobserver;

	
	//
	// Constructors
	//
public:

	/**
	 * Constructor. 
	 *
	 * @param strs Array of content streams. Typically only one.
	 * @param state Graphical state.
	 * @param res Graphical resources.
	 */
	CContentStream (CStreams& strs, 
					boost::shared_ptr<GfxState> state, 
					boost::shared_ptr<GfxResources> res);


	//
	// Get methods
	//
public:	
	/**
	 * Get the string representation of specific operators. 
	 *
	 * Traverse all operators using specific iterator and save their string
	 * repersentation. 
	 *
	 * REMARK: If an iterator accepts composite objects and also simple objects and a composite
	 * contains one of such simple objects, the 
	 * string representation will be incorrect. It will contain the string
	 * represenation of the composite including the child and also a separate
	 * string representation of the child.
	 *
	 * @param str Output string.
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
	 * Get the string representation of the content stream.
	 *
	 * @param str Output string.
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
	 * Get objects at position.
	 *
	 * We can compute a bounding box (rectangle) to every operator specifying its exact position.
	 * This bounding box is used to select specific operators.
	 *
	 * @param opContainer Output container.
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
	 * Get first level pdf operators.
	 *
	 * Operators form a tree-like structure. We save all root operands to
	 * container.
	 * 
	 * @param container Output container.
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
	 * Delete an operator from a content stream.
	 *
	 * We have to remove an operator from the iterator queue and also from the tree queue.
	 * 
	 * When removing from the tree queue an operator we have to find it and remove
	 * from the composite it is in (if not first level)
	 *
	 * Needs to be careful when removing from the iterator queue. If a composite is removed
	 * we have to update its top level operator and also the last item of the
	 * iterator queue that is still in the composite.
	 * 
	 * @param it Iterator pointing to operator that will be deleted.
	 * @param indicateChange If true, changed contentstream will be written to its cstreams, 
	 * otherwise the change will not be visible.
	 */
	void deleteOperator (OperatorIterator it, bool indicateChange = true);
	
	void deleteOperator (boost::shared_ptr<PdfOperator> oper, bool indicateChange = true)
		{ deleteOperator (PdfOperator::getIterator<OperatorIterator> (oper), indicateChange); };

	/**
	 * Insert pdf operator after specified position. If it is not valid it is
	 * assumend that the content stream is empty and operator will be inserted
	 * at the beginning. 
	 *
	 * We have to insert the operator into the iterator queue and also intto the tree  queue.
	 *
	 * @param it Iterator pointing to operator after which the new operator will be inserted.
	 * @param newOper Operator that will be inserted.
	 * @param indicateChange If true, changed contentstream will be written to its cstreams, 
	 * otherwise the change will not be visible.
	 */
	void insertOperator (OperatorIterator it, boost::shared_ptr<PdfOperator> newOper, bool indicateChange = true);

	/** \see insertOperator */
	void insertOperator (boost::shared_ptr<PdfOperator> oper, boost::shared_ptr<PdfOperator> newOper, bool indicateChange = true)
		{ insertOperator (PdfOperator::getIterator<OperatorIterator> (oper), newOper, indicateChange); };
	
	/**
	 * Insert pdf operator in the front. 
	 *
	 * @param newOper Operator that will be inserted.
	 * @param indicateChange If true, changed contentstream will be written to its cstreams, 
	 * otherwise the change will not be visible.
	 */
	void frontInsertOperator (boost::shared_ptr<PdfOperator> newOper, bool indicateChange = true);


	/**
	 * Replace an operator with another one.
	 *
	 * We have to modify both iterator queue and tree queue.
	 *
	 * This can be a problem. When replacing an operator with another operator
	 * which contains the same instance of the original operator (meaning
	 * the new operator is a composite) we can not use
	 * iterator queue of the original, because it was altered when inserting it
	 * into the new operator and points to items in the new operator.
	 *
	 * All change functions (e.g. operatorSetColor) behave this way. They insert
	 * the original operator in a new composite altering the iterator queue and
	 * making the content stream iterator queue invalid. That is why we need 
	 * the next and previous items of the original operator as function
	 * arguments.
	 *
	 * @param it Iterator pointing to the element that will be replaced.
	 * @param newOper New operator.
	 * @param itPrv Previous iterator of newOper in iterator queue
	 * @param itNxt Next iterator of newOper in iterator queue
	 * @param indicateChange If true, changed contentstream will be written to its cstreams, 
	 * otherwise the change will not be visible.
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
	 * Is the content stream empty.
	 * 
	 * @return True if the contentstream is empty, false otherwise.
	 */
	bool empty () const {return operators.empty ();};

	/**
	 * Reparse pdf operators and set their bounding boxes.
	 *
	 * @param bboxOnly If true only bounding boxes are set, if false operators
	 * are also reparsed.
	 * @param state Graphical state, if changed.
	 * @param res Graphical resources, if changed.
	 */
	void reparse (bool bboxOnly = false,
				  boost::shared_ptr<GfxState> state = boost::shared_ptr<GfxState> (), 
				  boost::shared_ptr<GfxResources> res = boost::shared_ptr<GfxResources> ());

	/**
	 * Save content stream to underlying cstream(s) and notify all observers. 
	 *
	 * Does not reparse anything. 
	 */
	void saveChange () 
		{ _objectChanged(); };

private:
	/**
	 * Save changes and indicate that the object has changed by calling all
	 * observers.
	 */
	void _objectChanged ();

	//
	// Observers
	//
protected:
	/**
	 * Register observers on all cstreams that this object consists of.
	 *
	 * This function is called in constructor and also after saving to more
	 * cstreams.
	 */
	void registerCStreamObservers () const;

	/**
	 * Unregister observers from all cstreams that this object consists of.
	 *
	 * This function is called when saving consten stream consisting of
	 * more streams. If we do not unregister observers, we would be notified
	 * that a stream has changed after the first save (when the content stream
	 * is invalid) and our observer would want to reparse an invalid stream.
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
//boost::shared_ptr<PdfOperator> createTextOperator (boost::shared_ptr<PdfOperator> oper);
// (text) Td
//boost::shared_ptr<PdfOperator> createText (const std::string text);
	

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
 * Tries to find first non stroking operator.
 *
 * Some operators are modified by stroking operators some by nonstroking. 
 * (e.g. when changing color, we can change color either of a stroking operator using RG
 * operator or of a nonstroking operator using rg operator. RG operator does not
 * change color of nonstroking operators.)
 *
 * @param oper Pdf operator that will be searched for a non stroking
 * operator.
 *
 * @return True if found, false otherwise.
 */
bool containsNonStrokingOperator (boost::shared_ptr<PdfOperator> oper);

/** 
 * Tries to find first stroking operator.
 *  
 * Some operators are modified by stroking operators some by nonstroking. 
 * (e.g. when changing color, we can change color either of a stroking operator using RG
 * operator or of a nonstroking operator using rg operator. RG operator does not
 * change color of nonstroking operators.)
 *
 * @param oper Pdf operator that will be searched for a stroking operator.
 *
 * @return True if found, false otherwise.
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
 * @param oper Operator that will be changed.
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
