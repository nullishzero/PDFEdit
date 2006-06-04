/** @file
 BoolOption - class for widget containing one editable setting of type bool
 @author Martin Petricek
*/

#include "booloption.h"
#include "settings.h"
#include <qstring.h>
#include <qcheckbox.h>

namespace gui {

/**
 Default constructor of BoolOption item
 @param parent parent Option Editor containing this control
 @param _key Key in settings for this option
 @param _defValue Default value if property not found
 */
BoolOption::BoolOption(const QString &_key,QWidget *parent/*=0*/,bool _defValue/*=false*/)
 : Option (_key,parent) {
 ed=new QCheckBox(this,"booloption_checkbox");
 connect(ed,SIGNAL(clicked()),this,SLOT(boolChange()));
 defValue=_defValue;
}

/** default destructor */
BoolOption::~BoolOption() {
 delete ed;
}

/** Called when clicked on the checkbox */
void BoolOption::boolChange() {
 changed=true;
}

/**
 return size hint of this control
 @return preferred size of this control
*/
QSize BoolOption::sizeHint() const {
 return ed->sizeHint();
}

/**
 Called on resizing of this option editing control
 @param e resize event parameters
*/
void BoolOption::resizeEvent (QResizeEvent *e) {
 ed->setFixedSize(e->size());
}

/** write edited value to settings */
void BoolOption::writeValue() {
 if (!changed) return;
 bool val=ed->isChecked();
 globalSettings->write(key,val?"1":"0");
}

/** read value from settings for editing */
void BoolOption::readValue() {
 bool val=globalSettings->readBool(key,defValue);
 ed->setChecked(val);
}

} // namespace gui

