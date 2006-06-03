#ifndef __PROPERTYMODECONTROLLER_H__
#define __PROPERTYMODECONTROLLER_H__

#include <modecontroller.h>

class QString;

namespace gui {

/**
 PropertyModeController - class providing wrapper around single ModeController
 instance and managing its configuration
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
