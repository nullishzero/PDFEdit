#ifndef __TOOLBAR_H__
#define __TOOLBAR_H__
#include <qtoolbar.h>
#include "toolbutton.h"


/** Class wrapping QToolBar, providing "click forwarding" from TButtons.
 Buttons must be added with addButton() for this to work properly
 */

class ToolBar : public QToolBar {
 Q_OBJECT
 public:
  ToolBar(const QString & label,QMainWindow * mainWindow);
  ToolBar(const QString & label,QMainWindow * mainWindow,QWidget * parent,bool newLine = FALSE,const char * name = 0,WFlags f = 0 );  
  ToolBar(QMainWindow * parent = 0,const char * name = 0);  
  void addButton(ToolButton *qb);
 signals:
  void itemClicked(int);
 public slots:
  void slotClicked(int c);
};

#endif
