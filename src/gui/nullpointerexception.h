#ifndef __NULLPOINTEREXCEPTION_H__
#define __NULLPOINTEREXCEPTION_H__

class QString;

namespace gui {

class NullPointerException {
public:
 NullPointerException(const QString &className,const QString &methodName);
 QString exceptionClassName();
 QString exceptionMethod();
 ~NullPointerException();
 QString message();
protected:
 /** source of exception - class */
 QString _class;
 /** source of exception - method */
 QString _method;
};

} //namespace gui

#endif
