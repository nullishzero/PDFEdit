/** @file
BoolOption - class for widget containing one editable setting of type bool
*/

#include "booloption.h"

/** Default constructor of BoolOption item
 @param parent parent Option Editor containing this control
 @param _key Key in settings for this option
 */
BoolOption::BoolOption(const QString &_key/*=0*/,QWidget *parent/*=0*/)
 : Option (_key,parent) {
 ed=new QCheckBox(this,"booloption_checkbox");
}

/** default destructor */
BoolOption::~BoolOption() {
 delete ed;
}

/** return size hint of this control */
QSize BoolOption::sizeHint() const {
 return ed->sizeHint();
}

/** Called on resizing of this option editing control */
void BoolOption::resizeEvent (QResizeEvent *e) {
 ed->setFixedSize(e->size());
}

/** write edited value to settings */
void BoolOption::writeValue() {
 bool val=ed->isChecked();
 set->write(key,val?"1":"0");
}

/** read value from settings for editing */
void BoolOption::readValue() {
 bool val=set->readBool(key);
 ed->setChecked(val);
}

