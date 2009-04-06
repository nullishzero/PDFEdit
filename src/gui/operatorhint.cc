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
 OperatorHint - class providing wrapper around single OperatorHinter
 instance and managing its configuration
 @author Martin Petricek
*/

#include "operatorhint.h"
#include "qtcompat.h"
#include "settings.h"
#include <qstring.h>
#include <stdlib.h>
#include <utils/confparser.h>
#include "util.h"

namespace gui {

using namespace configuration;
using namespace util;

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
 guiPrintDbg(debug::DBG_DBG,"Operator hint file: " << Q_OUT(confFile));
 int result=hinter.loadFromFile(convertFromUnicode(confFile,NAME),parserHint);
 if(result==-1) {
  guiPrintDbg(debug::DBG_WARN,"Operator hinter failed to parse file: " << Q_OUT(confFile));
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
 std::string opName=convertFromUnicode(operatorName,util::PDF);
 hinter.getHint(opName,theHint);
 return convertToUnicode(theHint,util::UTF8);
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
