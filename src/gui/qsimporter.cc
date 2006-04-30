/** @file
 QSImporter -> import QObjects from application without reevaluating project<br>
 Adding objects via addObject have disadvantage of clearing interpreter state
 (thus removing all functions loaded from initscript)
 Adding via addTransientObject disallow removing the object later.
 Can import any QObject into scripting layer under specified name and also can create
 QSCObjects from some common types (dict, page ..)
 @author Martin Petricek
*/

#include <utils/debug.h>
#include <qstring.h>
#include <qsproject.h>
#include <qsinterpreter.h>
#include "qsimporter.h"
#include "qsdict.h"
#include "qsiproperty.h"
#include "qspage.h"
#include "qscobject.h"
#include "qspdf.h"
#include "qstreeitem.h"
#include "util.h"

namespace gui {

/** Construct importer object for current QSProject to given context. Must be contructed before any scripts are evaluated */
QSImporter::QSImporter(QSProject *_qp,QObject *_context) {
 setName("importer");
 guiPrintDbg(debug::DBG_DBG,"Creating QSImporter");
 qp=_qp;
 context=_context;
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
 return new QSDict(dict);
}


/** Overloaded factory function to create QSCObjects from various C... classes
    Returns QSCObject that can be added directly with addQSObj()
 @param page CPage (shared_ptr) to wrap into to QSPage
 @return QSPage(page)
 */
QSCObject* QSImporter::createQSObject(boost::shared_ptr<CPage> page) {
 if (!page.get()) return NULL;
 return new QSPage(page);
}

/** Overloaded factory function to create QSCObjects from various C... classes
    Returns QSCObject that can be added directly with addQSObj()
 @param ip IProperty to wrap into to QSIProperty
 @return QSIProperty(ip)
 */
QSCObject* QSImporter::createQSObject(boost::shared_ptr<IProperty> ip) {
 if (!ip.get()) return NULL;
 return new QSIProperty(ip);
}

/** Overloaded factory function to create QSCObjects from various C... classes
    Returns QSCObject that can be added directly with addQSObj()
 @param pdf CPdf to wrap into to QSPdf
 @return QSPdf(pdf)
 */
QSPdf* QSImporter::createQSObject(CPdf* pdf) {
 if (!pdf) return NULL;
 return new QSPdf(pdf);
}

/** Overloaded factory function to create QSCObjects from various C... classes
    Returns QSCObject that can be added directly with addQSObj()
 @param item TreeItemAbstract to wrap into to QSTreeItem
 @return QSTreeItem(item)
 */
QSCObject* QSImporter::createQSObject(TreeItemAbstract *item) {
 if (!item) return NULL;
 return new QSTreeItem(item);
}

/** Import object into interpreter under specified name
 @param obj Object to import
 @param name Name of object in scripting
 */
void QSImporter::addQSObj(QObject *obj,const QString &name) {
 if (obj) obj->setName(name);//NULL can be imported too
 qobj=obj;
 if (obj) {
  guiPrintDbg(debug::DBG_DBG,"Importing " <<name);
 } else {
  guiPrintDbg(debug::DBG_DBG,"Importing NULL " <<name);
 }
 QString code=QString("var ")+name+"=importer.getQSObj();";
// guiPrintDbg(debug::DBG_DBG,"Importing with code:" <<code);
 qs->evaluate(code,context,"<qsimporter>");
 QString err=qs->errorMessage();
 if (!err.isNull()) {
  guiPrintDbg(debug::DBG_ERR,"Failed import: " <<err);
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

