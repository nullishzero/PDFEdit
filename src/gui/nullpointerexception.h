#ifndef __NULLPOINTEREXCEPTION_H__
#define __NULLPOINTEREXCEPTION_H__

#include <exception>

class QString;

namespace gui {

/**
 Exception raised when attempting to call some wrapper function
 methods, while the objest wrapped inside is a NULL pointer, meaning that the
 operation would result in null pointer dereference (and program crash), if
 they would be executed.
 \brief Exception raised when encountering NULL pointer when not expected
*/
class NullPointerException : public std::exception {
public:
 NullPointerException(const QString &className,const QString &methodName);
 QString exceptionClassName() const;
 QString exceptionMethod() const;
 QString message() const;
 virtual const char* what() const throw();
 ~NullPointerException() throw();
protected:
 /** source of exception - class */
 QString _class;
 /** source of exception - method */
 QString _method;
 /** Exception message */
 QString _message;
};

} //namespace gui

#endif
