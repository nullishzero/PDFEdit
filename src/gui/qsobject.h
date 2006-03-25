#ifndef __QSOBJECT_H__
#define __QSOBJECT_H__

#include <qobject.h>

template <typename T> class QSObject : public QObject {
public:
 QSObject(T *_obj);
 T *QSObject::get();
 virtual ~QSObject();
protected:
 /** Object held in class*/
 T *obj;
};

#endif
