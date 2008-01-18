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

#ifndef _TESTCPAGE_H_
#define _TESTCPAGE_H_

#include "tests/kernel/testmain.h"

//
//
//
boost::shared_ptr<CPage> getPage (const char* fileName, boost::shared_ptr<CPdf> pdf, size_t pageNum = 1);

#endif // _TESTCPAGE_H_

