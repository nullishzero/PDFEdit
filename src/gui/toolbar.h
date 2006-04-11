#ifndef __TOOLBAR_H__
#define __TOOLBAR_H__
#include <qtoolbar.h>
#include "toolbutton.h"

namespace gui {

/** Class wrapping QToolBar, providing "click forwarding" from TButtons.
 Buttons must be added with addButton() for this to work properly
 */

class ToolBar : public QToolBar {
 Q_OBJECT
 public:
  ToolBar(const QString & label,QMainWindow * mainWindow);
  ToolBar(const QString & label,QMainWindow * mainWindow,QWidget * parent,bool newLine=FALSE,const char * name=0,WFlags f=0);
  ToolBar(QMainWindow * parent,const char * name=0);
  void addButton(ToolButton *qb);
 signals:
  /** signal emitted when clicked on one of toolbar items. Send ID number associated with that item */
  void itemClicked(int);
 public slots:
  void slotClicked(int id);
};

} // namespace gui

#endif
