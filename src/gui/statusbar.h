#ifndef __STATUSBAR_H__
#define __STATUSBAR_H__

#include <qstatusbar.h>

namespace gui {

/** Class representing statusbar */
class StatusBar : public QStatusBar {
 Q_OBJECT
public:
 StatusBar(QWidget *parent=0,const char *name=0);
 ~StatusBar();
};

} // namespace gui

#endif
