/*                                                                              
 * PDFedit - free program for PDF document manipulation.                        
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko, 
 *                                              Miroslav Jahoda,       
 *                                              Jozef Misutka, 
 *                                              Martin Petricek                                             
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit                                                                      
 */
#ifndef __QTCOMPAT_H__
#define __QTCOMPAT_H__

/**
 @file 
 Compatibility fixes to allow Qt3 code to work in Qt4
*/
#include <qglobal.h>

#if defined(QT_VERSION) && QT_VERSION >= 0x040000

 // QT4 or newer
#define QT4 1

/** Macro working in QT3 and QT4, allowing to send QString to std::basic_stream */
#define QSTRINGDEBUG(x) (x.toUtf8().data())

#else

 // QT3
#define QT3 1

/** Macro working in QT3 and QT4, allowing to send QString to std::basic_stream */
#define QSTRINGDEBUG(x) (x.utf8())

#endif

#endif
