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
 * This is a special implementation, a hybrid between implementation of stl containers and GoF
 * example).
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
 * design pattern is used here. Decorator and composite together form a very
 * easy to use way how to change these operators. (e.g. changing font means 
 * wrapping the text operator to a composite, which includes also the font
 * change operator.) 
 *
 * This approach allows us to represent operators in a human readible way
 * (tree like structure) and to have the power of double linked list with
 * iterators.
 *
 * We can not simply represent these two queues in one. If we want to have all
 * info only in interator queue items can not be shares pointers because of the
 * cyclic shared pointer problem (no item would get ever deallocated). If we
 * want to have all the info in tree like queue, we would have to know the
 * parents and it would not be easy to write an iterator and it is not a clean
 * straight forward solution. Another possibility is to have the queue outside
 * but (e.g. in content stream) but then we would not be able to soimply create
 * composites.
 *
 * Another advantage of the composite designe pattern is the single unified interface.
 * In most cases we do not need to distinguish between a composite and a leaf.
 *
 * Iterators are crucial when selecting operators. 
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
	 * @param Operator to be erased.
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
	 * @param p Children container.
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
	 *
	 * @return Content stream that this pdfoperator belongs to.
	 */
	CContentStream* getContentStream () const
		{return contentstream;}

	//
	// BBox
	//
private:
	/** Bounding box of this operator. */
	Rectangle bbox;

public:
	/** 
	 * Set bounding box.
	 *
	 * @param bbox Bounding box.
	 */
	void setBBox (const Rectangle& rc)
		{ bbox = rc; };

	/**
	 * Get bounding box of this operator.
	 *
	 * @return Bounding box.
	 */
	Rectangle getBBox () const
		{ assert (COORDINATE_INVALID != bbox.xleft); return bbox; };
	
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
 * it as one. This object is special it is other than
 * every other operator according to the pdf specification.
 *
 * This is an implementation of Composite design pattern where leaves and
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
// PdfOperator iterators
//==========================================================

/** 
 * Iterator types. Needed when constructing specific iterators using templates.
 */
typedef enum
{
	// General iterators
		itTextIterator, itFontIterator, itInlineImageIterator, itNonStrokingIterator, itStrokingIterator, 
		itChangeableIterator, itGraphicalIterator
} IteratorType;


/**
 * Generic iterator that accepts set of operators.
 *
 * This is a very generic class iterates only over specified operators. Use this
 * class when a special iterator is desired.
 *
 * \see Iterator, RejectingPdfOperatorIterator
 */
template<int _NAME_COUNT, IteratorType T>
struct AcceptingPdfOperatorIterator: public PdfOperator::Iterator
{
	/** Number of accepted names. */
	static const size_t namecount = _NAME_COUNT;

	//
	// Constructor
	//
	AcceptingPdfOperatorIterator (ListItem oper, bool forwarddir = true) : PdfOperator::Iterator (oper)
	{
		if (forwarddir)
		{
			// Get to the first valid text operator
			while (!isEnd() && !validItem())
				this->next();
		}else
		{
			// Get to the first valid text operator in the backward direction
			while (!isBegin() && !validItem())
				this->prev();
		}
	}

	//
	// Template method interface
	//
	virtual bool 
	validItem () const
	{
		std::string name;
		_cur.lock()->getOperatorName (name);

		for (size_t i = 0; i < namecount; ++i)
			if (name == accepted_opers[i])
				return true;
		
		return false;
	}

private:
	static const std::string accepted_opers [namecount];
};


/**
 * Generic iterator that rejects set of operators.
 * 
 * This is a very generic class iterates over all but specified operators. Use this
 * class when a special iterator is desired.
 *
 * \see Iterator, AcceptingPdfOperatorIterator
 */
template<int _NAME_COUNT, IteratorType T>
struct RejectingPdfOperatorIterator: public PdfOperator::Iterator
{
	/** Number of accepted names. */
	static const size_t namecount = _NAME_COUNT;

	//
	// Constructor
	//
	RejectingPdfOperatorIterator (ListItem oper, bool forwarddir = true) : PdfOperator::Iterator (oper)
	{
		if (forwarddir)
		{
			// Get to the first valid text operator
			while (!isEnd() && !validItem())
				this->next();
		}else
		{
			// Get to the first valid text operator in the backward direction
			while (!isBegin() && !validItem())
				this->prev();
		}
	}

	//
	// Template method interface
	//
	virtual bool 
	validItem () const
	{
		std::string name;
		_cur.lock()->getOperatorName (name);

		for (size_t i = 0; i < namecount; ++i)
			if (name == rejected_opers[i])
				return false;
		
		return true;
	}

private:
	static const std::string rejected_opers [namecount];
};


/**
 * Text operator iterator.
 *
 * Constructed from an arbitrary operator, but it will always start from a valid
 * text operator. This is done in the constructor.
 */
typedef AcceptingPdfOperatorIterator<4, itTextIterator> TextOperatorIterator;


/**
 * Font operator iterator.
 *
 * This iterator traverses only through operators that alter graphical font state.
 */
typedef struct AcceptingPdfOperatorIterator<4, itFontIterator> FontOperatorIterator;


/**
 * Inline image iterator.
 *
 * Constructed from an arbitrary operator, but it will always start from a valid
 * inline image operator. This is done in the constructor.
 */
typedef struct AcceptingPdfOperatorIterator<1, itInlineImageIterator> InlineImageOperatorIterator;


/**
 * Changeable operator iterator.
 *
 * Constructed from an arbitrary operator, but it will always start from a valid
 * common operator. This is done in the constructor.
 *
 * This iterator excludes operators like q, Q etc.
 */
typedef struct RejectingPdfOperatorIterator<38, itChangeableIterator> ChangeableOperatorIterator;


/**
 * "Non stroking" operator iterator.
 * 
 * This iterator traverses only operators that are connected with non stroking
 * operations. 
 *
 * REMARK: See pdf specification for details.
 *
 * \see StrokingOperatorIterator
 */
typedef struct AcceptingPdfOperatorIterator<4, itNonStrokingIterator> NonStrokingOperatorIterator;


/**
 * "Stroking" operator iterator. 
 *
 * This iterator traverses only operators that are connected with stroking
 * operations. 
 *
 * REMARK: See pdf specification for details.
 *
 * \see NonStrokingOperatorIterator
 */
typedef struct AcceptingPdfOperatorIterator<4, itStrokingIterator> StrokingOperatorIterator;

/**
 * Graphical operator iterator.
 *
 * Constructed from an arbitrary operator, but it will always start from a valid
 * common operator. This is done in the constructor.
 *
 * This iterator iterates over all graphical operators on a page this includes
 * lines, rectangles, paths.
 */
typedef struct AcceptingPdfOperatorIterator<24, itGraphicalIterator> GraphicalOperatorIterator;

//==========================================================
// Helper funcions
//==========================================================

/** Is an operator a composite. */
inline bool 
isComposite (const PdfOperator* oper)
{
	const CompositePdfOperator* compo = dynamic_cast<const CompositePdfOperator*> (oper);
	return (NULL == compo) ? false : true;
}

/** Is an operator a composite. */
inline bool 
isComposite (PdfOperator::Iterator it)
	{ return isComposite (it.getCurrent().get()); }

/** Is an operator a composite. */
inline bool 
isComposite (boost::shared_ptr<PdfOperator> oper)
	{ return isComposite (oper.get()); }



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


	
//==========================================================
} // namespace pdfobjects
//==========================================================


#endif // _PDFOPERATORS_H_
