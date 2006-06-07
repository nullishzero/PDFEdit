/** @file
 Base Core - class that host scripts
 This class is also responsible for garbage collection of scripting objects
 @author Martin Petricek
*/

#include "basecore.h"
#include "qscobject.h"
#include "qsimporter.h"
#include "qstreeitem.h"
#include "settings.h"
#include <qsinterpreter.h>
#include <qstring.h>
#include <qsutilfactory.h> 
#include <qsinputdialogfactory.h>
#include <utils/debug.h>

namespace gui {

/**
 Create new BaseCore class 
*/
BaseCore::BaseCore() {
 //Create new interpreter and project
 qp=new QSProject(this,"qs_project");
 qs=qp->interpreter();

 //Add ability to open files, directories and run processes
 qs->addObjectFactory(new QSUtilFactory());
 //Add ability to create dialogs
 qs->addObjectFactory(new QSInputDialogFactory());

 //Add settings object
 assert(globalSettings);
 qp->addObject(globalSettings);
 //Create and add importer to QSProject and related QSInterpreter
 import=new QSImporter(qp,this,this);
}

/**
 Return interpreter instance used to launch scripts in this context
 @return current QSInterpreter
*/
QSInterpreter* BaseCore::interpreter() {
 return qs;
}

/**
 "Remove" defined variable from scripting context
 @param varName name of variable;
*/
void BaseCore::deleteVariable(const QString &varName) {
 qs->setErrorMode(QSInterpreter::Nothing);
 qs->evaluate(varName+"=undefined;",this,"<delete_item>");
}

/** if any script is running, stop it */
void BaseCore::stopScript() {
 if (qs->isRunning()) {
  qs->stopExecution();
 }
}

/**
 Script error invoked when script tried to access a NULL pointer
 @param className Name of class, in which this error occured
 @param methodName Name of method, in which this error occured
 */
void BaseCore::errorNullPointer(const QString &className,const QString &methodName) {
 qs->throwError(tr("Null pointer access in ")+className+"."+methodName);
}

/**
 Script error invoked when script function was executed with bad parameter
 @param className Name of class, in which this error occured
 @param methodName Name of method, in which this error occured
 @param paramNum(number of bad parameter)
 @param param Pointer to the QObject that was given as parameter
 @param expected What was expected to see as parameter
 */
void BaseCore::errorBadParameter(const QString &className,const QString &methodName,int paramNum,const QObject *param,const QString &expected) {
 QString invMessage=tr("Invalid object given for parameter %1 in ").arg(QString::number(paramNum))+className+"."+methodName;
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
 QPtrDict<void>* pDict=treeWrap[theItem];

 if (!pDict) {	//No wrappers for this yet ....
  //We must create and insert inner dictionary for this tree item
  pDict=new QPtrDict<void>(7);
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
 QPtrDict<void>* pDict=treeWrap[theItem];
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
 baseObjects.setAutoDelete(true);
 baseObjects.clear();
 baseObjects.setAutoDelete(false);
 guiPrintDbg(debug::DBG_DBG,"Garbage collection done");
}

/** destructor */
BaseCore::~BaseCore() {
 //Delete importer
 delete import;
 //Delete the project (and also the interpreter)
 delete qp;
 //Delete any remaining object wrappers
 cleanup();
}

} //namespace gui
