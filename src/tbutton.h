#ifndef __TBUTTON_H__
#define __TBUTTON_H__
#include <qtoolbutton.h>
#include <qpixmap.h>
#include <qstring.h>

/** Class wrapping QToolButton, providing different constructor (QPixmap instead of QIconSet)*/

class ToolButton : public QToolButton {
 Q_OBJECT
 public:
  ToolButton(QPixmap *icon, const QString tooltip, int id, QWidget *parent=0, const char *name="");
 private:
  int b_id;   
 signals:
  void clicked(int);
 protected slots:
  void slotClicked();
};

#endif
