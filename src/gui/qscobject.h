#ifndef __QSCOBJECT_H__
#define __QSCOBJECT_H__

#include <qobject.h>
class QString;

namespace gui {

class Base;

/*= Base class for all PDF objects used in scripts */
class QSCObject : public QObject {
Q_OBJECT
public:
 QSCObject(QString _typeName, Base* _base);
 QSCObject(QSCObject &source);
 virtual ~QSCObject();
public slots:
 virtual void deleteSelf();
 /*- return name of this object's type */
 virtual QString type();
protected:
 void treeNeedReload();
 /** Scripting base for this object */
 Base *base;
private:
 /** Name of this objects's type */
 QString typeName;
};

} // namespace gui

#endif
