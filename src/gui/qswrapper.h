/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko,
 *                                              Miroslav Jahoda,
 *                                              Jozef Misutka,
 *                                              Martin Petricek
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit
 */
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
