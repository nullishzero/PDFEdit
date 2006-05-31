// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80

/*
 *
 * $RCSfile$
 *
 * $Log$
 * Revision 1.5  2006/05/31 07:06:24  hockm0bm
 * doc update and prepared to be implemented
 *         - just for sync
 *
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








