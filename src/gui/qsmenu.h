#ifndef __QSMENU_H__
#define __QSMENU_H__

#include <qobject.h>
#include "qscobject.h"
#include <qptrlist.h> 
#include <qstringlist.h> 

class QPopupMenu;
class QMenuData;
class QString;

namespace gui {

class Menu;

/*=
 This type of object represent one popup menu.
 You can add menu items and submenus to it and you can execute the menu,
 which will then return the action picked by user (if any)
 To create new popup menu object, use the <link linkend="popupMenu">popupMenu</link> function
*/
/**
 Class providing Popup menu functionality to script with functions to create and show these menus
 \brief QObject wrapper around popup menu
 */
class QSMenu : public QSCObject {
 Q_OBJECT
public:
 QSMenu(Menu *_msys,BaseCore *_base,const QString &name=QString::null);
 QString getAction(int id);
 virtual ~QSMenu();
public slots:
 /*-
  Invoke popup menu near position of mouse pointer.
  Return script for action selected in menu
 */
 QString popup();
 /*- Reset the menu, removing all items from it */
 void reset();
 /*- Append item (only item, not a list) to menu using its definition */
 void addItemDef(QString def);
 /*- Append item or submenu (list) to menu using its name (it will be loaded from configuration) */
 void addItem(const QString &name);
 /*- Add separator to the menu */
 void addSeparator();
private:
 /** Menu held in class*/
 QPopupMenu *menu;
 /** Menu system of window in which this popup menu will be shown */
 Menu *msys;
 /** last used action ID */
 int actionId;
 /** List of menu actions */
 QStringList actions;
};

} // namespace gui

#endif
