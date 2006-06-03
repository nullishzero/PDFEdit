#ifndef __SELFDESTRUCTIVEWIDGET_H__
#define __SELFDESTRUCTIVEWIDGET_H__

#include <qwidget.h>

namespace gui {

/**
 Widget class that will destruct
 when appropriate signal from killer window is sent<br>
 Killer window can be any window, but must emit signal
 "selfDestruct()" for this widget to work<br>
 Used for helper dialogs that should close themselves
 when file/item they are editing is also closed<br>
*/
class SelfDestructiveWidget : public QWidget {
Q_OBJECT
public:
 SelfDestructiveWidget(QWidget *killer,QWidget *parent=0, const char *name=0, WFlags f=0);
 ~SelfDestructiveWidget();
};

} // namespace gui

#endif
