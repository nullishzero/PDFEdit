/*                                                                              
 * PDFedit - free program for PDF document manipulation.                        
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko, 
 *                                              Miroslav Jahoda,       
 *                                              Jozef Misutka, 
 *                                              Martin Petricek                                             
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit                                                                      
 */ 
// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80

#include "kernel/static.h"

#include "kernel/xpdf.h"
#include "kernel/exceptions.h"

//=====================================================================================
namespace xpdf {
//=====================================================================================

//
//
//
void 
freeXpdfObject (::Object* obj)
{
		assert (obj != NULL);
		if (NULL == obj)
			throw XpdfInvalidObject ();
	
	// delete all member variables
	obj->free ();
	// delete the object itself
	gfree(obj);
}


//=====================================================================================
} // namespace xpdf
//=====================================================================================
