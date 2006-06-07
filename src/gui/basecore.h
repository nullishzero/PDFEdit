#ifndef __BASECORE_H__
#define __BASECORE_H__

#include <qobject.h>
#include <qptrdict.h>

class QSProject;
class QSInterpreter;
class QString;

namespace gui {

class QSCObject;
class QSImporter;
class QSTreeItem;

/**
 Type containing binding between treeitem and its QSA wrapper.<br>
 Mapping is from TreeItemAbstract* to (QSTreeItem*)[]
*/
typedef QPtrDict<QPtrDict<void> > TreeBindingMap;

/**
 Iterator type for TreeBindingMap dictionary type
 \see TreeBindingMap
*/
typedef QPtrDictIterator<QSTreeItem> TreeBindingMapIterator;

/**
 Core of Class that host scripts and is responsible for garbage collection of scripting objects
 and core scriptiong functionality
*/
class BaseCore : public QObject {
 Q_OBJECT
public:
 BaseCore();
 ~BaseCore();
 void stopScript();
 void addGC(QSCObject *o);
 void removeGC(QSCObject *o);
 void errorNullPointer(const QString &className,const QString &methodName);
 void errorBadParameter(const QString &className,const QString &methodName,int paramNum,const QObject *param,const QString &expected);
 void addTreeItemToList(QSTreeItem* theWrap);
 void removeTreeItemFromList(QSTreeItem* theWrap);
 void cleanup();
 QSInterpreter* interpreter();
protected:
 void deleteVariable(const QString &varName);
protected:
 /** QSA Scripting Project */
 QSProject *qp;
 /** QSA Interpreter - taken from project */
 QSInterpreter *qs;
 /** All Scripting objects created under this base. Will be used for purpose of garbage collection */
 QPtrDict<QSCObject> baseObjects;
 /** QSObject Importer */
 QSImporter *import;
 /** map containing trees to disable if necessary*/
 TreeBindingMap treeWrap;//Warning - autodelete is on for this map ...
};

} //namespace gui

#endif
