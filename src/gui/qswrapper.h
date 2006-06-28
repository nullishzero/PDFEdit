#ifndef __QSWRAPPER_H__
#define __QSWRAPPER_H__

#include <qobject.h>
#include <qswrapperfactory.h>

namespace gui {

/**
 Wrapper factory that will return the object itself, thus handling its deallocation to QSA
 \brief QObject Wrapper factory
 */
class QSWrapper : public QSWrapperFactory {
public:
 QSWrapper();
 virtual ~QSWrapper();
 virtual QObject* create(const QString &className, void *ptr);
};

} // namespace gui

#endif
