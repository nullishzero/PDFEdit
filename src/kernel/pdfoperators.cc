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
#include "ccontentstream.h"


//==========================================================
namespace pdfobjects {
//==========================================================

using namespace std;
using namespace boost;
using namespace debug;


//==========================================================
// Concrete implementations of PdfOperator
//==========================================================



extern const char OPER_MOVETONEXTLINE [] 		= "'"; 
extern const char OPER_FILLSTROKE [] 			= "B"; 
extern const char OPER_EOFILLSTROKE [] 			= "B*"; 


//
// Constructor
//
UnknownPdfOperator::UnknownPdfOperator (Operands& opers, const char* opTxt)
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
UnknownPdfOperator::getStringRepresentation (std::string& str) const
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
}
	

//==========================================================
// Concrete implementations of CompositePdfOperator
//==========================================================


//
// Add an operator to the composite.
//
void 
CompositePdfOperator::push_back (const shared_ptr<PdfOperator> oper)
{
	printDbg (DBG_DBG, "CompositePdfOperator::add");
	operators.push_back (oper);
}

//
// Remove an operator.
//
void 
CompositePdfOperator::remove (shared_ptr<PdfOperator> oper)
{
	printDbg (DBG_DBG, "CompositePdfOperator::add");

	PdfOperators::iterator it = find (operators.begin(), operators.end(), oper);
	
	operators.erase (it);
}


//==========================================================
} // namespace pdfobjects
//==========================================================
