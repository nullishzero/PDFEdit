// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80
/*
 * =====================================================================================
 *        Filename:  stateupdater.cc
 *         Created:  06/10/2006 10:33:34 PM CET
 *          Author:  jmisutka, mjahoda
 * =====================================================================================
 */
#ifndef _STATEUPDATER_H_
#define _STATEUPDATER_H_

// static
#include "static.h"

#include "pdfoperators.h"


//==========================================================
namespace pdfobjects {
//==========================================================

/**
 * General state updater.
 */
class StateUpdater 
{

private:
	/** Maximum argument count of an operator. See pdf specfication of all operators.*/
	const static size_t MAX_OPERANDS = 6;

	/** Maximum operator name length. See pdf specification of all operators.*/
	const static size_t MAX_OPERATOR_NAMELEN = 4;

public:
	/**
	 * Structure representing one operator.
	 *
	 * It is used to find an update function, which alters graphical state
	 * according to the operator and its operands.
	 *
	 * It is also used to check whether the operand cound and type match the pdf specifiaction
	 * of an operator. It the argument number is less than zero, arbitrary
	 * number of operands is allowed up to the absolute value of argNum.
	 *
	 * If the operator is a composite, endTag is the string representation of
	 * the ending operator of the composite..
	 */
	typedef struct
	{
		char name[MAX_OPERATOR_NAMELEN];	/**< Operator name. */
		int argNum;						 	/**< Number of operator arguments. */
		unsigned short types[MAX_OPERANDS];	/**< Bits represent which types are allowed. */
		
		/** Function to execute when updating position. */
		GfxState* (*update) (GfxState* , boost::shared_ptr<GfxResources>, 
						const boost::shared_ptr<PdfOperator>, 
						const PdfOperator::Operands&, Rectangle* rc);
		
		char endTag[MAX_OPERATOR_NAMELEN]; /**< If it is a complex type, its end tag.*/	
		
	} CheckTypes;

		
protected:
	/**
	 * All known operators from pdf specification and their update functions. 
	 *
	 * If an operator does not have any effect on graphical state, default
	 * update function is used.
	 * 
	 * Operator number can be either >0, zero, <0. Zero means no operands are
	 * needed. >0 means that exact argNum of operands are needed. <0 means that
	 * at most argNum operands are needed.
	 */
	static CheckTypes KNOWN_OPERATORS[];

	//
	// Default update
	//
public:
	static GfxState* unknownUpdate (GfxState* state, 
								 boost::shared_ptr<GfxResources>, 
								 const boost::shared_ptr<PdfOperator>, 
								 const PdfOperator::Operands&, Rectangle* rc);
	
	//
	// Accessors
	//
public:
	/**
	 * Find operator specification.
	 *
	 * @param name Name of the operator.
	 */
	const CheckTypes* findOp (const std::string& name) const;

	/**
	 *  Get end tag of an operator.
	 *
	 * @param name Operator name.
	 * 
	 * @return Operator end tag. Can be empty.
	 */
	std::string getEndTag (const std::string& name) const;
	
public:
	/**
	 *  Update pdf operators.
	 *
	 *  REMARK: State is not gfx changed in this function. We can not make it
	 *  const because of the xpdf code.
	 *  
	 * @param it Iterator that will be used to traverse all operators.
	 * @param res Graphical resources.
	 * @param state Graphical state.
	 * @param ftor Functor applied after each update.
	 */
	template <typename Ftor>
	void updatePdfOperators (PdfOperator::Iterator it, boost::shared_ptr<GfxResources> res, /*const*/ GfxState& state, Ftor ftor) const
	{
		assert (!state.isPath());		// if isPath, state is from other ccontentstream or is bad
		GfxState* tmpstate = state.copy (false);

		utilsPrintDbg (debug::DBG_DBG, "");
		boost::shared_ptr<PdfOperator> op;
		Rectangle rc;

		// Init global variables
		xpdf::openXpdfMess ();
		
		while (!it.isEnd ())
		{
			op = it.getCurrent();
			// Get operator name
			std::string frst;
			op->getOperatorName(frst);
			// Get operator specification
			const CheckTypes* chcktp = findOp (frst);
			// Get operands
			PdfOperator::Operands ops;
			op->getParameters (ops);
			// If operator found use the function else use default
			if (NULL != chcktp)
			{
				// Check arguments
				if ( ((chcktp->argNum >= 0) && (ops.size () != (size_t)chcktp->argNum)) ||
				      ((chcktp->argNum < 0) && (ops.size () > (size_t)-chcktp->argNum)) )
				{
					kernelPrintDbg (debug::DBG_CRIT, "Bad content stream. Incorrect parameters.");
					
					// Delete gfx state
					delete tmpstate;
					
					throw CObjInvalidObject ();
				}

				// Update the state and 
				tmpstate = (chcktp->update) (tmpstate, res, op, ops, &rc);
				
			}else
			{
				// Update the state
				tmpstate = unknownUpdate (tmpstate, res, op, ops, &rc);
			}

			assert (tmpstate);
			ftor (op, *tmpstate, rc);
			it = it.next ();
		
		} // while

		// Close xpdf mess
		xpdf::closeXpdfMess ();
		// Delete gfx state
		delete tmpstate;
	}

};
	
/** 
 * Is it a simple or a composite operator. 
 * @param chck Check type structure.
 * @return True if the check type is a simple operator, false otherwise.
 */
inline bool 
isSimpleOp (const StateUpdater::CheckTypes& chck)
	{ return ('\0' == chck.endTag[0]); }



//==========================================================
} // namespace pdfobjects
//==========================================================

#endif // _STATEUPDATER_H_
