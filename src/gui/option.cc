/** @file
Option - class for widget containing one editable setting
Can be subclassed for editing specific option types (integer, ...)
This class provides editing of strings, which is the type of all settings
*/

#include <utils/debug.h>
#include "option.h"

/** Default constructor of option item
 @param parent parent Option Editor containing this control
 @param _key Key in settings for this option
 */
Option::Option(const QString &_key/*=0*/,QWidget *parent/*=0*/)
 : QWidget (parent, "option"){
 key=_key;
 ed=new QLineEdit(this,"option_edit");
 set=Settings::getInstance();
 printDbg(debug::DBG_DBG,"+ Option " << key);
}

/** return name of this option */
QString Option::getName() {
 return key;
}

/** default destructor */
Option::~Option() {
 delete ed;
 printDbg(debug::DBG_DBG,"- Option" << key);
}

/** return size hint of this property editing control */
QSize Option::sizeHint() const {
 return ed->sizeHint();
}

/** Called on resizing of property editing control */
void Option::resizeEvent (QResizeEvent *e) {
 ed->setFixedSize(e->size());
}

/** write edited value to settings */
void Option::writeValue() {
 set->write(key,ed->text());
}
/** read value from settings for editing */
void Option::readValue() {
 ed->setText(set->read(key));
}
