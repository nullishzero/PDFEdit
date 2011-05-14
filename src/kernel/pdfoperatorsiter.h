/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006-2009  PDFedit team: Michal Hocko,
 *                                        Jozef Misutka,
 *                                        Martin Petricek
 *                   Former team members: Miroslav Jahoda
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (in doc/LICENSE.GPL); if not, write to the 
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 * MA  02111-1307  USA
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit
 */
// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80

#ifndef _PDFOPERATORSITER_H_
#define _PDFOPERATORSITER_H_

// static includes
#include "kernel/static.h"

// iterator
#include "utils/iterator.h"
#include "kernel/pdfoperators.h"
#include "kernel/contentschangetag.h"


//==========================================================
namespace pdfobjects {
//==========================================================

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
 * This is a very generic class which iterates only over specified operators. Use this
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
 * This is a very generic class which iterates over all but specified operators. Use this
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
 * Constructed from an arbitrary operator, but it will always start from a valid
 * font operator. This is done in the constructor.
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
 * graphical operator. This is done in the constructor.
 *
 * This iterator iterates over all graphical operators on a page this includes
 * lines, rectangles, paths.
 */
typedef struct AcceptingPdfOperatorIterator<25, itGraphicalIterator> GraphicalOperatorIterator;



/**
 * Decides if the operator is from content stream of our change or not.
 */
struct ChangePdfOperatorIterator: public PdfOperator::Iterator
{
	/** If not found in MAX_OPS_TO_CHECK operators, it is not our change. */
	static const size_t MAX_OPS_TO_CHECK = 5;
	//
	// Constructor
	//
	ChangePdfOperatorIterator (ListItem oper) : PdfOperator::Iterator (oper)
	{
			// Get to the first valid text operator
			size_t i = 0;
			for (; !isEnd() && !validItem() && i < MAX_OPS_TO_CHECK; ++i)
				this->next();
			
			if (MAX_OPS_TO_CHECK == i)
			{
				// Go to the last one
				while (!isEnd())
					this->next();
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

		if (name == ContentsChangeTag::CHANGE_TAG_NAME)
			return true;
		
		return false;
	}
};



//==========================================================
} // namespace pdfobjects
//==========================================================


#endif // _PDFOPERATORSITER_H_
