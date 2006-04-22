/** @file
 ComboOption - class for widget containing one editable setting of type string, selectable from combobox<br>
 If current setting specify item not in the list, the first item in list is shown instead<br>
 User is unable to specify string not in the list<br>
 Used as one item in option window<br>
 @author Martin Petricek
*/

#include <utils/debug.h>
#include "combooption.h"
#include <qstring.h>
#include <qstringlist.h>
#include <qcombobox.h>
#include "settings.h"

namespace gui {

/** Default constructor of StringOption item
 @param parent parent Option Editor containing this control
 @param _key Key in settings for this option
 @param _values List of accepted values
 */
ComboOption::ComboOption(const QStringList &_values,const QString &_key/*=0*/,QWidget *parent/*=0*/)
 : Option (_key,parent) {
 ed=new QComboBox(false,this,"option_combo");
 values=_values;
 caseSensitive=false;
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
 //Look for item in the list
 QStringList matches=values.grep(value,caseSensitive);
 //If value not in list, use first value
 if (matches.count()==0) ed->setCurrentText(values[0]);
 //Use first match (Will "normalize" case in case of case-insensitive matching)
 ed->setCurrentText(matches[0]);
}

/** Set case sensitivity of list items. Default is case insensitive
 @param value New value for case-sensitivity
 */
void ComboOption::setCaseSensitive(bool value) {
 caseSensitive=value;
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
