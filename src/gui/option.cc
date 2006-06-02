/** @file
 Option - class for widget containing one editable setting<br>
 Abstract class, must be subclassed for editing specific option types (integer, string ...)<br>
 @author Martin Petricek
*/

#include <qstring.h>
#include "option.h"

namespace gui {

/**
 Default constructor of option item
 @param parent parent Option Editor containing this control
 @param _key Key in settings for this option
 */
Option::Option(const QString &_key/*=0*/,QWidget *parent/*=0*/) : QWidget (parent, "option"){
 key=_key;
}

/**
 return name of this option (key in settings that is edited by this option)
 @return name of option
*/
QString Option::getName() {
 return key;
}

/** default destructor */
Option::~Option() {
}

} // namespace gui
