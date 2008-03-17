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

// static
#include "kernel/static.h"
#include "kernel/pdfoperators.h"
//
#include "kernel/pdfoperatorsiter.h"
#include "kernel/iproperty.h"
#include "kernel/cinlineimage.h"

#include "kernel/ccontentstream.h"

//==========================================================
namespace pdfobjects {
//==========================================================

using namespace std;
using namespace boost;
using namespace debug;


//==========================================================
// Concrete implementations of PdfOperator
//==========================================================

//
//
//
void 
SimpleGenericOperator::getStringRepresentation (std::string& str) const
{
	std::string tmp;
	for (Operands::const_iterator it = operands.begin(); it != operands.end (); ++it)
	{
		tmp.clear ();
		(*it)->getStringRepresentation (tmp);
		str += tmp + " ";
	}

	// Add operator string
	str += opText;
}
	

//
//
//
shared_ptr<PdfOperator> 
SimpleGenericOperator::clone ()
{
	// Clone operands
	Operands ops;
	for (Operands::iterator it = operands.begin (); it != operands.end(); ++it)
		ops.push_back ((*it)->clone());
	assert (ops.size () == operands.size());

	// Create clone
	return shared_ptr<PdfOperator> (new SimpleGenericOperator (opText,ops));
}


//==========================================================
// Concrete implementations of CompositePdfOperator
//==========================================================

//
// UnknownCompositePdfOperator
//

//
//
//
UnknownCompositePdfOperator::UnknownCompositePdfOperator 
	(const char* opBegin_, const char* opEnd_) : CompositePdfOperator (), opBegin (opBegin_), opEnd (opEnd_)
{
	utilsPrintDbg (DBG_DBG, "Unknown composite operator: " << opBegin_ << " " << opEnd_);

}

//
//
//
void
UnknownCompositePdfOperator::getStringRepresentation (string& str) const
{
	// Header
	str += opBegin; str += " ";
	
	// Delegate
	CompositePdfOperator::getStringRepresentation (str);	
}

//
//
//
shared_ptr<PdfOperator> 
UnknownCompositePdfOperator::clone ()
{
	shared_ptr<UnknownCompositePdfOperator> clone (new UnknownCompositePdfOperator(opBegin,opEnd));

	for (PdfOperators::iterator it = _children.begin(); it != _children.end(); ++it)
		clone->push_back ((*it)->clone(),getLastOperator(clone));
	
	// Create clone
	return clone;
}

//
// InlineImageCompositePdfOperator
//

//
//
//
InlineImageCompositePdfOperator::InlineImageCompositePdfOperator 
	(const char* opBegin_, const char* opEnd_, boost::shared_ptr<CInlineImage> im_) 
		: CompositePdfOperator (), opBegin (opBegin_), opEnd (opEnd_), inlineimage (im_)
{
	utilsPrintDbg (DBG_DBG, opBegin_ << " " << opEnd_);
}

//
//
//
void
InlineImageCompositePdfOperator::getStringRepresentation (string& str) const
{
	// Header
	str += opBegin; str += "\n";
	// 
	if (inlineimage)
	{
		std::string tmp;
		inlineimage->getStringRepresentation (tmp);	
		str += tmp;
	}else
	{
		assert (!"Bad inline image.");
		throw CObjInvalidObject ();
	}
	// Footer
	str += opEnd; str += "\n";

}

//
//
//
void
InlineImageCompositePdfOperator::getParameters (Operands& opers) const
{
	boost::shared_ptr<IProperty> ip = inlineimage;
	opers.push_back (ip);
}

//
//
//
shared_ptr<PdfOperator> 
InlineImageCompositePdfOperator::clone ()
{
	// Clone operands
	shared_ptr<CInlineImage> imgclone = IProperty::getSmartCObjectPtr<CInlineImage> (inlineimage->clone());
	// Create clone
	return shared_ptr<PdfOperator> (new InlineImageCompositePdfOperator (opBegin, opEnd, imgclone));
}


//==========================================================
// Helper funcions
//==========================================================

//
//\todo improve performance
//
boost::shared_ptr<CompositePdfOperator>
findCompositeOfPdfOperator (PdfOperator::Iterator it, boost::shared_ptr<PdfOperator> oper)
{
	boost::shared_ptr<CompositePdfOperator> composite;
	typedef PdfOperator::PdfOperators Opers;
	Opers opers;


	while (!it.isEnd())
	{
		// Have we found what we were looking for
		if (isCompositeOp(it))
		{
			opers.clear ();
			it.getCurrent()->getChildren (opers);
			if (opers.end() != std::find (opers.begin(), opers.end(), oper))
				return boost::dynamic_pointer_cast<CompositePdfOperator, PdfOperator> (it.getCurrent());
		}else
		{
			// This can happen only in the "first level" but that should be
			// handled in caller
			if (it.getCurrent() == oper)
			{
				assert (!"Found highest level operator, that should be handled in the caller of this function.");
				throw CObjInvalidObject ();
			}
		}

		it.next ();
	}

	//
	// We should have found the operator
	// -- this can happen in an incorrect script
	// 		that remembers reference to a removed object
	// 
	//assert (!"Operator not found...");
	throw CObjInvalidOperation ();
}


	
//==========================================================
} // namespace pdfobjects
//==========================================================
