/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006-2009  PDFedit team: Michal Hocko,
 *                                        Jozef Misutka,
 *                                        Martin Petricek
 *                   Former team members: Miroslav Jahoda
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (in doc/LICENSE.GPL); if not, write to the 
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 * MA  02111-1307  USA
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit
 */
#ifndef __QSCOBJECT_H__
#define __QSCOBJECT_H__

#include <qobject.h>
#include <boost/shared_ptr.hpp>
#include "basecore.h"
class QString;

namespace gui {

//class Base;

/*= Base class for all PDF objects used in scripts */
/** \brief Common base class of all QObject wrappers used in scripting */
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
