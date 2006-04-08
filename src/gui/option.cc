/** @file
Option - class for widget containing one editable setting
Can be subclassed for editing specific option types (integer, ...)
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
 set=Settings::getInstance();
// printDbg(debug::DBG_DBG,"+ Option " << key);
}

/** return name of this option */
QString Option::getName() {
 return key;
}

/** default destructor */
Option::~Option() {
// printDbg(debug::DBG_DBG,"- Option " << key);
}
