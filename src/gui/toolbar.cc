/** @file
 ToolBar class, ancestor of QToolBar.
 This toolbar can contain only buttons from ToolButton class
*/

#include "toolbar.h"

/** Same constructor as QToolBar */
ToolBar::ToolBar(const QString & label,QMainWindow * mainWindow) : QToolBar(label,mainWindow) {
}

/** Same constructor as QToolBar */
ToolBar::ToolBar(const QString & label,QMainWindow * mainWindow,QWidget * parent,bool newLine/*=FALSE*/,const char * name/*=0*/,WFlags f/*=0*/)
 : QToolBar(label,mainWindow,parent,newLine,name,f) {
} 

/** Same constructor as QToolBar */
ToolBar::ToolBar(QMainWindow * parent,const char * name/*=0*/) : QToolBar(parent,name) {
 }


/** Add button to toolbar and link slots, so clicks from buttons are passed along with button ID to the application
 @param qb Button to add to this toolbar 
 */
void ToolBar::addButton(ToolButton *qb) {
 QObject::connect(qb, SIGNAL(clicked(int)), this, SLOT(slotClicked(int)));
}

/** slot that will emit clicked with ID of whis button */
void ToolBar::slotClicked(int id) {
 emit itemClicked(id);
}
