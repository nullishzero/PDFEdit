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
 Base Core - class that host scripts
 This class is also responsible for garbage collection of scripting objects
 @author Martin Petricek
*/

#include "basecore.h"
#include "consolewriter.h"
#include "qscobject.h"
#include "qsimporter.h"
#include "qspdf.h"
#include "qstreeitem.h"
#include "settings.h"
#include <qsinterpreter.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qsutilfactory.h>
#include <qsinputdialogfactory.h>
#include <utils/debug.h>
#include "util.h"
#include "qswrapper.h"

namespace gui {

/**
 Create new BaseCore class
*/
BaseCore::BaseCore() {
 con=NULL;
 //Create new interpreter and project
 qp=new QSProject(this,"qs_project");
 qs=qp->interpreter();
 qs->setErrorMode(QSInterpreter::Nothing);
 qs->addWrapperFactory(new QSWrapper());
 connect(qs,SIGNAL(error(const QString&,const QString&,int)),this,SLOT(scriptError(const QString&,const QString&,int)));

//TODO: connect QSInterpreter::timeout ( int elapsedTime ) and somehow allow to kill bad script

 //Add ability to open files, directories and run processes
 qs->addObjectFactory(new QSUtilFactory());
 //Add ability to create dialogs
 qs->addObjectFactory(new QSInputDialogFactory());

 //Add settings object
 assert(globalSettings);
 qp->addObject(globalSettings);
 //Create and add importer to QSProject and related QSInterpreter
 import=new QSImporter(qp,this,this);
 qpdf=new QSPdf(boost::shared_ptr<CPdf>(),this);
}

/**
 Set new console writer for this class.
 NULL mean no console writer, effectively turning off console output
 @param _con Console writer
*/
void BaseCore::setConWriter(ConsoleWriter *_con) {
 //Console writer
 con=_con;
}

/**
 Import currently edited document into scripting
  QSPDF wrapper will be created around the pdf document
 @param pdf PDF to import under name "document" into scripting
*/
void BaseCore::importDocument(boost::shared_ptr<pdfobjects::CPdf> pdf) {
 qpdf->set(pdf);
 //TODO: When QSA wrapper factory will be implemented, this need to change
 import->addQSObj(qpdf,"document");
}

/** destroy document - destroy it also in scripting */
void BaseCore::destroyDocument() {
 qpdf->set(boost::shared_ptr<CPdf>());
 deleteVariable("document");
}

/**
 Return QSA wrapper of current PDF document, or NULL if the wrapper is invalid or not present
 @return Current document (scripting wrapper)
 */
QSPdf* BaseCore::getQSPdf() const {
 if (qpdf->get()) return qpdf;
 return NULL;
}

/**
 Return interpreter instance used to launch scripts in this context
 @return current QSInterpreter
*/
QSInterpreter* BaseCore::interpreter() {
 return qs;
}

/**
 Removes objects added with addScriptingObjects
 \see addScriptingObjects
 */
void BaseCore::removeScriptingObjects() {
 //Can be overridden to remove extra objects
}

/**
 Create objects that should be available to scripting from current CPdf and related objects
 \see removeScriptingObjects
*/
void BaseCore::addScriptingObjects() {
 //Can be overridden to add extra objects
}

/**
 "Remove" defined variable from scripting context
 @param varName name of variable;
*/
void BaseCore::deleteVariable(const QString &varName) {
 qs->evaluate(varName+"=undefined;",this,"<delete_item>");
}

/** if any script is running, stop it */
void BaseCore::stopScript() {
 if (qs->isRunning()) {
  qs->stopExecution();
 }
}

/**
 Call a callback function (without return value) in a script
 @param name Function name
 @param arguments Function arguments, separated by comma. Strings must be properly quoted
*/
void BaseCore::call(const QString &name,const QString &arguments/*=""*/) {
 QString funct=name+"("+arguments+");";
 guiPrintDbg(debug::DBG_INFO,"Performing callback: " << Q_OUT(name));
 //Check if this call handler is called from a script
 bool running=qs->isRunning();
 if (!running) {
  //If called from inside running script, do not perform initialization (it was alreadydone)
  preRun(funct,true);
  //Do not tamper with the variables while the script is running
  addScriptingObjects();
 }
 try {
  guiPrintDbg(debug::DBG_DBG,"Begin callback: " << Q_OUT(name));
  //Call the function. Do not care about result
  qs->evaluate(funct,this,"<GUI>");
  guiPrintDbg(debug::DBG_DBG,"End callback: " << Q_OUT(name));
  if (globalSettings->readBool("console/show_handler_errors")) { //Show return value on console;
   QString error=qs->errorMessage();
   if (error!=QString::null) { /// some error occured
    con->printErrorLine(tr("Error in callback handler: ")+name);
    con->printErrorLine(error);
   }
  }
 } catch (...) {
  guiPrintDbg(debug::DBG_INFO,"Exception in callback: " << Q_OUT(name));
  //Do not care about exception in callbacks either ...
  if (globalSettings->readBool("console/show_handler_errors")) { //Show return value on console;
   con->printErrorLine(tr("Exception in callback handler: ")+name);
  }
 }
 if (!running) {
  //Do not tamper with the variables while the script is running
  //It may be undesirable to remove the objects while script will continue execution
  removeScriptingObjects();
  postRun();
 }
}

/**
 Print one line to console, followed by newline
 @param line Line to print
*/
void BaseCore::conPrintLine(const QString &line) {
 if (!con) return;
 con->printLine(line);
}

/**
 Print one error line to console, followed by newline
 @param line Error line to print
*/
void BaseCore::conPrintError(const QString &line) {
 if (!con) return;
 con->printErrorLine(line);
}

/**
 Slot called on error in any script
 @param message Error message
 @param scriptName Name of script in which the error occured
 @param lineNumber
*/
void BaseCore::scriptError(const QString &message,const QString &scriptName,int lineNumber) {
 guiPrintDbg(debug::DBG_DBG,"Script error in " << Q_OUT(scriptName) << " line " << lineNumber);
 guiPrintDbg(debug::DBG_DBG,"Script error: " << Q_OUT(message));
//It is not a good idea to print the message, at if it
// happened in the included file it is re-throw again after returing from include
// conPrintError(tr("In script")+" '"+scriptName+"', "+tr("line")+" "+QString::number(lineNumber)+":");
// conPrintError(message);

 //Store only first message, as the others will have bad linenumber
 if (!errMessage.isNull()) return;
 errMessage=message;
 errScript=scriptName;
 errLineNumber=lineNumber;
}

/**
 Clear internal error state
 Usually called after printing the error, so to avoid seeing the same error again
 */
void BaseCore::clearError() {
 errMessage=QString::null;
 errScript=QString::null;
}

/**
 Runs given script code
 @param script QT Script code to run
*/
void BaseCore::runScript(const QString &script) {
 preRun(script);
 //Before running script, add document-related objects to scripting engine and remove tham afterwards
 addScriptingObjects();
 QSArgument ret;
 try {
  ret=qs->evaluate(script,this,"<GUI>");
 } catch (...) {
  conPrintLine(tr("Unknown exception in script occured"));
 }

 if (globalSettings->readBool("console/showretvalue")) { //Show return value on console;
  switch (ret.type()) {
   case QSArgument::QObjectPtr: { //QObject -> print type
    QObject *ob=ret.qobject();
    conPrintLine(QString("(Object:")+ob->className()+")");
    break;
   }
   case QSArgument::VoidPointer: { //void *
    conPrintLine("(Pointer)");
    break;
   }
   case QSArgument::Variant: { //Variant - simple type.
    QVariant v=ret.variant();
    if (!v.isNull()) {
     //Null -> nothing to show
     QString retVar=v.toString();
     if (!retVar.isNull()) {
      //Type convertable to string
      conPrintLine(retVar);
     } else {
      //More complex type
      if (globalSettings->readBool("console/showretvalue_complex")) { //Show return value of complex types
       if (v.canCast(QVariant::StringList)) {
        //Print as string list
        QString list=v.toStringList().join("\n");
        conPrintLine(list);
       } else {
        //TODO: some more types in future?
        QString tName=v.typeName();
        assert(!tName.isNull());
        guiPrintDbg(debug::DBG_WARN,"Cannot display result: " << Q_OUT(tName));
        conPrintLine(QString("[")+tName+"]");
       }
      }
     }
    }
    break;
   }
   default: {
    //Invalid - print nothing (void, etc ...)
   }
  }
 }
/*
//Error would be printed directly by the handler
 QString error=qs->errorMessage();
 if (error!=QString::null) { /// some error occured
  con->printErrorLine(error);
 }
*/
 removeScriptingObjects();
 postRun();
}

/**
 If there was some error since last call of this function
 or since last clearing the error message, display it.
 Clear the error message before returning.
*/
void BaseCore::errorMessage() {
 if (errMessage.isNull()) return;
 //There was some error -> print it
 conPrintError(tr("In script")+" '"+errScript+"', "+tr("line")+" "+QString::number(errLineNumber)+":");
 conPrintError(errMessage);
 clearError();//Clear error message
}

/**
 Function to be run after the script is executed
 BaseCore::postRun should be called in overrriden function if overiding with own function
*/
void BaseCore::postRun() {
 //Can be overridden to do some extra cleanup
 errorMessage();
}

/**
 Function to be run before the script is executed
 BaseCore::preRun should be called in overrriden function if overiding with own function
 @param script Script code;
 @param callback is it callback from script?
*/
void BaseCore::preRun(const QString &script,bool callback/*=false*/) {
 //Can be overridden to do some extra initialization
 if (callback) return;
 con->printCommand(script);
 clearError();//Clear error message
}

/**
 Script error invoked when script tried to access a NULL pointer
 @param className Name of class, in which this error occured
 @param methodName Name of method, in which this error occured
 */
void BaseCore::errorNullPointer(const QString &className,const QString &methodName) {
 QString where;
 if (className=="") where=methodName; else where=className+"."+methodName;
 qs->throwError(tr("Null pointer access in ")+where);
}

/**
 Script error invoked when script encounter an exception
 @param className Name of class, in which this exception occured
 @param methodName Name of method, in which this exception occured
 @param exceptionInfo Extra exception information
 */
void BaseCore::errorException(const QString &className,const QString &methodName,const QString &exceptionInfo) {
 QString where;
 if (className=="") where=methodName; else where=className+"."+methodName;
 qs->throwError(tr("Exception in ")+where+" : "+exceptionInfo);
}

/**
 Script error invoked when script function was executed with bad parameter
 @param className Name of class, in which this error occured
 @param methodName Name of method, in which this error occured
 @param paramNum number of bad parameter
 @param param Pointer to the QObject that was given as parameter
 @param expected What was expected to see as parameter
 */
void BaseCore::errorBadParameter(const QString &className,const QString &methodName,int paramNum,const QObject *param,const QString &expected) {
 QString where;
 if (className=="") where=methodName; else where=className+"."+methodName;
 QString invMessage=tr("Invalid object given for parameter %1 in ").arg(QString::number(paramNum))+where;
 QString passedObject=param->className();
 qs->throwError(invMessage+"\n"
               +tr("Expected","object type")+" : "+expected+"\n"
               +tr("Got","object type")+" : "+passedObject);
}

/**
 Add QSCObject to list of object to delete when file in editor window is closed
 @param o Object to add to cleanup-list
*/
void BaseCore::addGC(QSCObject *o) {
 assert(o);
 baseObjects.replace(o,o);
}

/**
 Remove QSCObject from list of object to delete when file in editor window is closed
 @param o Object to remove from cleanup-list
*/
void BaseCore::removeGC(QSCObject *o) {
 assert(o);
 baseObjects.remove(o);
}

/**
 Add tree item wrapper to list of managed wrappers<br>
 Called from treeitem wrapper contructor
 @param theWrap Tree item wrapper
*/
void BaseCore::addTreeItemToList(QSTreeItem* theWrap) {
 //Get wrapped item
 TreeItemAbstract* theItem=theWrap->get();

 //Get inner dictionary for the treeitem (dict of all wrappers for given treeitem)
 // Some wrappers may already exist for this treeitem
 Q_PtrDict<void>* pDict=treeWrap[theItem];

 if (!pDict) {	//No wrappers for this yet ....
  //We must create and insert inner dictionary for this tree item
  pDict=new Q_PtrDict<void>(7);
  //Smaller dict, typically there will be few wrappers to same item
  treeWrap.insert(theItem,pDict);
 }

 //Insert wrapper in inner dictionary
 pDict->insert(theWrap,theWrap);
 guiPrintDbg(debug::DBG_DBG,"Added wrapper   " << (intptr_t)theWrap << " w. item "<< (intptr_t)theItem);
}

/**
 Remove tree item wrapper from list of managed wrappers<br>
 Called from treeitem wrapper contructor
 @param theWrap Tree item wrapper
*/
void BaseCore::removeTreeItemFromList(QSTreeItem* theWrap) {
 //Get wrapped item
 TreeItemAbstract* theItem=theWrap->get();

 //Get inner dictionary for the treeitem (dict of all wrappers for given treeitem)
 Q_PtrDict<void>* pDict=treeWrap[theItem];
 assert (pDict);//Not in list? WTF?
 assert (pDict->find(theWrap));//Not in list? WTF?

 //remove from inner dictionary
 pDict->remove(theWrap);
 guiPrintDbg(debug::DBG_DBG,"Removed wrapper " << (intptr_t)theWrap << " w. item "<< (intptr_t)theItem);

 if (!pDict->count()) {
  //This was last wrapper - delete dictionary
  treeWrap.remove(theItem);
  //Autodelete is on, so the inner dictionary will be deleted ....
  guiPrintDbg(debug::DBG_DBG,"Removed LAST wrapper : " << (intptr_t)theWrap << " w. item "<< (intptr_t)theItem);
 }
}

/** Delete all objects in cleanup list */
void BaseCore::cleanup() {
 //Delete list of tree wrappers, just to be sure.
 //They'll all be deleted just moment later and at this moment the list should be empty anyway
 guiPrintDbg(debug::DBG_INFO,"Garbage collection: " << treeWrap.count() << " items in tree wrap");
 treeWrap.clear();//Delete also inner dicts...
 treeWrap.setAutoDelete(false);
 guiPrintDbg(debug::DBG_INFO,"Garbage collection: " << baseObjects.count() << " objects");
 //Set autodelete and clear the list

 Q_PtrDict<QSCObject> baseObjectsCopy=baseObjects;

 baseObjectsCopy.setAutoDelete(true);
 baseObjectsCopy.clear();
 baseObjectsCopy.setAutoDelete(false);
 guiPrintDbg(debug::DBG_INFO,"Garbage collection: " << baseObjects.count() << " objects (should be 0)");
 baseObjects.clear();
 guiPrintDbg(debug::DBG_DBG,"Garbage collection done");
}

/** destructor */
BaseCore::~BaseCore() {
 //Delete the CPDF wrapper if it is there
 if (qpdf) delete qpdf;
 //Delete importer
 delete import;
 //Delete the project (and also the interpreter)
 delete qp;
 //Delete any remaining object wrappers
 cleanup();
}

} //namespace gui
