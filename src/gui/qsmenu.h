#ifndef __QSMENU_H__
#define __QSMENU_H__

#include <qobject.h>
#include "qscobject.h"

namespace gui {
/*=
 This type of object represent one popup menu.
 You can add menu items and submenus to it and you can execute the menu,
 which will then execute the action picked by user (if any)
*/
class QSMenu : public QSCObject {
 Q_OBJECT
public:
 QSMenu(const QString &name);
 /*-
  Invoke popup menu near position of mouse pointer.
  If any action is selected from menu, it is immediately executed
 */
 void popup();
 /*- Reset the menu and remove all items from it */
 void reset();
 /*- Append item or submenu (list) to menu using its definition */
 void addItem(const QString &def);
 virtual ~QSMenu();
private:
 /** Menu held in class*/
 //TODO
};

} // namespace gui

#endif
