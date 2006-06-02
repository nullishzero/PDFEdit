/** @file
 PropertyModeController - class providing wrapper around single ModeController
 instance and managing its configuration
 @author Martin Petricek
*/

#include "propertymodecontroller.h"
#include "settings.h"
#include <qstring.h>
#include <stdlib.h>
#include <modecontroller.h>
#include <confparser.h>

namespace gui {

using namespace configuration;

/** Single property mode instance for entire application */
PropertyModeController *propertyModeControllerInstance=NULL;

//Internal private data for single PropertyModeController instance

/** Mode Controller instance */
ModeController modeControler;

/**
 Low-level parser for simple configuration files (one line = one setting).
 This base parser is used by ConfParser,
 which is used by ModeController in loadFromFile 
*/
StringConfigurationParser baseParser(NULL);

/** Configuration parser */
ModeController::ConfParser parser(baseParser);

/** atexit handler that will delete the single instance if it was allocated */
void removePropertyModeController() {
 if (!propertyModeControllerInstance) return;
 delete propertyModeControllerInstance;
}

/**
 Constructor
 Class will be initialized and also configuration will be read, if specified in the settings
*/
PropertyModeController::PropertyModeController(){
 //Register to be destroyed at exit
 atexit(removePropertyModeController);
 QString confFile=globalSettings->getFullPathName("config",QString::null,"modecontroller/");
 if (confFile.isNull()) {
  guiPrintDbg(debug::DBG_WARN,"Mode controller config not found");
 }
 guiPrintDbg(debug::DBG_DBG,"Mode controller config file: " << confFile);
 int result=modeControler.loadFromFile(confFile,parser);
 if(result==-1) {
  guiPrintDbg(debug::DBG_WARN,"Mode controller failed to parse file: " << confFile);
  //TODO alert user
 }
};

/**
 Return Instance of PropertyModeController.
 Ensures only one instance of PropertyModeController exists at any time (singleton)
 If no instance exists, it is created.
 @return existing PropertyModeController object
*/
PropertyModeController* PropertyModeController::getInstance() {
 if (!propertyModeControllerInstance) propertyModeControllerInstance=new PropertyModeController();
 return propertyModeControllerInstance;
}

/**
 For given type and name return mode of the property
 @param type Type field value of complex type
 @param name Name of property
*/
PropertyMode PropertyModeController::mode(const QString &type,const QString &name) {
 return modeControler.getMode(type,name);
}

/** Destructor */
PropertyModeController::~PropertyModeController(){
 propertyModeControllerInstance=NULL;
};

} // namespace gui
