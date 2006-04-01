#ifndef __QSCOBJECT_H__
#define __QSCOBJECT_H__

#include <qobject.h>

class QSCObject : public QObject {
Q_OBJECT
public:
 QSCObject();
 virtual ~QSCObject();
public slots:
 virtual void deleteSelf();
};

#endif
