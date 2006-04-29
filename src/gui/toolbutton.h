#ifndef __TOOLBUTTON_H__
#define __TOOLBUTTON_H__

#include <qtoolbutton.h>
class QPixmap;
class QString;
class QIconSet;

namespace gui {

/**
 Class wrapping QToolButton, adding button ID and providing also
 a different constructors (with QPixmap or QIconSet)<br>
 Also, emits clicked(id) signal in addition to normal clicked() signal
 */
class ToolButton : public QToolButton {
 Q_OBJECT
public:
 ToolButton(const QIconSet *iconSet, const QString tooltip, int id, QWidget *parent=0, const char *name="");
 ToolButton(const QPixmap *icon, const QString tooltip, int id, QWidget *parent=0, const char *name="");
protected slots:
 void slotClicked();
private:
 /** ID number of this button */
 int b_id;   
signals:
 /** signal emitted when clicked on this button. The button sends ID of itself. */
 void clicked(int);
};

} // namespace gui

#endif
