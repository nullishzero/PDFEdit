/*
 * =====================================================================================
 *        Filename:  filters.cc
 *     Description: .
 *         Created:  12/03/2006 14:26:56 CET
 *          Author:  jmisutka (), 
 *         Changes: 
 * =====================================================================================
 */

//
#include "filters.h"

// static includes
#include "static.h"

//=======================================
namespace filters  {
//=======================================

		
void 
NoFilter::decode (const Params& /*params , char* ... */) const
{
	printDbg (debug::DBG_DBG, "NoFilter::decode"); 
}


//=======================================
}  // namespace filters
//=======================================

