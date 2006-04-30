/** @file
 Reference validator - checks if given String is valid Reference in given document
 Does validate only format, not if the reference exists (that would be too slow)
 Reference existence is validated on attempt to write it to the property
 @author Martin Petricek
 */

#include "refvalidator.h"
#include <qstring.h> 
#include <utils/debug.h> 
#include "util.h"

namespace gui {

/** Regular expression for validation */
QRegExp valid("\\d+,\\d+");

/** Regular expression for intermediate validation */
QRegExp ivalid("\\d*,?\\d*");


/** Constructor for this validator
@param parent Parent object
@param name name of this validator (not used, except debugging)
 */
RefValidator::RefValidator(QObject *parent,const char *name/*=0*/) : QValidator(parent,name) {
}

/** Try to "fix" the string to be valid. Does not guarantee that string will be valid after returning
 @param input String to fix (reference)
*/
void RefValidator::fixup(QString &input) const {
 input=input.simplifyWhiteSpace();
}

/** Try to validate the string as reference target
 @param input String to validate (reference)
 @param pos position of cursor (not changed)
 @return result of validation (Acceptable, Intermediate or Invalid)
*/
QValidator::State RefValidator::validate(QString &input,int &pos) const {
 if (valid.exactMatch(input)) {
  guiPrintDbg(debug::DBG_DBG,"Validate: " << input << " Acceptable");
  return Acceptable;
 }
 if (ivalid.exactMatch(input)) {
  guiPrintDbg(debug::DBG_DBG,"Validate: " << input << " Intermediate");
  return Intermediate;
 }
 guiPrintDbg(debug::DBG_DBG,"Validate: " << input << " Invalid");
 return Invalid;
}

} // namespace gui
