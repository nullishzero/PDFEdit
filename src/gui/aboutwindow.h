#ifndef __ABOUTWINDOW_H__
#define __ABOUTWINDOW_H__

#include <qwidget.h>

namespace gui {

/**
 Class representing about window.<br>
 Display authors, application name and version.
*/
class AboutWindow : public QWidget {
Q_OBJECT
public:
 AboutWindow(QWidget *parent=0,const char *name=0);
 ~AboutWindow();
protected:
 void closeEvent(QCloseEvent *e);
};

} // namespace gui

#endif
