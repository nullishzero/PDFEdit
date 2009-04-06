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
 QSImporter -> import QObjects from application without reevaluating project<br>
 Adding objects via addObject have disadvantage of clearing interpreter state
 (thus removing all functions loaded from initscript)
 Adding via addTransientObject disallow removing the object later.
 Can import any QObject into scripting layer under specified name and also can create
 QSCObjects from some common types (dict, page ..)
 @author Martin Petricek
*/
#include "qsimporter.h"
#include "qtcompat.h"
#include "qsarray.h"
#include "qscobject.h"
#include "qsdict.h"
#include "qsiproperty.h"
#include "qspage.h"
#include "qspdf.h"
#include "qspdfoperator.h"
#include "qsstream.h"
#include "qstreeitem.h"
#include "qstreeitemcontentstream.h"
#include "treeitemcontentstream.h"
#include "util.h"
#include <qsinterpreter.h>
#include <qsproject.h>
#include <qstring.h>
#include <utils/debug.h>

namespace gui {

/**
 Construct importer object for current QSProject to given context. Must be contructed before any scripts are evaluated<br>
 @param _qp QSProject in which this importer is installed
 @param _context Context in which all objects will be imported
 @param _base scripting base under which all objects will be created
 */
QSImporter::QSImporter(QSProject *_qp,QObject *_context,BaseCore *_base) {
 setName("importer");
 guiPrintDbg(debug::DBG_DBG,"Creating QSImporter");
 qp=_qp;
 context=_context;
 base=_base;
 //add itself to Project
 qp->addObject(this);
 qs=qp->interpreter();
}

/** Overloaded factory function to create QSCObjects from various C... classes
    Returns QSCObject that can be added directly with addQSObj()
 @param dict CDict to wrap into to QSDict
 @return QSDict(dict)
 */
QSCObject* QSImporter::createQSObject(boost::shared_ptr<CDict> dict) {
 if (!dict.get()) return NULL;
 return new QSDict(dict,base);
}


/** Overloaded factory function to create QSCObjects from various C... classes
    Returns QSCObject that can be added directly with addQSObj()
 @param page CPage (shared_ptr) to wrap into to QSPage
 @return QSPage(page)
 */
QSCObject* QSImporter::createQSObject(boost::shared_ptr<CPage> page) {
 if (!page.get()) return NULL;
 return new QSPage(page,base);
}

/** Overloaded factory function to create QSCObjects from various C... classes
    Returns QSCObject that can be added directly with addQSObj()
 @param ip IProperty to wrap into to QSIProperty
 @return QSIProperty(ip)
 */
QSCObject* QSImporter::createQSObject(boost::shared_ptr<IProperty> ip) {
 return createQSObject(ip,base);
}

/** Overloaded factory function to create QSCObjects from various classes
    Returns QSCObject that can be added directly with addQSObj()
 @param op PdfOperator to wrap into to QSPdfOperator
 @return QSPdfOperator(op)
 */
QSCObject* QSImporter::createQSObject(boost::shared_ptr<PdfOperator> op) {
 return new QSPdfOperator(op,base);
}

/** Static version of factory function to create QSCObjects from various C... classes
    Returns QSCObject that can be added directly with addQSObj()
 @param ip IProperty to wrap into to QSIProperty
 @param _base Scripting base
 @return QSIProperty(ip)
 */
QSCObject* QSImporter::createQSObject(boost::shared_ptr<IProperty> ip,BaseCore *_base) {
 if (!ip.get()) return NULL;
 //Try if it is stream
 boost::shared_ptr<CStream> cs=boost::dynamic_pointer_cast<CStream>(ip);
 if (cs.get()) return new QSStream(cs,_base);
 //Try if it is dict
 boost::shared_ptr<CDict> cd=boost::dynamic_pointer_cast<CDict>(ip);
 if (cd.get()) return new QSDict(cd,_base);
 //Try if it is array
 boost::shared_ptr<CArray> ca=boost::dynamic_pointer_cast<CArray>(ip);
 if (ca.get()) return new QSArray(ca,_base);
 //Nothing - just plain IProperty
 return new QSIProperty(ip,_base);
}

/**
 Overloaded factory function to create QSCObjects from various C... classes
 Returns QSCObject that can be added directly with addQSObj()
 @param pdf CPdf to wrap into to QSPdf
 @return QSPdf(pdf)
 */
QSPdf* QSImporter::createQSObject(boost::shared_ptr<CPdf> pdf) {
 if (!pdf) return NULL;
 return new QSPdf(pdf,base);
}

/**
 Static version of factory function to create QSCObjects from various C... classes
 Returns QSCObject that can be added directly with addQSObj()
 @param item TreeItemAbstract to wrap into to QSTreeItem
 @param _base Scripting base
 @return QSTreeItem(item)
*/
QSCObject* QSImporter::createQSObject(TreeItemAbstract *item,BaseCore *_base) {
 if (!item) return NULL;
 TreeItemContentStream* tiCS=dynamic_cast<TreeItemContentStream*>(item);
 if (tiCS) {
  return new QSTreeItemContentStream(tiCS,_base);
 }
 return new QSTreeItem(item,_base);
}

/**
 Overloaded factory function to create QSCObjects from various C... classes
 Returns QSCObject that can be added directly with addQSObj()
 @param item TreeItemAbstract to wrap into to QSTreeItem
 @return QSTreeItem(item)
*/
QSCObject* QSImporter::createQSObject(TreeItemAbstract *item) {
 return createQSObject(item,base);
}

/** Import object into interpreter under specified name
 @param obj Object to import
 @param name Name of object in scripting
 */
void QSImporter::addQSObj(QObject *obj,const QString &name) {
 if (obj) obj->setName(name);//NULL can be imported too
 qobj=obj;
 if (obj) {
  guiPrintDbg(debug::DBG_DBG,"Importing " << Q_OUT(name));
 } else {
  guiPrintDbg(debug::DBG_DBG,"Importing NULL " << Q_OUT(name));
 }
 QString code=QString("var ")+name+"=importer.getQSObj();";
// guiPrintDbg(debug::DBG_DBG,"Importing with code:" <<code);
 qs->evaluate(code,context,"<qsimporter>");
 QString err=qs->errorMessage();
 if (!err.isNull()) {
  guiPrintDbg(debug::DBG_ERR,"Failed import: " << Q_OUT(err));
 }
 qobj=NULL;
}

/** Internal importing function. Do not use outside this class.
 @return Imported object */
QObject* QSImporter::getQSObj() {
 return qobj;
}

/** destructor */
QSImporter::~QSImporter() {
 //remove itself from project
 if (qp) qp->removeObject(this);
 guiPrintDbg(debug::DBG_DBG,"Destroying QSImporter");
}

} // namespace gui

