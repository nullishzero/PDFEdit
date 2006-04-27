/** @file
 InvalidMenuException
 - class for exception raised when found some error in menu definitions
 @author Martin Petricek
*/

#include <qstring.h>
#include "invalidmenuexception.h"

namespace gui {

/** Default constructor of exception
 @param message Exception message
 */
InvalidMenuException::InvalidMenuException(const QString &message){
 msg=message;
}

/** Return exception message passed in constructor of this exception
 @return exception message */
QString InvalidMenuException::message() {
 return msg;
}

/** default destructor */
InvalidMenuException::~InvalidMenuException() {
}

} // namespace gui
