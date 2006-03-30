/** @file
 QSImporter -> import QObjects from application without reevaluating project
 Adding objects via addObject have disadvantage of clearing interpreter state
 (thus removing all functions loaded from initscript)
 Adding via addTransientObject disallow removing the object later.
*/

#include <utils/debug.h>
#include "qsimporter.h"
#include "qsdict.h"
#include "qspage.h"
#include "qspdf.h"

/** Construct importer object for current QSProject to given context. Must be contructed before any scripts are evaluated */
QSImporter::QSImporter(QSProject *_qp,QObject *_context) {
 setName("importer");
 printDbg(debug::DBG_DBG,"Creating QSImporter");
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
QSCObject* QSImporter::createQSObject(CDict* dict) {
 return new QSDict(dict);
}


/** Overloaded factory function to create QSCObjects from various C... classes
    Returns QSCObject that can be added directly with addQSObj()
 @param page CPage (shared_ptr) to wrap into to QSPage
 @return QSPage(page)
 */
QSCObject* QSImporter::createQSObject(boost::shared_ptr<CPage> page) {
 return new QSPage(page);
}

/** Overloaded factory function to create QSCObjects from various C... classes
    Returns QSCObject that can be added directly with addQSObj()
 @param pdf CPdf to wrap into to QSPdf
 @return QSPdf(pdf)
 */
QSCObject* QSImporter::createQSObject(CPdf* pdf) {
 return new QSPdf(pdf);
}

/** Import object into interpreter under specified name
 @param obj Object to import
 @param name Name of object in scripting
 */
void QSImporter::addQSObj(QObject *obj,const QString &name) {
 obj->setName(name);
 qobj=obj;
 printDbg(debug::DBG_DBG,"Importing " <<name);
 QString code=QString("var ")+name+"=importer.getQSObj();";
// printDbg(debug::DBG_DBG,"Importing with code:" <<code);
 qs->evaluate(code,context,"qsimporter");
 QString err=qs->errorMessage();
 if (!err.isNull()) {
  printDbg(debug::DBG_ERR,"Failed import: " <<err);
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
 printDbg(debug::DBG_DBG,"Destroying QSImporter");
}

