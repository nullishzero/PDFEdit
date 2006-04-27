#ifndef __QSCOBJECT_H__
#define __QSCOBJECT_H__

#include <qobject.h>
class QString;

namespace gui {

/*= Base class for all PDF objects used in scripts */
class QSCObject : public QObject {
Q_OBJECT
public:
 QSCObject(QString _typeName);
 QSCObject(QSCObject &source);
 virtual ~QSCObject();
public slots:
 virtual void deleteSelf();
 /*- return name of this object's type */
 virtual QString type();
private:
 /** Name of this objects's type */
 QString typeName;
};

} // namespace gui

#endif
