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
#ifndef __BASECORE_H__
#define __BASECORE_H__

#include <qobject.h>
#include <boost/shared_ptr.hpp>
#include "qtcompat.h"
#include QPTRDICT
class QSProject;
class QSInterpreter;
class QString;
namespace pdfobjects {
 class CPdf;
}

namespace gui {

class ConsoleWriter;
class QSCObject;
class QSImporter;
class QSTreeItem;
class QSPdf;

/**
 Type containing binding between treeitem and its QSA wrapper.<br>
 Mapping is from TreeItemAbstract* to (QSTreeItem*)[]
*/
typedef Q_PtrDict<Q_PtrDict<void> > TreeBindingMap;

/**
 Iterator type for TreeBindingMap dictionary type
 \see TreeBindingMap
*/
typedef Q_PtrDictIterator<QSTreeItem> TreeBindingMapIterator;

/**
 Core of Class that host scripts and is responsible for garbage collection of scripting objects
 and core scriptiong functionality
 \brief Core scripting functions
*/
class BaseCore : public QObject {
 Q_OBJECT
public:
 BaseCore();
 virtual ~BaseCore();
 void setConWriter(ConsoleWriter *_con);
 void conPrintError(const QString &line);
 void conPrintLine(const QString &line);
 void runScript(const QString &script);
 void call(const QString &name,const QString &arguments="");
 void importDocument(boost::shared_ptr<pdfobjects::CPdf> pdf);
 void destroyDocument();
 QSPdf* getQSPdf() const;
 void stopScript();
 void addGC(QSCObject *o);
 void removeGC(QSCObject *o);
 void errorNullPointer(const QString &className,const QString &methodName);
 void errorBadParameter(const QString &className,const QString &methodName,int paramNum,const QObject *param,const QString &expected);
 void errorException(const QString &className,const QString &methodName,const QString &exceptionInfo);
 void addTreeItemToList(QSTreeItem* theWrap);
 void removeTreeItemFromList(QSTreeItem* theWrap);
 QSInterpreter* interpreter();

protected:
 //Override these two to to some extra/less initialization/finalization in script
 virtual void preRun(const QString &script,bool callback=false);
 virtual void postRun();
 //Override these two to add more/less objects to the script
 virtual void removeScriptingObjects();
 virtual void addScriptingObjects();
 void deleteVariable(const QString &varName);
 void errorMessage();
 void clearError();
private slots:
 void scriptError(const QString &message,const QString &scriptName,int lineNumber);
private:
 void cleanup();
protected:
 /** QSA Scripting Project */
 QSProject *qp;
 /** QSA Interpreter - taken from project */
 QSInterpreter *qs;
 /** QSObject Importer */
 QSImporter *import;
 /** map containing trees to disable if necessary*/
 TreeBindingMap treeWrap;//Warning - autodelete is on for this map ...
 /** QObject wrapper around CPdf (document) that is exposed to scripting. Lifetime of this class is the same as lifetime of document */
 QSPdf *qpdf;
 /** Flag specifying if the tree have changed while running script to the degree it need to be reloaded */
 bool treeReloadFlag;
private:
 /** All Scripting objects created under this base. Will be used for purpose of garbage collection */
 Q_PtrDict<QSCObject> baseObjects;
 /** Console writer handler */
 ConsoleWriter *con;
 /** Last-seen error message from script */
 QString errMessage;
 /** Script filename for last-seen error message */
 QString errScript;
 /** Script line number for last-seen error message */
 int errLineNumber;
};

} //namespace gui

#endif
