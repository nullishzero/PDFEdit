#ifndef __TOOLFACTORY_H__
#define __TOOLFACTORY_H__

class QMainWindow;
class QString;
class QWidget;

namespace gui {

class ToolBar;

/**
 Factory class for creating special toolbar items
 \brief Special Toolbar item factory
 */
class ToolFactory {
public:
 static QWidget *specialItem(ToolBar *tb,const QString &item,QMainWindow *main);
};

} // namespace gui

#endif
