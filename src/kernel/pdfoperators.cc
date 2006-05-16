// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80
/*
 * =====================================================================================
 *        Filename:  pdfopertors.cc
 *         Created:  03/24/2006 10:33:34 PM CET
 *          Author:  jmisutka (), 
 * =====================================================================================
 */

// static
#include "static.h"
//
#include "pdfoperators.h"
//
#include "iproperty.h"
//
#include "cinlineimage.h"

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
// Constructor
//
UnknownPdfOperator::UnknownPdfOperator (Operands& opers, const string& opTxt)
	: opText (opTxt)
{
	utilsPrintDbg (DBG_DBG, opTxt);

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
//
//
size_t 
UnknownPdfOperator::getParametersCount () const
	{ return operands.size (); }

//
//
//
void
UnknownPdfOperator::getParameters (Operands& container) const
	{ copy (operands.begin(), operands.end (), back_inserter(container) ); }

//
//
//
void
UnknownPdfOperator::getOperatorName (std::string& first) const
	{ first = opText; }
	
//
//
//
void 
UnknownPdfOperator::getStringRepresentation (std::string& str) const
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



//==========================================================
// CompositePdfOperator
//==========================================================

//
//
//
void 
CompositePdfOperator::push_back (const boost::shared_ptr<PdfOperator> op)
	{ children.push_back (op); }

//
//
//
void
CompositePdfOperator::remove (boost::shared_ptr<PdfOperator> op)
{ 
	PdfOperators::iterator it =  find (children.begin(), children.end(), op);
	assert (it != children.end());
	if (it == children.end())
		throw CObjInvalidOperation ();
	
	// Erase it
	children.erase (it); 
}

//
//
//
void 
CompositePdfOperator::getChildren (PdfOperators& container) const
	{copy (children.begin(), children.end (), back_inserter (container));}

//
//
//
void
CompositePdfOperator::getStringRepresentation (std::string& str) const
{
	//
	// Get string representation of every child and append it
	//
	// Indicate that we are a composite
	string tmp;
	PdfOperators::const_iterator it = children.begin ();
	for (; it != children.end(); ++it)
	{
		tmp.clear ();
		(*it)->getStringRepresentation (tmp);
		str += tmp + " ";
	}
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
	str += "\n";
	str += opEnd;
}

//
//
//
boost::shared_ptr<PdfOperator>
findCompositeOfPdfOperator (PdfOperator::Iterator it, 
							PdfOperator::Iterator expected, 
							PdfOperator::Iterator& next)
{
	boost::shared_ptr<PdfOperator> composite;
	while (!it.isEnd())
	{
		if (it == expected)
		{
			assert (composite);
			assert (isComposite (composite));
			
			PdfOperator::PdfOperators opers;
			composite->getChildren (opers);
			for (PdfOperator::PdfOperators::iterator itt = opers.begin(); itt != opers.end(); ++itt)
			{
				if (*itt == expected.getCurrent())
				{
					// Get the next element, if it is the end of composite,
					// simply use the next iterator
					++itt;
					if (itt != opers.end())
						next = PdfOperator::ListItem (*itt);
					else
						next = expected.next ();
					
					break;
				}
			}
			
			return composite;
		}

		// Store it if it is a composite
		if (isComposite (it))
			composite = it.getCurrent();

		it.next ();
	}

	// Weh should have found the operator
	assert (!"Operator not found...");
	throw CObjInvalidOperation ();
}

//==========================================================
} // namespace pdfobjects
//==========================================================
