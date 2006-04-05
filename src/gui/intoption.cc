/** @file
IntOption - class for widget containing one editable setting of type integer
*/

#include "intoption.h"
#include <qvalidator.h>

/** Default constructor of IntOption item
 @param parent parent Option Editor containing this control
 @param _key Key in settings for this option
 */
IntOption::IntOption(const QString &_key/*=0*/,QWidget *parent/*=0*/)
 : StringOption (_key,parent) {
 ed->setText("0");
 ed->setValidator(new QIntValidator(ed));
}

/** default destructor */
IntOption::~IntOption() {
}

