/*                                                                              
 * PDFedit - free program for PDF document manipulation.                        
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko, 
 *                                              Miroslav Jahoda,       
 *                                              Jozef Misutka, 
 *                                              Martin Petricek                                             
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
#include <assert.h>
#include <iostream>
#include <qobject.h>
#include QLIST
#include <qstringlist.h>
#include <stdlib.h>

using namespace std;
using namespace util;

/** Stop processing options from comandline? */
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

//Forward declarations that should not be visible in header file
int handleOption(const QString &param);
const QString nextParam(const QString &param);

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
  cout.width(16);		//width of option name
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
 fatalError(QObject::tr("Parameter missing for option : ")+param);
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
   if (handleOption(param)==-1) fatalError(QObject::tr("Invalid commandline option : ")+param);
  } else {
   params+=param;
  }
 }
 return params;
}
