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
struct TextOperatorIterator;
struct InlineImageOperatorIterator;
							 
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
	typedef std::deque<boost::shared_ptr<IProperty> > 	 			Operands;
	typedef iterator::SharedDoubleLinkedListIterator<PdfOperator>	Iterator;
	typedef Iterator::ListItem										ListItem;
	typedef std::list<boost::shared_ptr<PdfOperator> > 				PdfOperators;

	// iterator has to be a friend
	friend class iterator::SharedDoubleLinkedListIterator<PdfOperator>;

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
	 * Add an operator to the end of composite.
	 * 
	 * @param oper Operator to be added.
	 */
	virtual void push_back (const boost::shared_ptr<PdfOperator>)
		{ throw NotImplementedException ("PdfOperator::push_back ()"); };

	/**
	 * Insert an operator after an item.
	 * 
	 * @param oper Operator after which an oper will be inserted.
	 * @param newOper Operator to be inserted.
	 */
	virtual void insert_after (const boost::shared_ptr<PdfOperator>, boost::shared_ptr<PdfOperator>)
		{ throw NotImplementedException ("PdfOperator::insert_after ()"); };

	/**
	 * Remove an operator from the composite interface.
	 *
	 * REMARK: This won't delete it from the Iterator list
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
	template<typename ITER>
	static ITER getIterator (boost::shared_ptr<PdfOperator> op) 
		{ return ITER (ListItem (op)); }
	static Iterator getIterator (boost::shared_ptr<PdfOperator> op) 
		{ return Iterator (ListItem (op)); }

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
		{ setNext (ListItem (nxt));	}
	void setPrev (boost::shared_ptr<PdfOperator> prv) 
		{ setPrev (ListItem (prv)); }

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

protected:
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
	void setContentStream (CContentStream& cs)
		{contentstream = &cs;}
	
	
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
	virtual void insert_after (const boost::shared_ptr<PdfOperator> oper, boost::shared_ptr<PdfOperator> newOper);
	
	//
	// PdfOperator interface
	//
public:
	virtual size_t getParametersCount () const {return 0;};
	virtual void getParameters (Operands&) const {};
	virtual void getStringRepresentation (std::string& str) const;
	virtual void getOperatorName (std::string& first) const = 0;

	//
	// Destructor
	//
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
	// End operator is added to composite as normal operator so just prepand start operator
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
// Helper funcions
//==========================================================

/** Is composite. */
inline bool 
isComposite (const PdfOperator* oper)
{
	const CompositePdfOperator* compo = dynamic_cast<const CompositePdfOperator*> (oper);
	return (NULL == compo) ? false : true;
}

inline bool 
isComposite (PdfOperator::Iterator it)
	{ return isComposite (it.getCurrent().get()); }

inline bool 
isComposite (boost::shared_ptr<PdfOperator> oper)
	{ return isComposite (oper.get()); }



/**
 * Find composite operator into which an operator belongs.
 *
 * @param begin Start iterator.
 * @param it Object we want to find the composite in which it resides.
 *
 * @return Composite operator.
 */
boost::shared_ptr<CompositePdfOperator> findCompositeOfPdfOperator (PdfOperator::Iterator begin, PdfOperator::Iterator it);

/**
 * Get last operator if a composite.
 *
 * @param oper Operator.
 */
boost::shared_ptr<PdfOperator> getLastOperator (boost::shared_ptr<PdfOperator> oper);

/**
 * Get last operator if a composite.
 *
 * @param it Iterator.
 */
inline boost::shared_ptr<PdfOperator> getLastOperator (PdfOperator::Iterator it)
	{ return getLastOperator (it.getCurrent()); }


//==========================================================
// PdfOperator iterators
//==========================================================

/**
 * Text operator iterator.
 *
 * Constructed from an arbitrary operator, but it will always start from a valid
 * text operator. This is done in the constructor.
 */
struct TextOperatorIterator: public PdfOperator::Iterator
{
	/** Number of accepted names. */
	static const size_t NAME_COUNT = 4;

	//
	// Constructor
	//
	TextOperatorIterator (ListItem oper) : PdfOperator::Iterator (oper)
	{
		// Get to the first valid text operator
		while (!_cur.expired() && !validItem ())
			this->next();
	}
	//
	// Template method interface
	//
	virtual bool 
	validItem () const
	{
		std::string name;
		_cur.lock()->getOperatorName (name);

		for (size_t i = 0; i < NAME_COUNT; ++i)
			if (name == accepted_opers[i])
				return true;
		
		return false;
	}

private:
	static const std::string accepted_opers [NAME_COUNT];
};

/**
 * Inline image iterator.
 *
 * Constructed from an arbitrary operator, but it will always start from a valid
 * inline image operator. This is done in the constructor.
 */
struct InlineImageOperatorIterator: public PdfOperator::Iterator
{
	//
	// Constructor
	//
	InlineImageOperatorIterator (ListItem oper) : PdfOperator::Iterator (oper)
	{
		// Get to the first valid text operator
		while (!_cur.expired() && !validItem ())
			this->next();
	}
	//
	// Template method interface
	//
	virtual bool 
	validItem () const
	{
		std::string name;
		_cur.lock()->getOperatorName (name);

		if (name == accepted_opers)
			return true;
		
		return false;
	}

private:
	static const std::string accepted_opers;
};

//==========================================================
} // namespace pdfobjects
//==========================================================


#endif // _PDFOPERATORS_H_
