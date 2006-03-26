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
#include "iproperty.h"


//==========================================================
namespace pdfobjects {
//==========================================================

		
namespace mpl = boost::mpl;


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
 * Generic function to check if operands are really the same type
 * as we want.
 *
 * It is evaluated till Position is 0. Partial template specialization
 * is used here.
 */
template<typename TLIST, typename ITERATOR, int Position>
struct CheckOperatorTypes
{
	bool operator() (ITERATOR it, ITERATOR end)
	{
		// Check if we are still in the array
		if (it == end)
			throw OutOfRange ();

		// Store iterator
		ITERATOR itOld = it;

		// Increment the iterator
		++it;
		
		// Check lower levels
		struct CheckOperatorTypes<TLIST, ITERATOR, Position - 1> check;
		if (true != check (it, end))
			return false;

		// Check the types at this level
		if (mpl::at<TLIST, mpl::long_<Position> >::type::value == (*itOld)->getType ())
			return true;
		else
			return false;
	}
};
//
// Partial specialization of the function
//
template<typename TLIST, typename ITERATOR>
struct CheckOperatorTypes<TLIST, ITERATOR, 0>
{
	bool operator() (ITERATOR it, ITERATOR end)
	{
		// Check if we are still in the array
		if (it != end)
		{
			// Check the types
			if (mpl::at<TLIST, mpl::long_<0> >::type::value == (*it)->getType ())
				return true;
			else
				return false;
		}else
			throw OutOfRange ();
	}
};


/**
 * A very advanced c++ template technique is used here to construct SimpleGenericOperator.
 *
 * It is called metaprogramming (special Typelists), firstly designed in Loki (perhaps). It is pure compile
 * time business.
 *
 * Almost all simple operators will be constructed by specifying types of operands and the
 * text representation of the operator. 
 *
 * <cref exception="MalformedFormatExeption"> Thrown when the operands do not match the specification.
 * 
 */
template<typename TYPES, const char* OPSTRING>
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
	SimpleGenericOperator (Operands& opers) : opText (OPSTRING)
	{
		printDbg (debug::DBG_DBG, "Opeartor [" << OPSTRING << "] Operator size: " << opers.size());

		//
		// We will traverse from back and compare the type of the template parameter at appropriate position 
		// with the type of last operand
		//
		Operands::reverse_iterator first = opers.rbegin ();
		Operands::reverse_iterator end = opers.rend ();
		// Compare it to what we expect
		struct CheckOperatorTypes<TYPES, Operands::reverse_iterator, mpl::size<TYPES>::value - 1> check;
		if (!check (first, end))
		{
			throw MalformedFormatExeption ("Content stream operator has incorrect operands.");
		}

		//
		// Store the operands and remove it from the stack
		//
		for (int i = 0; i < mpl::size<TYPES>::value; ++i)
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
	{
		assert ((int)mpl::size<TYPES>::value == operands.size());
		return mpl::size<TYPES>::value;
	};

	virtual void getParameters (IPContainer& container) const
		{ copy (operands.begin(), operands.end (), back_inserter(container) ); };

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
	UnknownPdfOperator (Operands& opers, const char* opTxt);
		
	//
	// PdfOperator interface
	//
	virtual size_t getParametersCount () const;
	virtual void getParameters (IPContainer& container) const;
	virtual void getStringRepresentation (std::string& str) const;
	
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
// Specialized classes representing operators
//==========================================================


// {"\"",  3, {tchkNum,    tchkNum,    tchkString},
  
extern const char OPER_MOVETONEXTLINE []; 
typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>,OPER_MOVETONEXTLINE> 	COperMoveShowText;

extern const char OPER_FILLSTROKE []; 
typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>,OPER_FILLSTROKE> 		COperFillStroke;

extern const char OPER_EOFILLSTROKE []; 
typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>,OPER_EOFILLSTROKE> 		COperEoFillStroke;


  /*{"B",   0, {tchkNone},
          &Gfx::opFillStroke},
  {"B*",  0, {tchkNone},
          &Gfx::opEOFillStroke},
  {"BDC", 2, {tchkName,   tchkProps},
          &Gfx::opBeginMarkedContent},
  {"BI",  0, {tchkNone},
          &Gfx::opBeginImage},
  {"BMC", 1, {tchkName},
          &Gfx::opBeginMarkedContent},
  {"BT",  0, {tchkNone},
          &Gfx::opBeginText},
  {"BX",  0, {tchkNone},
          &Gfx::opBeginIgnoreUndef},
  {"CS",  1, {tchkName},
          &Gfx::opSetStrokeColorSpace},
  {"DP",  2, {tchkName,   tchkProps},
          &Gfx::opMarkPoint},
  {"Do",  1, {tchkName},
          &Gfx::opXObject},
  {"EI",  0, {tchkNone},
          &Gfx::opEndImage},
  {"EMC", 0, {tchkNone},
          &Gfx::opEndMarkedContent},
  {"ET",  0, {tchkNone},
          &Gfx::opEndText},
  {"EX",  0, {tchkNone},
          &Gfx::opEndIgnoreUndef},
  {"F",   0, {tchkNone},
          &Gfx::opFill},
  {"G",   1, {tchkNum},
          &Gfx::opSetStrokeGray},
  {"ID",  0, {tchkNone},
          &Gfx::opImageData},
  {"J",   1, {tchkInt},
          &Gfx::opSetLineCap},
  {"K",   4, {tchkNum,    tchkNum,    tchkNum,    tchkNum},
          &Gfx::opSetStrokeCMYKColor},
  {"M",   1, {tchkNum},
          &Gfx::opSetMiterLimit},
  {"MP",  1, {tchkName},
          &Gfx::opMarkPoint},
  {"Q",   0, {tchkNone},
          &Gfx::opRestore},
  {"RG",  3, {tchkNum,    tchkNum,    tchkNum},
          &Gfx::opSetStrokeRGBColor},
  {"S",   0, {tchkNone},
          &Gfx::opStroke},
  {"SC",  -4, {tchkNum,   tchkNum,    tchkNum,    tchkNum},
          &Gfx::opSetStrokeColor},
  {"SCN", -5, {tchkSCN,   tchkSCN,    tchkSCN,    tchkSCN,
	       tchkSCN},
          &Gfx::opSetStrokeColorN},
  {"T*",  0, {tchkNone},
          &Gfx::opTextNextLine},
  {"TD",  2, {tchkNum,    tchkNum},
          &Gfx::opTextMoveSet},
  {"TJ",  1, {tchkArray},
          &Gfx::opShowSpaceText},
  {"TL",  1, {tchkNum},
          &Gfx::opSetTextLeading},
  {"Tc",  1, {tchkNum},
          &Gfx::opSetCharSpacing},
  {"Td",  2, {tchkNum,    tchkNum},
          &Gfx::opTextMove},
  {"Tf",  2, {tchkName,   tchkNum},
          &Gfx::opSetFont},
  {"Tj",  1, {tchkString},
          &Gfx::opShowText},
  {"Tm",  6, {tchkNum,    tchkNum,    tchkNum,    tchkNum,
	      tchkNum,    tchkNum},
          &Gfx::opSetTextMatrix},
  {"Tr",  1, {tchkInt},
          &Gfx::opSetTextRender},
  {"Ts",  1, {tchkNum},
          &Gfx::opSetTextRise},
  {"Tw",  1, {tchkNum},
          &Gfx::opSetWordSpacing},
  {"Tz",  1, {tchkNum},
          &Gfx::opSetHorizScaling},
  {"W",   0, {tchkNone},
          &Gfx::opClip},
  {"W*",  0, {tchkNone},
          &Gfx::opEOClip},
  {"b",   0, {tchkNone},
          &Gfx::opCloseFillStroke},
  {"b*",  0, {tchkNone},
          &Gfx::opCloseEOFillStroke},
  {"c",   6, {tchkNum,    tchkNum,    tchkNum,    tchkNum,
	      tchkNum,    tchkNum},
          &Gfx::opCurveTo},
  {"cm",  6, {tchkNum,    tchkNum,    tchkNum,    tchkNum,
	      tchkNum,    tchkNum},
          &Gfx::opConcat},
  {"cs",  1, {tchkName},
          &Gfx::opSetFillColorSpace},
  {"d",   2, {tchkArray,  tchkNum},
          &Gfx::opSetDash},
  {"d0",  2, {tchkNum,    tchkNum},
          &Gfx::opSetCharWidth},
  {"d1",  6, {tchkNum,    tchkNum,    tchkNum,    tchkNum,
	      tchkNum,    tchkNum},
          &Gfx::opSetCacheDevice},
  {"f",   0, {tchkNone},
          &Gfx::opFill},
  {"f*",  0, {tchkNone},
          &Gfx::opEOFill},
  {"g",   1, {tchkNum},
          &Gfx::opSetFillGray},
  {"gs",  1, {tchkName},
          &Gfx::opSetExtGState},
  {"h",   0, {tchkNone},
          &Gfx::opClosePath},
  {"i",   1, {tchkNum},
          &Gfx::opSetFlat},
  {"j",   1, {tchkInt},
          &Gfx::opSetLineJoin},
  {"k",   4, {tchkNum,    tchkNum,    tchkNum,    tchkNum},
          &Gfx::opSetFillCMYKColor},
  {"l",   2, {tchkNum,    tchkNum},
          &Gfx::opLineTo},
  {"m",   2, {tchkNum,    tchkNum},
          &Gfx::opMoveTo},
  {"n",   0, {tchkNone},
          &Gfx::opEndPath},
  {"q",   0, {tchkNone},
          &Gfx::opSave},
  {"re",  4, {tchkNum,    tchkNum,    tchkNum,    tchkNum},
          &Gfx::opRectangle},
  {"rg",  3, {tchkNum,    tchkNum,    tchkNum},
          &Gfx::opSetFillRGBColor},
  {"ri",  1, {tchkName},
          &Gfx::opSetRenderingIntent},
  {"s",   0, {tchkNone},
          &Gfx::opCloseStroke},
  {"sc",  -4, {tchkNum,   tchkNum,    tchkNum,    tchkNum},
          &Gfx::opSetFillColor},
  {"scn", -5, {tchkSCN,   tchkSCN,    tchkSCN,    tchkSCN,
	       tchkSCN},
          &Gfx::opSetFillColorN},
  {"sh",  1, {tchkName},
          &Gfx::opShFill},
  {"v",   4, {tchkNum,    tchkNum,    tchkNum,    tchkNum},
          &Gfx::opCurveTo1},
  {"w",   1, {tchkNum},
          &Gfx::opSetLineWidth},
  {"y",   4, {tchkNum,    tchkNum,    tchkNum,    tchkNum},
          &Gfx::opCurveTo2},
*/







//==========================================================
}
//==========================================================


#endif // _PDFOPERATORS_H_
