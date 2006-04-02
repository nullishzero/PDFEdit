#ifndef __QSCOBJECT_H__
#define __QSCOBJECT_H__

#include <qobject.h>
#include <qstring.h>

class QSCObject : public QObject {
Q_OBJECT
public:
 QSCObject(QString _typeName);
 virtual ~QSCObject();
public slots:
 virtual void deleteSelf();
 virtual QString type();
private:
 /** Name of this objects's type */
 QString typeName;
};

#endif
