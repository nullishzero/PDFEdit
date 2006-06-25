/** @file
 ToolBar class, ancestor of QToolBar.
 This toolbar can handle only buttons derived from ToolButton class
 (any generic QWidget can be inserted, but the widget must then handle clicks, etc ... by itself)
 @author Martin Petricek
*/

#include "toolbar.h"
#include "toolbutton.h"
#include <qstring.h>

namespace gui {

/**
 Same constructor as QToolBar
 @param label Caption of toolbar
 @param mainWindow Main window in which toolbar will be managed
 */
ToolBar::ToolBar(const QString &label,QMainWindow *mainWindow) : QToolBar(label,mainWindow) {
 //Empty constructor
}

/**
 Same constructor as QToolBar
 @param label Name of toolbar
 @param mainWindow Main window in which toolbar will be managed
 @param parent Parent window containing toolbar
 @param newLine Dock toolbar in a new line? (default is false = dock in same line)
 @param name Parameter passed to QDockWindow
 @param f Parameter passed to QDockWindow
 */
ToolBar::ToolBar(const QString &label,QMainWindow *mainWindow,QWidget *parent,bool newLine/*=FALSE*/,const char * name/*=0*/,WFlags f/*=0*/)
 : QToolBar(label,mainWindow,parent,newLine,name,f) {
 //Empty constructor
} 

/**
 Same constructor as QToolBar
 @param parent Main window in which toolbar will be managed
 @param name Name of toolbar (for debugging)
 */
ToolBar::ToolBar(QMainWindow *parent,const char *name/*=0*/) : QToolBar(parent,name) {
 //Empty constructor
}

/**
 Add button to toolbar and link slots, so clicks from buttons are passed along with button ID to the application
 @param qb Button to add to this toolbar 
 */
void ToolBar::addButton(ToolButton *qb) {
 QObject::connect(qb,SIGNAL(clicked(int)),this,SLOT(slotClicked(int)));
 QObject::connect(qb,SIGNAL(helpText(const QString&)),this,SLOT(receiveHelpText(const QString&)));
}

/**
 slot that will emit clicked with ID of button
 @param id Id of button
*/
void ToolBar::slotClicked(int id) {
 emit itemClicked(id);
}

/** 
 Signal called when receiving help message.
 Forward the message via helpText
 @param message Help message
*/
void ToolBar::receiveHelpText(const QString &message) {
 emit helpText(message);
}

} // namespace gui
