#ifndef __QSIMPORTER_H__
#define __QSIMPORTER_H__

#include <qobject.h>
#include <cobject.h>
#include <cpage.h>
#include <cpdf.h>
#include <pdfoperators.h>
class QSProject;
class QSInterpreter;
class QString;

namespace gui {

class QSCObject;
class QSPdf;
class TreeItemAbstract;
class Base;

using namespace pdfobjects;

class QSImporter : public QObject {
 Q_OBJECT
public:
 virtual ~QSImporter();
 void addQSObj(QObject *obj,const QString &name);
 //factory-style functions
 static QSCObject* createQSObject(boost::shared_ptr<IProperty> ip,Base *_base);
 static QSCObject* createQSObject(TreeItemAbstract *item,Base *_base);
 QSCObject* createQSObject(boost::shared_ptr<pdfobjects::PdfOperator> op);
 QSCObject* createQSObject(boost::shared_ptr<IProperty> ip);
 QSCObject* createQSObject(boost::shared_ptr<CDict> dict);
 QSCObject* createQSObject(boost::shared_ptr<CPage> page);
 QSCObject* createQSObject(TreeItemAbstract *item);
 QSPdf* createQSObject(CPdf* pdf);
 QSImporter(QSProject *_qp,QObject *_context,Base *_base);
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
 Base *base;
};

} // namespace gui

#endif
