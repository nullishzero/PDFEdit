// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80
/*
 * =====================================================================================
 *        Filename:  pdfoperators.h
 *         Created:  03/26/2006 16:54:03 PM CET
 *          Author:  jmisutka ()
 * =====================================================================================
 */

#ifndef _PDFOPERATORS_H_
#define _PDFOPERATORS_H_

// static includes
#include "static.h"

// iterator
#include "utils/iterator.h"


//==========================================================
namespace pdfobjects {
//==========================================================

//
// Forward declaration
//
class IProperty;
class CContentStream;
		
//==========================================================
// PdfOperator
//==========================================================
	
/**
 * This class is the base class for COMPOSITE design pattern and also DECORATOR design pattern.
 * 
 * They are very similar in a way (COMPOSITE is a very special case of DECORATOR. This is a 
 * special implementation, somewhat a hybrid between stl and GoF example.
 *
 * We have content stream consisting of operators and operands and also objects. 
 * These objects are composits. 
 *
 * Perhaps we would like to construct something like CBoldText and 
 * we would like the text to be also a superscript so we use DECORATOR pattern.
 *
 */
class PdfOperator
{	
		
public:
	typedef std::deque<boost::shared_ptr<IProperty> > 	 Operands;
	typedef boost::weak_ptr<PdfOperator>				 ListItem;
	typedef iterator::LinkedListIterator<ListItem> 		 Iterator;
	typedef std::vector<boost::shared_ptr<PdfOperator> > PdfOperators;

	// iterator has to be a friend
	friend class iterator::LinkedListIterator<ListItem>;

	//
	// CContentStream pointer
	//
private:
	/** This enables mapping between pdfoperator and contentstream. */
	CContentStream* contentstream;
	
	//
	// Constructor
	// 
protected:
	
	/**
	 * Constructor.
	 */
	PdfOperator (ListItem prv = ListItem(), ListItem nxt = ListItem()) : contentstream (NULL), next(nxt), prev(prv) {};


	//
	// Destructor
	//
public:

	/**
	 * Destructor.
	 */
	virtual ~PdfOperator () {};

	
	//
	// Pdf operator interface
	//
public:
	
	/**
	 * Return the number of parameters.
	 *
	 * @return Number of parameters.
	 */
	virtual size_t getParametersCount () const = 0;

	/**
	 * Get the parameters used with this operator.
	 *
	 * @param container Will be used to store parameters.
	 */
	virtual void getParameters (Operands& container) const = 0;

	/**
	 * Get the string representation of this operator.
	 *
	 * @param str String that will hold the representation of this operator.
	 */
	virtual void getStringRepresentation (std::string& str) const = 0;

	/**
	 * Get the string operator name.
	 *
	 * @param str String that will hold operator name.
	 */
	virtual void getOperatorName (std::string& first) const = 0;

	
	//
	// Composite interface
	//
public:
	
	/**
	 * Add an operator to the composite.
	 * 
	 * @param oper Operator to be added.
	 */
	virtual void push_back (const boost::shared_ptr<PdfOperator>)
		{ throw NotImplementedException ("PdfOperator::push_back ()"); };

	/**
	 * Remove an operator.
	 *
	 * @param Operator to be erased.
	 */
	virtual void remove (boost::shared_ptr<PdfOperator>)
		{ throw NotImplementedException ("PdfOperator::remove ()"); };

	/**
	 * Get children count.
	 *
	 * @return Number of children.
	 */
	virtual size_t getChildrenCount () const
		{ return 0; };	
	
	/**
	 * Get childrens.
	 *
	 * @param p Children container.
	 */
	virtual void getChildren (PdfOperators&) const
		{ throw NotImplementedException ("PdfOperator::getChildrens ()"); };	
	
	//
	// Iterator interface
	//

private:
	ListItem next;
	ListItem prev;

public:

	/**
	 * Get iterator.
	 * 
	 * REMARK: We can't simply make getIterator a method, because we have to
	 * know the smart pointer behind the object. Member variable "this" is not
	 * sufficient.
	 *
	 * @param op Operator from which we want the iterator.
	 * 
	 * @return Iterator.
	 */
	static Iterator getIterator (boost::shared_ptr<PdfOperator> op) 
		{ return Iterator (ListItem (op)); };

	/**
	 * Put behind.
	 * 
	 * REMARK: We can't simply make putBehind a method, because we have to
	 * know the smart pointer behind the object. Member variable "this" is not
	 * sufficient.
	 *
	 * @behindWhich Behind which elemnt.
	 * @which 		Which element.
	 */
	static void putBehind (	boost::shared_ptr<PdfOperator> behindWhich, 
							boost::shared_ptr<PdfOperator> which)
	{
		if (behindWhich && which)
		{
			if (!behindWhich->next.expired ())
			{ // we are not at the end
			
				which->setNext (behindWhich->next);
				assert (behindWhich->next.lock());
				behindWhich->next.lock()->setPrev (ListItem (which));
				which->setPrev (ListItem (behindWhich));
				behindWhich->setNext (ListItem (which));
				
			}else
			{ // we are at the end
				
				behindWhich->setNext (ListItem (which));
				which->setPrev (ListItem (behindWhich));
			}
		
		}else
			throw CObjInvalidOperation ();
	}
	
	/**
	 * Set next or prev item.
	 */
	void setNext (boost::shared_ptr<PdfOperator> nxt) 
		{ setNext (ListItem (nxt));	};
	void setPrev (boost::shared_ptr<PdfOperator> prv) 
		{ setPrev (ListItem (prv)); };
	
private:

	/**
	 * Set next or prev item.
	 */
	void setNext (ListItem nxt) 
	{ 
		if (!next.expired ())
			utilsPrintDbg (debug::DBG_DBG, "Changing valid next variable.");

		next = nxt;
	};
	void setPrev (ListItem prv) 
	{ 
		if (!prev.expired ()) 
			utilsPrintDbg (debug::DBG_DBG, "Changing valid prev variable.");
		
		prev = prv; 
	};

	/**
	 * Get previous item in a list that is implemented by PdfOperator.
	 * It is the same as linked list.
	 *
	 * @return Previous item.
	 */
	ListItem _next () const {return next;};
	
	/**
	 * Get next item in a list that is implemented by PdfOperator.
	 * It is the same as linked list.
	 *
	 * @return Next item.
	 */
	ListItem _prev () const {return prev;};

	//
	// CContentStream interface
	//
public:
	/**
	 * Set content stream.
	 *
	 * Every pdf operator needs to know into which content stream it belongs.
	 * This is due to the fact, that some functions just return container of 
	 * pdfoperators, and if we want to work with them, we have to know the
	 * contentstream.
	 *
	 * We can find out the CStream but there is no mapping between CStream and
	 * content stream.
	 *
	 * @param cs Content stream.
	 */
	void setContentStream (CContentStream* cs)
		{assert (NULL == contentstream); contentstream = cs;}
	
	
	/**
	 * Get content stream.
	 *
	 * @return Content stream that this pdfoperator belongs to.
	 */
	CContentStream* getContentStream () const
		{assert (NULL != contentstream); return contentstream;}
	
};



//==========================================================
// CompositePdfOperator
//==========================================================

/**
 * Composite object. It is just an interface.
 *
 *
 */
class CompositePdfOperator : public PdfOperator
{
private:

	/** Child operator, where all calls are redirected. */
	PdfOperators children;

protected:
		
	/**
	 * Constructor.
	 *
	 * Implementation of the Decorator design pattern. 
	 *
	 * Be carefull, this function does NOT set prev and next, because it does
	 * not have smart pointer to this object !
	 * 
	 * @param op Operator from which we will inherit all its children.
	 */
	CompositePdfOperator (boost::shared_ptr<PdfOperator> op)
		{children.push_back (op);};

	/**
	 * Constructor.
	 * @param ops Operators from which we will inherit all its children.
	 */
	CompositePdfOperator (PdfOperators* ops = NULL)
	{
		if (NULL != ops)
			copy (ops->begin(), ops->end (), back_inserter (children));
	};

	//
	// Composite interface
	//
public:
	virtual size_t getChildrenCount () const {return children.size ();};	
	virtual void push_back (const boost::shared_ptr<PdfOperator> op);
	virtual void remove (boost::shared_ptr<PdfOperator> op);
	virtual void getChildren (PdfOperators& container) const;

	//
	// PdfOperator interface
	//
public:
	virtual size_t getParametersCount () const {return 0;};
	virtual void getParameters (Operands&) const {};
	virtual void getStringRepresentation (std::string& str) const;
	virtual void getOperatorName (std::string& first) const = 0;

public:
	
	/**
	 * Destructor.
	 */
	virtual ~CompositePdfOperator() {};
		
};


//==========================================================
// Concrete implementations of PdfOperator
//==========================================================

/**
 * Almost all simple operators will be constructed by specifying types of operands and the
 * text representation of the operator. 
 *
 * <cref exception="MalformedFormatExeption"> Thrown when the operands do not match the specification.
 * 
 */
class SimpleGenericOperator : public PdfOperator
{
private:
	/** Operands. */
	Operands operands;

	/** Text representing the operator. */
	const char* opText;
	
public:

	/** 
	 * Constructor. 
	 * Create it as a standalone object. Prev and Next are not valid.
	 *
	 * @param opTxt Operator name text representation.
	 * @param numOper (Maximum) Number of operands.
	 * @param opers This is a stack of operands from which we take number specified
	 * 				by numOper or while any operand left.
	 */
	SimpleGenericOperator (const char* opTxt, const size_t numOper, Operands& opers) : opText (opTxt)
	{
		//utilsPrintDbg (debug::DBG_DBG, "Operator [" << opTxt << "] Operand size: " << numOper << " got " << opers.size());
		assert (numOper >= opers.size());
		if (numOper < opers.size())
			throw MalformedFormatExeption ("Operator operand size mismatch.");

		//
		// Store the operands and remove it from the stack
		// REMARK: the op count can vary ("scn" operator takes arbitrary number of
		// parameters)
		//
		for (size_t i = 0; (i < numOper) && !opers.empty(); ++i)
		{
			Operands::value_type val = opers.back ();
			// Store the last element of input parameter
			operands.push_front (val);
			// Remove the element from input parameter
			opers.pop_back ();
		}
	};

	//
	// PdfOperator interface
	//
public:

	virtual size_t getParametersCount () const
		{ return operands.size (); };

	virtual void getParameters (Operands& container) const
		{ copy (operands.begin(), operands.end (), back_inserter(container) ); };

	virtual void getOperatorName (std::string& first) const
		{ first = opText;}
	
	virtual void getStringRepresentation (std::string& str) const;
};




/**
 * This class consumes all operands from "stack", it should be safe.
 */
class UnknownPdfOperator : public PdfOperator
{
private:
	/** Operands. */
	Operands operands;

	/** Text representing the operator. */
	std::string opText;

public:
	
	/** Constructor. */
	UnknownPdfOperator (Operands& opers, const std::string& opTxt);
		
	//
	// PdfOperator interface
	//
	virtual size_t getParametersCount () const;
	virtual void getParameters (Operands& container) const;
	virtual void getStringRepresentation (std::string& str) const;
	virtual void getOperatorName (std::string& first) const;
	
};



//==========================================================
// Concrete implementations of CompositePdfOperator
//==========================================================


/**
 *
 *
 */
class UnknownCompositePdfOperator : public CompositePdfOperator
{
private:
		
	/** Text representing the beginning operator. */
	const char* opBegin;
	/** Text representing the ending operator. */
	const char* opEnd;

public:
	
	/** 
	 * Constructor. 
	 * Create it as a standalone object. Prev and Next are not valid.
	 *
	 * @param opBegin_ Start operator name text representation.
	 * @param opEnd_ End operator name text representation.
	 */
	UnknownCompositePdfOperator (const char* opBegin_, const char* opEnd_);

public:
	virtual void getStringRepresentation (std::string& str) const;
	virtual void getOperatorName (std::string& first) const {first = opBegin;};

};


//
// Forward declaration
//
class CInlineImage;

/**
 * Inline image pdf operator.
 *
 */
class InlineImageCompositePdfOperator : public CompositePdfOperator
{
private:
		
	/** Text representing the beginning operator. */
	const char* opBegin;
	/** Text representing the ending operator. */
	const char* opEnd;
	/** Stream representing inline image. */
	boost::shared_ptr<CInlineImage> inlineimage;

public:
	
	/** 
	 * Constructor. 
	 * Create it as a standalone object. Prev and Next are not valid.
	 *
	 * @param opBegin_ Start operator name text representation.
	 * @param opEnd_ End operator name text representation.
	 */
	InlineImageCompositePdfOperator (const char* opBegin_, const char* opEnd_, boost::shared_ptr<CInlineImage> im_);

public:
	virtual void getStringRepresentation (std::string& str) const;
	virtual void getOperatorName (std::string& first) const {first = opBegin;};

};

//==========================================================
}
//==========================================================


#endif // _PDFOPERATORS_H_
