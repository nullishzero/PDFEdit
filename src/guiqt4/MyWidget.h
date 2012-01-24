#ifndef __WIDGET__
#define __WIDGET__

#include <QWidget>

class MyWidget : public QWidget
{
﻿  Q_OBJECT
public:
﻿  MyWidget( QWidget * parent =0);
protected:
﻿  void keyPressEvent(QKeyEvent *event);
};

#endif