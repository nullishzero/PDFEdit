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

/*
 * $RCSfile$
 *
 * $Log$
 * Revision 1.7  2007/02/04 20:17:03  mstsxfx
 * Common Licence comment for all cc and h files available in doc/licence_header
 * file and its content to all cc and h files in src/{gui,kernel,utils}
 * directories.
 * Xpdf code, QSA and QOutputDevice code are not touched
 *
 * Revision 1.6  2006/04/22 19:32:21  hockm0bm
 * * old test style replaced by CPPUINT TestCPdf class
 * * TestCPdf::pageIterationTC method implemented
 *
 * Revision 1.5  2006/04/12 20:40:44  hockm0bm
 * getTestCPdf is not inline now - because of linking problem for kernel binary
 *
 * Revision 1.4  2006/04/12 20:19:33  hockm0bm
 * methods reorganization
 *
 * Revision 1.3  2006/04/09 21:29:33  misuj1am
 *
 *
 * -- changed the method that creates CPdf to match my views
 *
 * Revision 1.2  2006/04/03 14:38:21  misuj1am
 *
 *
 * -- ADD: getTestPdf function that returns valid pdf object
 *
 * Revision 1.1  2006/04/01 00:39:50  misuj1am
 *
 *
 * -- tests
 *
 * Revision 1.21  2006/03/31 21:14:40  hockm0bm
 * * file format changed (tabulators - same style)
 * * CVS header
 * * main changed to contain only test functions
 *         - all code from main moved to cobjects_test function
 *         - added cpdf_tests function for CPdf testing
 *
 *
 */
#ifndef _TESTCPDF_H_
#define _TESTCPDF_H_

#include "testmain.h"


pdfobjects::CPdf * getTestCPdf(const char* filename);

#endif // _TESTCPDF_H_
