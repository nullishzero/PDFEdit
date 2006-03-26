/** @file
 QSImporter -> import QObjects from application without reevaluating project
 Adding objects via addObject have disadvantage of clearing interpreter state
 (thus removing all functions loaded from initscript)
 Adding via addTransientObject disallow removing the object later.
*/

#include <utils/debug.h>
#include "qsimporter.h"

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

