#ifndef __TOOLBUTTON_H__
#define __TOOLBUTTON_H__

#include <qtoolbutton.h>
class QPixmap;
class QString;

namespace gui {

/** Class wrapping QToolButton, adding button ID and providing different constructor (QPixmap instead of QIconSet)
 Also, emits clicked(id) signal in addition to normal clicked() signal
 */

class ToolButton : public QToolButton {
 Q_OBJECT
 public:
  ToolButton(QPixmap *icon, const QString tooltip, int id, QWidget *parent=0, const char *name="");
 protected slots:
  void slotClicked();
 private:
  int b_id;   
 signals:
  /** signal emitted when clicked on this button. The button sends ID of itself. */
  void clicked(int);
};

} // namespace gui

#endif
