/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006, 2007, 2008  PDFedit team: Michal Hocko,
 *                                              Miroslav Jahoda,
 *                                              Jozef Misutka,
 *                                              Martin Petricek
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

#ifndef _PDFOPERATORS_H_
#define _PDFOPERATORS_H_

// static includes
#include "kernel/pdfoperatorsbase.h"

//==========================================================
namespace pdfobjects {
//==========================================================


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
	}

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
		{ return operands.size (); }

	virtual void getParameters (Operands& container) const
		{ copy (operands.begin(), operands.end(), back_inserter(container)); }

	virtual void getOperatorName (std::string& first) const
		{ first = opText;}
	
	virtual void getStringRepresentation (std::string& str) const;

	//
	// Clone interface
	//
protected:
	virtual boost::shared_ptr<PdfOperator> clone ();

	
	//
	// Observer interface
	//
private:
	boost::shared_ptr<observer::IObserver<IProperty> > _operandobserver;
public:
	void init_operands (boost::shared_ptr<observer::IObserver<IProperty> > observer,
						CPdf* pdf,
						IndiRef* rf)
	{ 
		// store observer
		_operandobserver = observer; 
		//
		for (Operands::iterator oper = operands.begin (); oper != operands.end (); ++oper)
		{
			if (hasValidPdf(*oper))
			{ // We do not support adding operators from another stream
				if ( ((*oper)->getPdf() != pdf) || !((*oper)->getIndiRef() == *rf) )
				{
					kernelPrintDbg (debug::DBG_CRIT, "Pdf or indiref do not match: want " << *rf <<  " op has" <<(*oper)->getIndiRef());
					throw CObjInvalidObject ();
				}
				
			}else
			{
				(*oper)->setPdf (pdf);
				(*oper)->setIndiRef (*rf);
				REGISTER_SHAREDPTR_OBSERVER((*oper), observer);
				(*oper)->lockChange ();
			}
		} // for
	}

	//
	// Destructor
	//
public:
	
	/**
	 * Destructor.
	 */
	virtual ~SimpleGenericOperator() 
	{
			// can happen when used as a temporary object
			if (0 < operands.size() && !(_operandobserver)) 
				return;
		for (Operands::iterator it = operands.begin(); it != operands.end(); ++it) {
			UNREGISTER_SHAREDPTR_OBSERVER ((*it), _operandobserver);
		}
	}

}; // class SimpleGenericOperator



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
	const char* _opBegin;
	/** Text representing the ending operator. */
	const char* _opEnd;

public:
	
	/** 
	 * Constructor. 
	 * Create it as a standalone object. Prev and Next are not valid.
	 *
	 * @param opBegin_ Start operator name text representation.
	 * @param opEnd_ End operator name text representation.
	 */
	UnknownCompositePdfOperator (const char* opBegin, const char* opEnd);

public:
	// End operator is added to composite as normal operator so just prepand start operator
	virtual void getStringRepresentation (std::string& str) const;
	virtual void getOperatorName (std::string& first) const {first = _opBegin;}

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
	const char* _opBegin;
	/** Text representing the ending operator. */
	const char* _opEnd;
	/** Stream representing inline image. */
	boost::shared_ptr<CInlineImage> _inlineimage;

public:
	
	/** 
	 * Constructor. 
	 * Create it as a standalone object. Prev and Next are not valid.
	 *
	 * @param opBegin_ Start operator name text representation.
	 * @param opEnd_ End operator name text representation.
	 * @param im_ Stream representing inline image.
	 */
	InlineImageCompositePdfOperator (const char* opBegin, const char* opEnd, boost::shared_ptr<CInlineImage> im);

	
	//
	// PdfOperator interface
	//
public:
	virtual size_t getParametersCount () const {return 1;}
	virtual void getParameters (Operands& opers) const;
	virtual void getStringRepresentation (std::string& str) const;
	virtual void getOperatorName (std::string& first) const {first = _opBegin;}

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


//==========================================================
} // namespace pdfobjects
//==========================================================


#endif // _PDFOPERATORS_H_
