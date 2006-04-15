/** @file
StringOption - class for widget containing one editable setting of type string, selectable from combobox
*/

#include <utils/debug.h>
#include "combooption.h"
#include <qstring.h>
#include <qcombobox.h>
#include "settings.h"

namespace gui {

/** Default constructor of StringOption item
 @param parent parent Option Editor containing this control
 @param _key Key in settings for this option
 @param values List of accepted values
 */
ComboOption::ComboOption(const QStringList &values,const QString &_key/*=0*/,QWidget *parent/*=0*/)
 : Option (_key,parent) {
 ed=new QComboBox(false,this,"option_combo");
/* for (int i=0;i<values.count();i++) {
  ed->insertItem(styles[i]);
 }*/
 ed->insertStringList(values);
 ed->setInsertionPolicy(QComboBox::NoInsertion);
}

/** default destructor */
ComboOption::~ComboOption() {
 delete ed;
}

/** write edited value to settings */
void ComboOption::writeValue() {
 globalSettings->write(key,ed->currentText());
}

/** read value from settings for editing */
void ComboOption::readValue() {
 QString value=globalSettings->read(key);
 if (value.isNull()) return;
 //TODO: if value not in list, use first value
 ed->setCurrentText(value);
}

/** return size hint of this property editing control */
QSize ComboOption::sizeHint() const {
 return ed->sizeHint();
}

/** Called on resizing of property editing control */
void ComboOption::resizeEvent (QResizeEvent *e) {
 ed->setFixedSize(e->size());
}

} // namespace gui
