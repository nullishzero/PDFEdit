#ifndef __INVALIDMENUEXCEPTION_H__
#define __INVALIDMENUEXCEPTION_H__

#include <exception>

class QString;

namespace gui {

/**
 Exception raised when program encountered some error while loading in menu definitions
*/
class InvalidMenuException : public std::exception {
public:
 InvalidMenuException(const QString &message);
 ~InvalidMenuException() throw();
 QString message() const;
protected:
 /** exception message*/
 QString msg;
};

} //namespace gui

#endif
