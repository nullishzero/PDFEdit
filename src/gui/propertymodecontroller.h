/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko,
 *                                              Miroslav Jahoda,
 *                                              Jozef Misutka,
 *                                              Martin Petricek
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit
 */
#ifndef __PROPERTYMODECONTROLLER_H__
#define __PROPERTYMODECONTROLLER_H__

#include <kernel/modecontroller.h>

class QString;

namespace gui {

using namespace configuration;

/**
 PropertyModeController - class providing wrapper around single ModeController
 instance and managing its configuration
 \brief Mode controller wrapper
*/
class PropertyModeController {
public:
 ModeController* get();
 static PropertyModeController* getInstance();
 PropertyMode mode(const QString &type,const QString &name);
 ~PropertyModeController();
private:
 PropertyModeController();
};

} // namespace gui

#endif
