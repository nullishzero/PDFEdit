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
 OperatorHint - class providing wrapper around single OperatorHinter
 instance and managing its configuration
 @author Martin Petricek
*/

#include "operatorhint.h"
#include "settings.h"
#include <qstring.h>
#include <stdlib.h>
#include <confparser.h>

namespace gui {

using namespace configuration;

/** Single hinter instance for entire application */
OperatorHint *OperatorHintInstance=NULL;

//Internal private data for single OperatorHint instance

/** Operator hinter instance */
OperatorHinter hinter;

/**
 Low-level parser for simple configuration files (one line = one hint).
 This base parser is used by ConfParser,
 which is used by OperatorHinter in loadFromFile 
*/
StringConfigurationParser baseParserHint(StringConfigurationParser(NULL));

/** Configuration parser */
StringConfigurationParser parserHint(baseParserHint);

/** atexit handler that will delete the single instance if it was allocated */
void removeOperatorHint() {
 if (!OperatorHintInstance) return;
 delete OperatorHintInstance;
}

/**
 Constructor
 Class will be initialized and also configuration will be read, if specified in the settings
*/
OperatorHint::OperatorHint(){
 //Register to be destroyed at exit
 atexit(removeOperatorHint);
 QString confFile=globalSettings->getFullPathName("config",QString::null,"operatorhint/");
 if (confFile.isNull()) {
  guiPrintDbg(debug::DBG_WARN,"Operator hint file not found");
 }
 guiPrintDbg(debug::DBG_DBG,"Operator hint file: " << confFile);
 int result=hinter.loadFromFile(confFile,parserHint);
 if(result==-1) {
  guiPrintDbg(debug::DBG_WARN,"Operator hinter failed to parse file: " << confFile);
  //TODO alert user
 }
};

/**
 Return Instance of OperatorHint.
 Ensures only one instance of OperatorHint exists at any time (singleton)
 If no instance exists, it is created.
 @return existing OperatorHint object
*/
OperatorHint* OperatorHint::getInstance() {
 if (!OperatorHintInstance) OperatorHintInstance=new OperatorHint();
 return OperatorHintInstance;
}

/**
 For given operator name return its description
 @param operatorName name of the operator
 @return description of operator
*/
QString OperatorHint::hint(const QString &operatorName) {
 std::string theHint;
 std::string opName=operatorName;
 hinter.getHint(opName,theHint);
 return theHint;
}

/**
 Return instance of OperatorHinter that is being used by this class
 @return OperatorHinter instance
*/
OperatorHinter* OperatorHint::get() {
 return &hinter;
}

/** Destructor */
OperatorHint::~OperatorHint(){
 OperatorHintInstance=NULL;
};

} // namespace gui
