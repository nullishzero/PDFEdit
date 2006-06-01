#ifndef __PropertyModeController_H__
#define __PropertyModeController_H__

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
