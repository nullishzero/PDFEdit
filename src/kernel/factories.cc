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

//
#include "factories.h"

// static includes
#include "static.h"
//
#include "cpage.h"

//=======================================
namespace pdfobjects {
//=======================================

//
//
//
CPage* 
CPageFactory::getInstance (boost::shared_ptr<CDict> pageDict)
{
	return new CPage(pageDict);
}

//=======================================
}  // namespace pdfobjects
//=======================================

