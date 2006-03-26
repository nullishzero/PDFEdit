/*
 * =====================================================================================
 *        Filename:  ccontentstream.cc
 *         Created:  03/24/2006 10:33:34 PM CET
 *          Author:  jmisutka (), 
 * =====================================================================================
 */

// static
#include "static.h"

//
#include "ccontentstream.h"

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



//==========================================================
// Concrete implementations of PdfOperator
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
