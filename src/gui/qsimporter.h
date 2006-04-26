#ifndef __QSIMPORTER_H__
#define __QSIMPORTER_H__

#include <qobject.h>
#include <cobject.h>
#include <cpage.h>
#include <cpdf.h>
class QSProject;
class QSInterpreter;
class QString;

namespace gui {

class QSCObject;
class QSPdf;

using namespace pdfobjects;

class QSImporter : public QObject {
 Q_OBJECT
public:
 QSImporter(QSProject *_qp,QObject *_context);
 virtual ~QSImporter();
 void addQSObj(QObject *obj,const QString &name);
 //factory-style functions
 QSCObject* createQSObject(boost::shared_ptr<IProperty> ip);
 QSCObject* createQSObject(boost::shared_ptr<CDict> dict);
 QSCObject* createQSObject(boost::shared_ptr<CPage> page);
 QSPdf* createQSObject(CPdf* pdf);
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
};

} // namespace gui

#endif
