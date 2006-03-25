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

// iterator
#include "../utils/iterator.h"

//==========================================================
namespace pdfobjects {
//==========================================================

		
/**
 * This class is the base class for COMPOSITE design pattern and also DECORATOR design pattern.
 * They are very similar in a way (COMPOSITE is a very special case of DECORATOR. 
 *
 * This is a special implementation, somewhat a hybrid between stl and GoF example.
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
	typedef std::vector<boost::shared_ptr<IProperty> > IPContainer; 
	typedef boost::shared_ptr<PdfOperator> ListItem;
	typedef iterator::LinkedListIterator<boost::shared_ptr<PdfOperator> > Iterator;

	// iterator has to be a friend
	friend class iterator::LinkedListIterator<ListItem>;

protected:
	
	/**
	 * Constructor.
	 */
	PdfOperator (ListItem prv, ListItem nxt) : next(nxt), prev(prv) {};


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
	 * Get the string representation of this operator
	 *
	 * @param str String that will hold the representation of this operator.
	 */
	virtual void getStringRepresentation (std::string& str) const = 0;

	
	//
	// Composite interface
	//
public:
	
	/**
	 * Add an operator to the composite.
	 * 
	 * @param oper Operator to be added.
	 */
	virtual void push_back (const boost::shared_ptr<PdfOperator> oper) = 0;

	/**
	 * Remove an operator.
	 *
	 * @param Operator to be erased.
	 */
	virtual void remove (boost::shared_ptr<PdfOperator> oper) = 0;

	/**
	 * Get children count.
	 *
	 * @return Number of children.
	 */
	virtual size_t getChildrenCount () = 0;
	
	
	//
	// Iterator interface
	//

private:
	ListItem next;
	ListItem prev;
	
private:
	
	/**
	 * Get previous item in a list that is implemented by PdfOperator.
	 * It is the same as linked list.
	 *
	 * @return Previous item.
	 */
	ListItem _next () {return next;};
	
	/**
	 * Get next item in a list that is implemented by PdfOperator.
	 * It is the same as linked list.
	 *
	 * @return Next item.
	 */
	ListItem _prev () {return prev;};

};



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



//==========================================================
// Concrete implementations of CompositePdfOperator
//==========================================================


/**
 *
 *
 */




//==========================================================
}
//==========================================================


#endif // _CCONTENTSTREAM_H_
