#ifndef __QSIMPORTER_H__
#define __QSIMPORTER_H__

#include <qobject.h>
#include <cobject.h>
class QSProject;
class QSInterpreter;
class QString;
namespace pdfobjects {
 class CPage;
 class PdfOperator;
 class CPdf;
}

namespace gui {

class QSCObject;
class QSPdf;
class TreeItemAbstract;
class BaseCore;

using namespace pdfobjects;

/**
 QSImporter -> import QObjects from application without re-evaluating project<br>
 Adding objects via addObject have disadvantage of clearing interpreter state
 (thus removing all functions loaded from initscript)
 Adding via addTransientObject disallow removing the object later.
 Can import any QObject into scripting layer under specified name and also can create
 QSCObjects from some common types (dict, page ..)
 \brief QObject importer into scripting
*/
class QSImporter : public QObject {
 Q_OBJECT
public:
 virtual ~QSImporter();
 void addQSObj(QObject *obj,const QString &name);
 //factory-style functions
 static QSCObject* createQSObject(boost::shared_ptr<IProperty> ip,BaseCore *_base);
 static QSCObject* createQSObject(TreeItemAbstract *item,BaseCore *_base);
 QSCObject* createQSObject(boost::shared_ptr<PdfOperator> op);
 QSCObject* createQSObject(boost::shared_ptr<IProperty> ip);
 QSCObject* createQSObject(boost::shared_ptr<CDict> dict);
 QSCObject* createQSObject(boost::shared_ptr<CPage> page);
 QSCObject* createQSObject(TreeItemAbstract *item);
 QSPdf* createQSObject(CPdf* pdf);
 QSImporter(QSProject *_qp,QObject *_context,BaseCore *_base);
public slots:
 QObject* getQSObj();
private:
 /** Current QObject to import */
 QObject *qobj;
 /** context in which objects will be imported */
 QObject *context;
 /** Interpreter from QProject qp. All objects will be imported in it. */
 QSInterpreter *qs;
 /** QSProject in which this importer is installed. */
 QSProject *qp;
 /** Scripting base for created objects */
 BaseCore *base;
};

} // namespace gui

#endif
