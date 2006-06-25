#ifndef __TOOLFACTORY_H__
#define __TOOLFACTORY_H__

class QMainWindow;
class QString;

namespace gui {

class ToolBar;

/**
 Factory class for creating special toolbar items
 \brief Special Toolbar item factory
 */
class ToolFactory {
public:
 static bool specialItem(ToolBar *tb,const QString &item,QMainWindow *main);
};

} // namespace gui

#endif
