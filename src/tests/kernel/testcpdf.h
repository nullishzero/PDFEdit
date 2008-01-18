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

#ifndef _TESTCPDF_H_
#define _TESTCPDF_H_

#include "tests/kernel/testmain.h"


pdfobjects::CPdf * getTestCPdf(const char* filename);

#endif // _TESTCPDF_H_
