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
 * This is a special implementation, a hybrid between implementation of stl containers and GoF
 * example.
 *
 * Content stream consists of operators and their operands. Operators can be
 * composites. Each operator alters graphical state in a predefined way.
 * Operators have to be processed sequentially.
 * 
 * This is the building  block of almost all visible objects on a page. Every
 * operation on a text, graphics etc.. means adding/deleting/changing operators. 
 *
 * Operators themselves build a double linked list queue. We use iterators to
 * iterate over them sequentially.
 *
 * They also form a tree like structure and that is the reason why Composite
 * design pattern is used here. Decorator and composite together allow a very
 * easy way to change these operators. (e.g. changing font means 
 * wrapping the text operator to a composite, which includes also the font
 * change operator.) 
 *
 * This approach allows us to represent operators in a human readable way
 * (tree like structure) and to have the power of double linked list with
 * iterators.
 *
 * We can not simply represent these two queues in one. If we want to have all
 * info only in interator queue, items can not be shared pointers because of the
 * cyclic shared pointer problem (no item would get ever deallocated). If we
 * want to have all the info in tree like queue, we would have to know the
 * parents and it would not be easy to write an iterator. It would not be a clean
 * straight forward solution. Another possibility is to have the queue outside
 * (e.g. in content stream) but then we would not be able to simply create
 * composites.
 *
 * Another advantage of the composite design pattern is the single unified interface.
 * In most cases we do not need to distinguish between a composite and a leaf.
 *
 * Iterators are crucial when selecting operators. 
 */
class PdfOperator
{	
		
public:
	typedef std::deque<boost::shared_ptr<IProperty> > 	Operands;
	typedef iterator::SharedDoubleLinkedListIterator<PdfOperator> Iterator;
	typedef Iterator::ListItem							ListItem;
	typedef std::list<boost::shared_ptr<PdfOperator> > 	PdfOperators;
	typedef Rectangle									BBox;

	// iterator has to be a friend
	friend class iterator::SharedDoubleLinkedListIterator<PdfOperator>;
	friend class CContentStream;

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
	PdfOperator () : contentstream (NULL) {};


	//
	// Destructor
	//
public:

	/**
	 * Destructor.
	 */
	virtual ~PdfOperator ()	{};

	
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
	 * @param first String that will hold operator name.
	 */
	virtual void getOperatorName (std::string& first) const = 0;

	
	//
	// Composite interface
	//
public:
	
	/**
	 * Add an operator to the end of composite. One problem is when the
	 * composite is empty, we have to specify prev variable because we can not
	 * get shared pointer from this.
	 * 
	 * @param oper Operator to be added.
	 * @param prev Operator, after which we should place the added one in
	 * iterator list. If not specified, it is assumed that children are not
	 * empty and that the last item is not a composite.
	 */
	virtual void push_back ( __attribute__((unused)) const boost::shared_ptr<PdfOperator> oper, 
							__attribute__((unused)) boost::shared_ptr<PdfOperator> prev = boost::shared_ptr<PdfOperator> ())
		{ throw NotImplementedException ("PdfOperator::push_back ()"); };

protected:
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
	 * REMARK: This will not remove it from the Iterator list.
	 *
	 * @param op to be erased.
	 */
	virtual void remove (boost::shared_ptr<PdfOperator>)
		{ throw NotImplementedException ("PdfOperator::remove ()"); };

protected:	
	/**
	 * Put behind.
	 * 
	 * REMARK: We can't simply make putBehind a method, because we have to
	 * know the smart pointer behind the object. Member variable "this" is not
	 * sufficient.
	 *
	 * @param behindWhich Behind which elemnt.
	 * @param which 		Which element.
	 */
	static void putBehind (	boost::shared_ptr<PdfOperator> behindWhich, 
							boost::shared_ptr<PdfOperator> which)
	{
		assert (behindWhich);
		assert (which);

		if (behindWhich && which)
		{
			if (!behindWhich->next.expired ())
			{ // we are not at the end
			
				assert (behindWhich->next.lock());
				which->setNext (behindWhich->next);
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

	//
	// Composite interface
	//
public:
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
	 * @param opers Children container.
	 */
	virtual void getChildren (PdfOperators& opers) const
		{ opers.clear(); };	

	
	//
	// Iterator interface
	//

private:
	ListItem next;	/**< Reference to previous operator in iterator list. */
	ListItem prev;	/**< Reference to next operator in iterator list. */

public:

	/**
	 * Get iterator.
	 * 
	 * REMARK: We can't simply make getIterator a method, because we have to
	 * know the smart pointer behind the object. Member variable "this" is not
	 * sufficient.
	 *
	 * @param op Operator from which we want the iterator.
	 * @param forward Direction of traversing when looking for the first valid
	 * item.
	 * 
	 * @return Iterator.
	 */
	template<typename ITER>
	static ITER getIterator (boost::shared_ptr<PdfOperator> op, bool forward) 
		{ return ITER (ListItem (op),forward); }

	/** \copydoc getIterator. */
	template<typename ITER>
	static ITER getIterator (boost::shared_ptr<PdfOperator> op) 
		{ return ITER (ListItem (op)); }

	/** Create simple iterator. */
	static Iterator getIterator (boost::shared_ptr<PdfOperator> op) 
		{ return Iterator (ListItem (op)); }

public:	
	/**
	 * Set next item.
	 *
	 * @param nxt Next item in iterator list.
	 */
	void setNext (boost::shared_ptr<PdfOperator> nxt) 
		{ setNext (ListItem (nxt));	}
	/**
	 * Set previous item.
	 *
	 * @param prv Previous item in iterator list.
	 */
	void setPrev (boost::shared_ptr<PdfOperator> prv) 
		{ setPrev (ListItem (prv)); }

	/** \copydoc setNext */
	void setNext (ListItem nxt) 
	{ 
		if (!next.expired ())
			utilsPrintDbg (debug::DBG_DBG, "Changing valid next variable.");

		next = nxt;
	};
	
	/** \copydoc setPrev */
	void setPrev (ListItem prv) 
	{ 
		if (!prev.expired ()) 
			utilsPrintDbg (debug::DBG_DBG, "Changing valid prev variable.");
		
		prev = prv; 
	};

protected:
	/**
	 * Get previous item in the iterator list that is implemented by PdfOperator.
	 *
	 * @return Previous item.
	 */
	ListItem _next () const {return next;};
	
	/**
	 * Get next item in the iterator list that is implemented by PdfOperator.
	 *
	 * @return Next item.
	 */
	ListItem _prev () const {return prev;};

	//
	// Clone interface
	//
public:
	/**
	 * Clone this object.
	 *
	 * @return New clone.
	 */
	virtual boost::shared_ptr<PdfOperator> clone () = 0;

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
	 * @return Content stream that this pdfoperator belongs to.
	 */
	boost::shared_ptr<CContentStream> getContentStream () const;

	//
	// BBox
	//
private:
	/** Bounding box of this operator. */
	BBox bbox;

public:
	/** 
	 * Set bounding box.
	 *
	 * @param rc Bounding box.
	 */
	void setBBox (const BBox& rc)
		{ bbox = rc; };

	/**
	 * Get bounding box of this operator.
	 *
	 * @return Bounding box.
	 */
	BBox getBBox () const
		{ assert (BBox::isInitialized(bbox)); return bbox; };
	
};



//==========================================================
// CompositePdfOperator
//==========================================================

/**
 * Composite object.
 *
 * This is an implementation of Composite design pattern where leaves and
 * composites share the same interface.
 *
 * \see PdfOperator
 */
class CompositePdfOperator : public PdfOperator
{
protected:

	/** Child operator, where all calls are redirected. */
	PdfOperators children;

protected:
	/**
	 * Constructor.
	 */
	CompositePdfOperator () {};

	//
	// Composite interface
	//
public:
	virtual size_t getChildrenCount () const {return children.size ();};	
	virtual void push_back (const boost::shared_ptr<PdfOperator> oper, 
							boost::shared_ptr<PdfOperator> prev = boost::shared_ptr<PdfOperator> ());
	virtual void getChildren (PdfOperators& container) const;
protected:
	virtual void remove (boost::shared_ptr<PdfOperator> op);
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
	// Clone interface
	//
protected:
	virtual boost::shared_ptr<PdfOperator> clone () = 0;

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
 * Almost all simple (non composite) operators will be constructed by specifying number of operands, operands and the
 * text representation of the operator.
 *
 * This is an implementation of Composite design pattern where leaves and
 * composites share the same interface.
 *
 * \see PdfOperator

 */
class SimpleGenericOperator : public PdfOperator
{
private:
	/** Operands. */
	Operands operands;

	/** Text representing the operator. */
	const std::string opText;
	
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

	/** 
	 * Constructor. 
	 */
	SimpleGenericOperator (const std::string& opTxt, Operands& opers): opText (opTxt)
	{
		utilsPrintDbg (debug::DBG_DBG, opTxt);
		//
		// Store the operands and remove it from opers
		//
		while (!opers.empty())
		{
			// Store the last element of input parameter
			operands.push_front ( opers.back() );
			// Remove the element from input parameter
			opers.pop_back ();
		}
	}


	
	//
	// PdfOperator interface
	//
public:

	virtual size_t getParametersCount () const
		{ return operands.size (); };

	virtual void getParameters (Operands& container) const
		{ copy (operands.begin(), operands.end(), back_inserter(container)); };

	virtual void getOperatorName (std::string& first) const
		{ first = opText;}
	
	virtual void getStringRepresentation (std::string& str) const;

	//
	// Clone interface
	//
protected:
	virtual boost::shared_ptr<PdfOperator> clone ();

};



//==========================================================
// Concrete implementations of CompositePdfOperator
//==========================================================


/**
 * Unknown composite operator which is constructed from a begin and end tag.
 *
 * This is an implementation of Composite design pattern where leaves and
 * composites share the same interface.
 * 
 * \see PdfOperator, CompositePdfOperator
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

	//
	// Clone interface
	//
protected:
	virtual boost::shared_ptr<PdfOperator> clone ();


};


//
// Forward declaration
//
class CInlineImage;

/**
 * Inline image pdf operator. This is not a real composite but we can represent
 * it as one. This object is special. It is different than
 * every other pdf operator.
 *
 * This is an implementation of the Composite design pattern where leaves and
 * composites share the same interface.
 * 
 * \see PdfOperator, CompositePdfOperator
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
	 * @param im_ Stream representing inline image.
	 */
	InlineImageCompositePdfOperator (const char* opBegin_, const char* opEnd_, boost::shared_ptr<CInlineImage> im_);

	
	//
	// PdfOperator interface
	//
public:
	virtual size_t getParametersCount () const {return 1;};
	virtual void getParameters (Operands& opers) const;
	virtual void getStringRepresentation (std::string& str) const;
	virtual void getOperatorName (std::string& first) const {first = opBegin;};

	//
	// Clone interface
	//
protected:
	virtual boost::shared_ptr<PdfOperator> clone ();

};




//==========================================================
// Helper funcions - general
//==========================================================

/** Is an operator a composite. */
inline bool 
isCompositeOp (const PdfOperator* oper)
{
	const CompositePdfOperator* compo = dynamic_cast<const CompositePdfOperator*> (oper);
	return (NULL == compo) ? false : true;
}

/** Is an operator a composite. */
inline bool 
isCompositeOp (PdfOperator::Iterator it)
	{ return isCompositeOp (it.getCurrent().get()); }

/** Is an operator a composite. */
inline bool 
isCompositeOp (boost::shared_ptr<PdfOperator> oper)
	{ return isCompositeOp (oper.get()); }


/** Is an operator an inline image. */
inline bool 
isInlineImageOp (const PdfOperator* oper)
{
	const InlineImageCompositePdfOperator* img = dynamic_cast<const InlineImageCompositePdfOperator*> (oper);
	return (NULL == img) ? false : true;
}
/** Is an operator an inline image. */
inline bool 
isInlineImageOp (boost::shared_ptr<PdfOperator> oper)
	{ return isInlineImageOp (oper.get()); }


/**
 * Find composite operator into which an operator belongs.
 *
 * @param begin Start iterator.
 * @param oper Object we want to find the composite in which it resides.
 *
 * @return Composite operator.
 */
boost::shared_ptr<CompositePdfOperator> findCompositeOfPdfOperator (PdfOperator::Iterator begin, 
																	boost::shared_ptr<PdfOperator> oper);

/**
 * Get last operator if a composite.
 *
 * @param oper Operator.
 *
 * @return Last operator of a composite.
 */
boost::shared_ptr<PdfOperator> getLastOperator (boost::shared_ptr<PdfOperator> oper);

/**
 * Get last operator if a composite.
 *
 * @param it Iterator.
 * 
 * @return Iterator over last operator of a composite.
 */
inline boost::shared_ptr<PdfOperator> getLastOperator (PdfOperator::Iterator it)
	{ return getLastOperator (it.getCurrent()); }


/**
 * Create our own tag indicating time when the change occured. This enables us
 * to sort our changes according to time.
 *
 * @return Operator that identifies our changes
 */
boost::shared_ptr<PdfOperator> createChangeTag ();

/**
 * Get change tag name.
 */
std::string getChangeTagName ();

/**
 * Get change tag id. It is the first object in new stream.
 */
std::string getChangeTagId ();

/**
 * Get change tag time.
 */
time_t getChangeTagTime (boost::shared_ptr<PdfOperator> op);


//
// Specific operator
//
inline bool
isPdfOp (const PdfOperator& op, const std::string& opn)
{
	std::string tmp;
	op.getOperatorName (tmp);
	return opn == tmp;
}
inline bool
isPdfOp (const PdfOperator& op, 
		 const std::string& opn1, 
		 const std::string& opn2)
{ 
	std::string tmp;
	op.getOperatorName (tmp);
	return (tmp == opn1) || (tmp == opn2); 
}
inline bool
isPdfOp (const PdfOperator& op, 
		 const std::string& opn1, 
		 const std::string& opn2, 
		 const std::string& opn3)
{
	std::string tmp;
	op.getOperatorName (tmp);
	return (tmp == opn1) || (tmp == opn2) || (tmp == opn3); 
}
inline bool
isPdfOp (const PdfOperator& op, 
		 const std::string& opn1, 
		 const std::string& opn2, 
		 const std::string& opn3, 
		 const std::string& opn4)
{
	std::string tmp;
	op.getOperatorName (tmp);
	return (tmp == opn1) || (tmp == opn2) || (tmp == opn3) || (tmp == opn4);
}

/** Is specific operator. */
template<typename T>
inline bool
isPdfOp (const T& op, const std::string& opn)
	{ return isPdfOp (*op, opn); }
template<typename T>
inline bool
isPdfOp (const T& op, const std::string& opn1, const std::string& opn2)
	{ return isPdfOp (*op, opn1, opn2); }
template<typename T>
inline bool
isPdfOp (const T& op, const std::string& opn1, const std::string& opn2, const std::string& opn3)
	{ return isPdfOp (*op, opn1, opn2, opn3); }
template<typename T>
inline bool
isPdfOp (const T& op, const std::string& opn1, const std::string& opn2, const std::string& opn3, const std::string& opn4)
	{ return isPdfOp (*op, opn1, opn2, opn3, opn4); }



//==========================================================
} // namespace pdfobjects
//==========================================================


#endif // _PDFOPERATORS_H_
