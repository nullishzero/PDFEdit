/** @file
 NullPointerException
 - class for exception raised when attempting to call some wrapper function
 methods, while the objest wrapped inside is a NULL pointer, meaning that the
 operation would result in null pointer dereference (and program crash), if
 they would be executed.
 @author Martin Petricek
*/

#include <qstring.h>
#include <qobject.h>
#include "nullpointerexception.h"

namespace gui {

/**
 Default constructor of exception
 @param className Name of class, in which this exception occured
 @param methodName Name of method, in which this exception occured
 */
NullPointerException::NullPointerException(const QString &className,const QString &methodName){
 _class=className;
 _method=methodName;
 _message=QObject::tr("Null pointer access in ")+_class+"."+_method;
}

/**
 Return human-readable exception message
 @return exception message
*/
QString NullPointerException::message() const {
 return _message;
}

/**
 Return class in which this exception was thrown
 @return class name
*/
QString NullPointerException::exceptionClassName() const {
 return _class;
}

/**
 Return method in which this exception was thrown
 @return method name
*/
QString NullPointerException::exceptionMethod() const {
 return _method;
}

/** default destructor */
NullPointerException::~NullPointerException() throw() {
}

/**
 Overloaded what() from std::exception<br>
 For getting usable error message if the exception is not handled
 */
const char* NullPointerException::what() const throw(){
 return _message.ascii();
}

} // namespace gui
