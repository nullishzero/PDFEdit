#ifndef __QSCOBJECT_H__
#define __QSCOBJECT_H__

#include <qobject.h>
#include <boost/shared_ptr.hpp>
#include "basecore.h"
class QString;

namespace gui {

//class Base;

/*= Base class for all PDF objects used in scripts */
/** Common base class of all QObject wrappers used in scripting */
class QSCObject : public QObject {
Q_OBJECT
public:
 QSCObject(QString _typeName, BaseCore* _base);
 QSCObject(QSCObject &source);
 virtual ~QSCObject();
public slots:
 /*- return name of this object's type */
 virtual QString type();
protected:
 bool nullPtr(const void* ptr,const QString &methodName);
protected:
 /** Scripting base for this object */
 BaseCore *base;
private:
 /** Name of this objects's type */
 QString typeName;

 //Template functions
protected:
 /**
  Template variant of nullPtr checking any shared pointer
  \see nullPtr(void*,const QString &);
  @param ptr Shared pointer to check if it is not empty
  @param methodName Name of method that called this
 */
 template<typename T> bool nullPtr(boost::shared_ptr<T> ptr,const QString &methodName) {
  return nullPtr(ptr.get(),methodName);
 }
 /**
  Template function that will try to dynamic cast object to template type.
  In case of failure return NULL and pass 'bad parameter' message to script error handler
  \see Base::errorBadParameter
  @param methodName Name of method that called this
  @param param Parameter number that is being checked
  @param paramNum Position of parameter in parameters
  @param expected Name of type that is expected in parameter
 */
 template<typename T> T qobject_cast(QObject *param,const QString &methodName,int paramNum,const QString &expected) {
  T out=dynamic_cast<T>(param);
  if (!out) {
   //the cast was not successful somehow
   base->errorBadParameter(typeName,methodName,paramNum,param,expected);
  }
  return out;
 }
};

} // namespace gui

#endif
