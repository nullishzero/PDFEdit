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
	printDbg (DBG_DBG, "Unknown operator: " << opTxt);

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
UnknownPdfOperator::getParameters (IPContainer& container) const
	{ copy (operands.begin(), operands.end (), back_inserter(container) ); }

//
//
//
void 
UnknownPdfOperator::getOperatorName (std::string& first, std::string& last) const
{ 
	first = opText; last.clear (); 
}
	
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
// Concrete implementations of CompositePdfOperator
//==========================================================



//==========================================================
} // namespace pdfobjects
//==========================================================
