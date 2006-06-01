#ifndef __INVALIDMENUEXCEPTION_H__
#define __INVALIDMENUEXCEPTION_H__

class QString;

namespace gui {

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
