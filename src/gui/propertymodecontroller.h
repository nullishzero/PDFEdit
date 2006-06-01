#ifndef __PROPERTYMODECONTROLLER_H__
#define __PROPERTYMODECONTROLLER_H__

namespace gui {

class PropertyModeController {
public:
 static PropertyModeController* getInstance();
 ~PropertyModeController();
private:
 PropertyModeController();
};

} // namespace gui

#endif
