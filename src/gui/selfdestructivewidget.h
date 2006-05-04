#ifndef __SELFDESTRUCTIVEWIDGET_H__
#define __SELFDESTRUCTIVEWIDGET_H__

#include <qwidget.h>

namespace gui {

class SelfDestructiveWidget : public QWidget {
Q_OBJECT
public:
 SelfDestructiveWidget(QWidget *killer,QWidget *parent=0, const char *name=0, WFlags f=0);
 ~SelfDestructiveWidget();
};

} // namespace gui

#endif
