/** @file
 QSMenu - class providing Popup menu with functions to create and show these menus<br>
 Export some functions to scripting.<br>
 See doc/user/scripting.xml or kernel documentation for more informations about these functions
 @author Martin Petricek
*/

#include "qsmenu.h"

namespace gui {

/** Construct empty popup menu */
QSMenu::QSMenu(const QString &name) : QSCObject ("Menu") {
 //TODO
}

/** Invoke popup menu near position of mouse pointer. If any action is selected from menu, it is immediately executed */
void QSMenu::popup() {
 //TODO
}

/** Append item (or list) to menu using its definition
 @param def Definition of item (item or list, same format as used in application menus)*/
void QSMenu::addItem(const QString &def) {
 //TODO
}

/** Reset the menu and remove all items from it */
void QSMenu::reset() {
 //TODO
}

/** destructor */
QSMenu::~QSMenu() {
 //TODO
}

} // namespace gui

