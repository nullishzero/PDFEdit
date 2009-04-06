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
/** @file
 Utility functions to work with commandline arguments
 \brief Commandline arguments utility functions
 @author Martin Petricek
*/

#include "args.h"
#include "qtcompat.h"
#include "util.h"
#include "version.h"
#include <assert.h>
#include <iostream>
#include <qobject.h>
#include QLIST
#include <qstringlist.h>
#include <stdlib.h>

using namespace std;
using namespace util;

/** Option handler map*/
typedef QMap<QString, optHandler> OptionMap;

/** Option handler map (with parameter) */
typedef QMap<QString, optHandlerParam> OptionMapParam;

/** Option help map*/
typedef QMap<QString, QString> OptionHelp;

/** Stop processing options from commandline? */
bool stopOpt=false;

/** Name of the program (argv[0]) */
QString binName;
/** Option help texts */
OptionHelp optHelp;
/** Option parameter names */
OptionHelp optParamName;
/** Option handlers (without parameters) */
OptionMap optMap;
/** Option handlers (with parameters) */
OptionMapParam optMapParam;

/** Argument index currently processed */
int argIndex;
/** Standard argc */
int argc;
/** Standard argv */
char **argv;
/** Length of longest parameter name (including name of its argument)*/
int maxParamNameLen=1;

//Forward declarations that should not be visible in header file
int handleOption(const QString &param);
const QString nextParam(const QString &param);

/**
 Print out application header (name and version) to stdout
*/
void printHeader() {
 cout << APP_NAME << " " << PDFEDIT_VERSION << endl;
}

/**
 Print out application header (name and version) to stderr
*/
void printHeaderErr() {
 cerr << APP_NAME << " " << PDFEDIT_VERSION << endl;
}

/**
 Standard handler to handle --help parameter
 Prints all registered commandline options along with their helptext to STDOUT
 and exits
*/
void handleHelpOptions(){
 cout << convertFromUnicode(QObject::tr("Options: "),CON) << endl;
 Q_List<QString> opt=optHelp.keys();
 for (Q_List<QString>::Iterator it=opt.begin();it!=opt.end();++it) {
  QString paramName=*it;
  if (optParamName.contains(paramName)) { //Add name of parameter
   paramName+=" ["+optParamName[paramName]+"]";
  }
  cout << " ";
  cout.width(maxParamNameLen+1);		//width of option name
  cout.flags(ios::left);
  cout << convertFromUnicode(paramName,CON) << convertFromUnicode(optHelp[*it],CON) << endl;
 }
 exit(0);
}

/** handle -- parameter (stop processing option) */
void handleStopOpt(){
 stopOpt=true;
}

/**
 Register function to handle option without parameter
 @param param Name of option (case sensitive)
 @param h Function to handle this option
 @param help Brief one-line help about this option
*/
void optionHandler(const QString &param, optHandler h,const QString &help) {
 optMap[param]=h;
 optHelp[param]=help;
 int len=param.length();	//Length of parameter including its name in help
 if (len>maxParamNameLen) maxParamNameLen=len;
}

/**
 Register function to handle option with parameter
 Parameters can be accepted as: -opt[n], -opt [n]
 @param param Name of option (case sensitive)
 @param paramName Name of parameter (only shown in help)
 @param h Function to handle this option
 @param help Brief one-line help about this option
*/
void optionHandlerParam(const QString &param, const QString &paramName, optHandlerParam h,const QString &help) {
 optMapParam[param]=h;
 optHelp[param]=help;
 optParamName[param]=paramName;
 int len=param.length()+3+paramName.length();	//Length of parameter including its name in help
 if (len>maxParamNameLen) maxParamNameLen=len;
}

/**
 Calls option handler on given option
 @param param Commandline option to check
 @return 0 if option is valid, but without parameters, 1 if option 'consumed' parameter after it, -1 if option is invalid
*/
int handleOption(const QString &param) {
 //parameterless option
 if (optMap.contains(param)) {
  assert(optMap[param]);
  optMap[param]();
  return 0;
 }
 //parameterized option, exact match, option parameter is next parameter
 if (optMapParam.contains(param)) {
  assert(optMapParam[param]);
  optMapParam[param](nextParam(param));
  return 1;
 }
 //parameterized option, option parameter is directly appended
 QString paramBase;
 for(unsigned int i=2;i<param.length();i++) {
  //Try to find prefix as name of parameters
  paramBase=param.left(i);
  if (optMapParam.contains(paramBase)) {
   //option is found -> execute handler
   assert(optMapParam[paramBase]);
   optMapParam[paramBase](param.mid(i));
   return 0;
  }
 }
 return -1;
}

/**
 Sets name of binary to show in help, etc ...
 (parameter should be argv[0])
 @param name name of binary
*/
void setArgv0(const QString &name) {
 binName=name;
}

/**
 Get next parameter from commandline
 @param param name of currently processed parameter
*/
const QString nextParam(const QString &param) {
 argIndex++;
 if (argIndex<argc) {
  assert(argv[argIndex]);
  return argv[argIndex];
 }
 printHeaderErr();
 fatalError(QObject::tr("Parameter missing for option : ")+param+"\n\n"
           +QObject::tr("Use '%1 --help' to see list of commandline options and their parameters").arg(binName));
 return "";
}

/**
 Function to handle commandline parameters using installed callbacks
 Will terminate program with error message if unknown or invalid commandline options are found
 @param _argc count of parameters (standard argc)
 @param _argv parameters (standard argv)
 @return List of non-option parameters;
*/
QStringList handleParams(int _argc,char **_argv) {
 argc=_argc;
 argv=_argv;
 setArgv0(argv[0]);
 QString param;
 QStringList params;
 for (argIndex=1;argIndex<argc;argIndex++) {
  assert(argv[argIndex]);
  param=argv[argIndex];
  if (param.startsWith("-") && !stopOpt) { //option
   if (handleOption(param)==-1) {
    printHeaderErr();
    fatalError(QObject::tr("Invalid commandline option : ")+param+"\n\n"
              +QObject::tr("Use '%1 --help' to see list of commandline options and their parameters").arg(binName));
   }
  } else {
   params+=param;
  }
 }
 return params;
}
