/** @file
StringOption - class for widget containing one editable setting of type string
*/

#include <utils/debug.h>
#include "stringoption.h"

/** Default constructor of StringOption item
 @param parent parent Option Editor containing this control
 @param _key Key in settings for this option
 */
StringOption::StringOption(const QString &_key/*=0*/,QWidget *parent/*=0*/)
 : Option (_key,parent) {
 ed=new QLineEdit(this,"option_edit");
}

/** default destructor */
StringOption::~StringOption() {
 delete ed;
}

/** write edited value to settings */
void StringOption::writeValue() {
 set->write(key,ed->text());
}

/** read value from settings for editing */
void StringOption::readValue() {
 ed->setText(set->read(key));
}

/** return size hint of this property editing control */
QSize StringOption::sizeHint() const {
 return ed->sizeHint();
}

/** Called on resizing of property editing control */
void StringOption::resizeEvent (QResizeEvent *e) {
 ed->setFixedSize(e->size());
}
