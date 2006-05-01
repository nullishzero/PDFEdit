/*
 * =====================================================================================
 *        Filename:  modecontroller.cc
 *     Description: .
 *         Created:  12/03/2006 22:35:14 PM CET
 *          Author:  jmisutka (), 
 *         Changes: 
 * =====================================================================================
 */

// static
#include "static.h"

#include "modecontroller.h"


// =====================================================================================
using namespace std;

//
//
//
ModeController&
ModeController::Instance (const std::string& fileName)
{
	// Create the object and get modes
	static ModeController obj (fileName);
	
	return obj;
}

//
//
//
ModeController::ModeController (const std::string& /*fileName*/) 
{
		utilsPrintDbg (debug::DBG_INFO, "Mode controller created.");

		//
		// get the modes
		// 	?xml 
		// 		-- QT
		// 			-- dom
		// 			-- sax
		// 	?plain file
		//
}


//
//
//
PropertyMode
ModeController::getMode (const string& /*type*/, const string& /*name*/) const
{
	//
	// find it and return item.second
	//
	
	return mdUnknown;
}








