/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006-2009  PDFedit team: Michal Hocko,
 *                                        Jozef Misutka,
 *                                        Martin Petricek
 *                   Former team members: Miroslav Jahoda
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (in doc/LICENSE.GPL); if not, write to the 
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 * MA  02111-1307  USA
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit
 */
#ifndef __ARGS_H__
#define __ARGS_H__
/** @file
 \brief Commandline arguments utility functions header
*/

#include <qmap.h>
#include <qstring.h>

class QStringList;

/**
 Name of the program (argv[0]).
 This variable will be set after running handleParams
*/
extern QString binName;

/** Option handler function */
typedef void optHandlerFn();

/** Option handler function (with parameter) */
typedef void optHandlerFnParam(const QString&);

/** Option handler function pointer*/
typedef optHandlerFn *optHandler;

/** Option handler function pointer (with parameter)*/
typedef optHandlerFnParam *optHandlerParam;

void handleHelpOptions();
void handleStopOpt();
void optionHandler(const QString &param, optHandler h,const QString &help="");
void optionHandlerParam(const QString &param, const QString &paramName, optHandlerParam h,const QString &help="");
QStringList handleParams(int _argc,char **_argv);
void printHeader();
void printHeaderErr();

#endif
