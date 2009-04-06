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
#ifndef _STATEUPDATER_H_
#define _STATEUPDATER_H_

// static
#include "kernel/static.h"

#include "kernel/pdfoperators.h"


//==========================================================
namespace pdfobjects {
//==========================================================

/**
 * Graphical state updater.
 *
 * Content stream is a sequence of operations which alter graphical state. We
 * need to obtain some information after each of these operations and xpdf code
 * is no suitable for this sort of things.
 *
 * If appropriate functions are defined, this mechanism could be used to display
 * the content stream. E.g. we update op*Update functions to call output device.
 */
class StateUpdater 
{
	typedef PdfOperator::BBox BBox;

private:
	/** Maximum argument count of an operator. See pdf specfication of all operators.*/
	static const size_t MAX_OPERANDS = 6;

	/** Maximum operator name length. See pdf specification of all operators.*/
	static const size_t MAX_OPERATOR_NAMELEN = 4;

public:
	/**
	 * Structure representing one operator.
	 *
	 * It is used to find an update function, which alters graphical state
	 * according to the operator and its operands.
	 *
	 * It is also used to check whether the operand count and type match the pdf specification
	 * of an operator. It the argument number is less than zero, arbitrary
	 * number of operands is allowed up to the absolute value of argNum.
	 *
	 * If the operator is a composite, endTag is the string representation of
	 * the ending operator of the composite.
	 */
	typedef struct
	{
		char name[MAX_OPERATOR_NAMELEN];	/**< Operator name. */
		int argNum;						 	/**< Number of operator arguments. */
		unsigned short types[MAX_OPERANDS];	/**< Bits represent which types are allowed. */
		
		/** Function to execute when updating position. */
		GfxState* (*update) (GfxState* , boost::shared_ptr<GfxResources>, 
						const boost::shared_ptr<PdfOperator>, 
						const PdfOperator::Operands&, BBox* rc);
		
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
								 const PdfOperator::Operands&, BBox* rc);
	
	//
	// Accessors
	//
public:
	/**
	 * Find operator specification.
	 *
	 * @param name Name of the operator.
	 */
	static const CheckTypes* findOp (const std::string& name);

	/**
	 *  Get end tag of an operator.
	 *
	 * @param name Operator name.
	 * 
	 * @return Operator end tag. Can be empty.
	 */
	static std::string getEndTag (const std::string& name);
	
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
	static boost::shared_ptr<GfxState> 
	updatePdfOperators (PdfOperator::Iterator it, 
						boost::shared_ptr<GfxResources> res, 
						/*const*/ GfxState& state, 
						Ftor ftor) 
	{
		assert (!state.isPath());		// if isPath, state is from other ccontentstream or is bad
		GfxState* tmpstate = state.copy (false);

		assert (tmpstate);
		utilsPrintDbg (debug::DBG_DBG, "");
		boost::shared_ptr<PdfOperator> op;
		BBox rc;

		// Init ftor
		ftor (res);

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

				// Update the state
				tmpstate = (chcktp->update) (tmpstate, res, op, ops, &rc);
				
			}else
			{
				// Update the state
				tmpstate = unknownUpdate (tmpstate, res, op, ops, &rc);
			}

			assert (tmpstate);
			ftor (op, rc, *tmpstate);
			it = it.next ();
		
		} // while

		// If malformed content stream (missing Q)
		while (tmpstate->hasSaves())
			tmpstate = tmpstate->restore ();

		// Return gfx state
		return boost::shared_ptr<GfxState> (tmpstate);
	}


	//
	// Helper functions
	//
public:
	static GfxState* printTextUpdate (GfxState* state, const std::string& txt, BBox* rc);

};
	

/** 
 * Is it a simple or a composite operator. 
 * @param chck Check type structure.
 * @return True if the chck is a simple operator, false otherwise.
 */
inline bool 
isSimpleOp (const StateUpdater::CheckTypes& chck)
	{ return ('\0' == chck.endTag[0]); }

/**
 * Is it a text operator (one which holds text to be displayed).
 * @param chck Check type structure.
 * @return True if chck is a text operator, false otherwise.
 */
inline bool isTextOp(const StateUpdater::CheckTypes& chck)
{
	if (!strcmp(chck.name, "TJ") || 
	    !strcmp(chck.name, "Tj") || 
	    !strcmp(chck.name, "\"") || 
	    !strcmp(chck.name, "'")
	   )
		return true;
	return false;
}

/**
 * Check if the operands match the specification and replace operand with
 * its stronger equivalent.
 *
 * (e.g. When xpdf returns an object with integer type, but the operand can be a real, we have to
 * convert it to real.)
 *
 * @param ops Operator specification
 * @param operands Operand stack.
 *
 * @return True if type and count match, false otherwise.
 */
bool checkAndFixOperator (const pdfobjects::StateUpdater::CheckTypes& ops, PdfOperator::Operands& operands);


//==========================================================
} // namespace pdfobjects
//==========================================================

#endif // _STATEUPDATER_H_
