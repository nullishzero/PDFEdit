/** @file
 Reference validator - checks if given String is valid Reference in given document
 */

#include "refvalidator.h"
#include <qstring.h> 

namespace gui {

/** regexp for validation */
QRegExp valid("(\\d+),(\\d+)");

/** regexp for intermediate validation */
QRegExp ivalid("(\\d+)?,?(\\d+)?");


/** Constructor for this validator
@param parent Parent object
@param name name of this validator (not used, except debugging)
 */
RefValidator::RefValidator(QObject *parent,const char *name/*=0*/) : QValidator(parent,name) {
}

/** Try to "fix" the string to be valid
 @param input String to fix (reference)
*/
void RefValidator::fixup(QString &input) const {
 input=input.simplifyWhiteSpace();
}

/** Try to validate the string
 @param input String to validate (reference)
 @param pos position of cursor (not changed)
*/
QValidator::State RefValidator::validate(QString &input,int &pos) const {
 if (valid.exactMatch(input)) {
  //TODO: validate with PDF -> if (!validRef) return Intermediate;
  //TODO: may be slow, if slow, validate before write
  //TODO: use CXRef from Cpdf for this
  return Acceptable;
 }
 if (ivalid.exactMatch(input)) return Intermediate;
 return Invalid;
}

} // namespace gui
