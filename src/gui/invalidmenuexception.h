#ifndef __INVALIDMENUEXCEPTION_H__
#define __INVALIDMENUEXCEPTION_H__

class QString;

namespace gui {

/**
 Exception raised when program encountered some error while loading in menu definitions
*/
class InvalidMenuException {
public:
 InvalidMenuException(const QString &message);
 ~InvalidMenuException();
 QString message() const;
protected:
 /** exception message*/
 QString msg;
};

} //namespace gui

#endif
