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
#include "../utils/iterator.h"

// IProperty
//#include "iproperty.h"


//==========================================================
namespace pdfobjects {
//==========================================================

//
// Forward declaration
//
class IProperty;
		
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
	typedef std::list<boost::shared_ptr<IProperty> > 	Operands;
	typedef std::vector<boost::shared_ptr<IProperty> > 	IPContainer; 
	typedef boost::shared_ptr<PdfOperator> 				ListItem;
	typedef iterator::LinkedListIterator<boost::shared_ptr<PdfOperator> > Iterator;

	// iterator has to be a friend
	friend class iterator::LinkedListIterator<ListItem>;

protected:
	
	/**
	 * Constructor.
	 */
	PdfOperator (ListItem prv = ListItem (), ListItem nxt = ListItem ()) : next(nxt), prev(prv) {};


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
	virtual void getParameters (IPContainer& container) const = 0;

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
	virtual void getOperatorName (std::string& first, std::string& last) const = 0;

	
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
	
	//
	// Iterator interface
	//

private:
	ListItem next;
	ListItem prev;

public:
	/**
	 * Set next or prev item.
	 */
	void setNext (ListItem nxt) 
	{ 
		if (next) 
			printDbg (debug::DBG_DBG, "Changing valid next variable.");

		next = nxt; 
	};
	void setPrev (ListItem prv) 
	{ 
		if (prev) 
			printDbg (debug::DBG_DBG, "Changing valid prev variable.");
		
		prev = prv; 
	};
	
	/**
	 * Put behind this object.
	 *
	 * @param beh Object that will be moved behind this object.
	 */
	void putBehind (boost::shared_ptr<PdfOperator> beh)
	{
		// Are we the end
		if (next)
		{
			beh->setNext (next);
			next->setPrev (beh);
			beh->setPrev (ListItem (this));
			next = beh;
		}else
		{
			next = beh;
			beh->setPrev (ListItem (this)); 
		}
	};	
private:
	
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
typedef std::vector<boost::shared_ptr<PdfOperator> > PdfOperators;

private:

	/** List of all operators in this composite. */
	PdfOperators operators;
	
protected:
		
	/**
	 * Constructor.
	 */
	CompositePdfOperator ();
	
public:
	
	//
	// Functions inherited from PdfOperator
	//
	virtual size_t getParametersCount () const = 0;
	virtual void getParameters (IPContainer& container) const = 0;
	virtual void getStringRepresentation (std::string& str) const = 0;
	virtual void getOperatorName (std::string& first, std::string& last) const = 0;
	
	//
	// Add an operator to the composite.
	//
	virtual void push_back (const boost::shared_ptr<PdfOperator> oper);

	//
	// Remove an operator.
	//
	virtual void remove (const boost::shared_ptr<PdfOperator> oper);

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
	 * Initialize opText with second template argument.
	 * 
	 * @param opers This is a stack of operands from which we take number specified
	 * 				by template parameter.
	 */
	SimpleGenericOperator (const char* opTxt, const unsigned int numOper, Operands& opers) : opText (opTxt)
	{
		printDbg (debug::DBG_DBG, "Operator [" << opTxt << "] Operand size: " << numOper << " got " << opers.size());
		assert (opers.size() == numOper);
		if (numOper != opers.size())
			throw MalformedFormatExeption ("Operator operand size mismatch.");

		//
		// Store the operands and remove it from the stack
		//
		for (size_t i = 0; i < numOper; ++i)
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

	virtual void getParameters (IPContainer& container) const
		{ copy (operands.begin(), operands.end (), back_inserter(container) ); };

	virtual void getOperatorName (std::string& first, std::string& last) const
		{ first = opText; last = ""; }
	
	virtual void getStringRepresentation (std::string& str) const
	{
		std::string tmp;
		for (Operands::const_iterator it = operands.begin(); it != operands.end (); ++it)
		{
			tmp = "";
			(*it)->getStringRepresentation (tmp);
			str += tmp + " ";
		}

		// Add operator string
		str += opText;
	};
	
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
	virtual void getParameters (IPContainer& container) const;
	virtual void getStringRepresentation (std::string& str) const;
	virtual void getOperatorName (std::string& first, std::string& last) const;
	
};



//==========================================================
// Concrete implementations of CompositePdfOperator
//==========================================================


/**
 *
 *
 */
template<typename T>
class UnknownCompositePdfOperator : public CompositePdfOperator
{
	
};


//==========================================================
}
//==========================================================


#endif // _PDFOPERATORS_H_
