/** @file
 FontOption - class for widget containing one editable string setting,
 while allowing to either type (allowing things like copy/paste) the font
 description (name), or use dialog to pick it. User's choice
 @author Martin Petricek
*/

#include "fontoption.h"
#include <qstring.h>
#include <qfontdialog.h> 

namespace gui {

/**
 Default constructor of FontOption item
 @param parent parent Option Editor containing this control
 @param _key Key in settings for this option
 @param defValue Default value if option not found in configuration
 */
FontOption::FontOption(const QString &_key/*=0*/,QWidget *parent/*=0*/,const QString &defValue/*=0*/)
 : DialogOption (_key,parent,defValue) {
}

//See DialogOption for description of this abstract method
void FontOption::dialog(QString &value) {
 QFont initial;
 initial.fromString(value);
 bool ok;
 QFont ret=QFontDialog::getFont(&ok,initial,this,"font_dialog");
 if (ok) { //Some font picked
  // font is set to the font the user selected
  value=ret.toString();
 }
}

/** default destructor */
FontOption::~FontOption() {
}

} // namespace gui
