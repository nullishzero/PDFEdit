// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80
/*
 * =====================================================================================
 *        Filename:  pdfoperatorsiter.h
 *         Created:  06/11/2006 01:54:03 PM CET
 *          Author:  jmisutka ()
 * =====================================================================================
 */

#ifndef _PDFOPERATORSITER_H_
#define _PDFOPERATORSITER_H_

// static includes
#include "static.h"

// iterator
#include "utils/iterator.h"


//==========================================================
namespace pdfobjects {
//==========================================================

//==========================================================
// PdfOperator iterators
//==========================================================

//
// Forward declarations
//
class PdfOperator;

//
//
//
typedef iterator::SharedDoubleLinkedListIterator<PdfOperator> PdfOperatorIterator;
	
	
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
struct AcceptingPdfOperatorIterator: public PdfOperatorIterator
{
	/** Number of accepted names. */
	static const size_t namecount = _NAME_COUNT;

	//
	// Constructor
	//
	AcceptingPdfOperatorIterator (ListItem oper, bool forwarddir = true) : PdfOperatorIterator (oper)
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
struct RejectingPdfOperatorIterator: public PdfOperatorIterator
{
	/** Number of accepted names. */
	static const size_t namecount = _NAME_COUNT;

	//
	// Constructor
	//
	RejectingPdfOperatorIterator (ListItem oper, bool forwarddir = true) : PdfOperatorIterator (oper)
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
} // namespace pdfobjects
//==========================================================


#endif // _PDFOPERATORSITER_H_
