/** @file
 StatusBar - class representing Status bar
 @author Martin Petricek
*/
#include "statusbar.h"

namespace gui {

using namespace std;

/**
 Constructor of StatusBar
 @param parent Parent window of this dialog
 @param name Name of this window (used only for debugging
*/
StatusBar::StatusBar(QWidget *parent/*=0*/,const char *name/*=0*/):QStatusBar(parent,name) {
 //TODO: add some default widgets, add progressbar
}

/** default destructor */
StatusBar::~StatusBar() {
}

} // namespace gui
