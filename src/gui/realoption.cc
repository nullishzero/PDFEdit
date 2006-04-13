/** @file
RealOption - class for widget containing one editable setting of type float
*/

#include "realoption.h"
#include <qvalidator.h>
#include <qlineedit.h>

namespace gui {

/** Default constructor of REalOption item
 @param parent parent Option Editor containing this control
 @param _key Key in settings for this option
 */
RealOption::RealOption(const QString &_key/*=0*/,QWidget *parent/*=0*/)
 : StringOption (_key,parent) {
 ed->setText("0");
 ed->setValidator(new QDoubleValidator(ed));
}

/** default destructor */
RealOption::~RealOption() {
}

} // namespace gui
