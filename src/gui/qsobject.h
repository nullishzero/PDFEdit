#ifndef __QSOBJECT_H__
#define __QSOBJECT_H__

#include <qobject.h>

class QSObject : public QObject {
Q_OBJECT
public:
 QSObject();
 virtual ~QSObject();
};

#endif
