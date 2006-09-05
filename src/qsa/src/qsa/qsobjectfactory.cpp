/****************************************************************************
** $Id$
**
** Copyright (C) 2001-2006 Trolltech AS.  All rights reserved.
**
** This file is part of the Qt Script for Applications framework (QSA).
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding a valid Qt Script for Applications license may use
** this file in accordance with the Qt Script for Applications License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about QSA Commercial License Agreements.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
*****************************************************************************/

#include "qsobjectfactory.h"
#include "qsinterpreter.h"
#include "quickinterpreter.h"
#include "qsenv.h"

#include <qmetaobject.h>

class QSObjectFactoryPrivate
{
public:
    QSInterpreter *ip;
    QMap<QString,QObject*> staticDescriptors;
    QMap<QString,QString> instanceDescriptors;
};

/*!
  \class QSObjectFactory qsobjectfactory.h

  \brief The QSObjectFactory class provides a method for Qt Script
  programs to create C++ QObjects.

  To enable script programmers to create their own C++ QObjects,
  application programmers can provide a QObject subclass that has a
  slot which is a factory function that returns QObjects.
  Alternatively, the application programmer can subclass
  QSObjectFactory and reimplement the create() and classes()
  functions.

  The registerClass() function is called to register to the scripting
  engine each of the classes that this object factory can instantiate.
  In addition, the create() function is called by the scripting engine
  to create the instance, i.e. when the user writes something like


  \code
  var x = new SomeCppObject( arg1, arg2 ); // Qt Script
  \endcode

  The function create() is reimplemented in each QSObjectFactory
  to provide the an instance of the QObject subclass that describes
  what the scripter should experience as an object of the type
  SomeCppObject.

  A single QSObjectFactory subclass may be used to provide any number
  of creatable QObject classes. A QSObjectFactory becomes active when
  it is added to a QSInterpreter using the function
  QSInterpreter::addObjectFactory(). An object factory can only be
  added to one QSInterpreter at a time.
*/


/*! Constructor.
  Creates the object factory. To make the object factory available to
  an interpreter, use the function: QSInterpreter::addObjectFactory().
*/

QSObjectFactory::QSObjectFactory()
{
    d = new QSObjectFactoryPrivate;
    d->ip = 0;
}


/*!
  \internal
*/
QSObjectFactory::~QSObjectFactory()
{
    if( interpreter() )
	interpreter()->removeObjectFactory( this );
    QValueList<QObject*> objects = d->staticDescriptors.values();
    while( objects.count() > 0 ) {
	const QObject *obj = objects.front();
	objects.pop_front();
	delete obj;
    }

    delete d;
}

/*!
    Informs the interpreter that an error has occurred. The error is
    treated like a normal Qt Script error. The error message is passed
    in \a message.
*/

void QSObjectFactory::throwError( const QString &message )
{
    if( !d->ip ) {
#if defined ( QT_CHECK_RANGE )
	qWarning( "QSObjectFactory::throwError(), no interpreter" );
#endif
	return;
    }
    d->ip->throwError( message );
}


/*!
  Returns the interpreter that this object factory is creating
  objects for.
*/
QSInterpreter *QSObjectFactory::interpreter() const
{
    return d->ip;
}

/*!
  \internal
*/
void QSObjectFactory::setInterpreter( QSInterpreter *i )
{
    if( i && d->ip ) {
#if defined( QT_CHECK_STATE )
	qWarning( "QSObjectFactory::setInterpreter(), already has an interpreter" );
#endif
	return;
    }
    d->ip = i;
}


/*!
  This virtual function should be reimplemented in your
  QSObjectFactory subclass when you want to instantiate objects from
  script. The subclass can be used to create any number of different
  classes. The name of the required class is passed in the \a
  className argument, and the arguments to be passed to that class's
  constructor are passed in the \a arguments list. See QSArgument for
  further information about the arguments. \a context is the script
  QObject context in which the class has been instantiated, or 0 if
  the class has not been instantiated in a QObject context.

  Only QObject subclasses may be created in this way. This
  function returns an instance of the requested class.

  This function is called by the scripting engine, e.g. when it
  encounters code similar to the following:
  \code
    var x = new ACppObject( arg1, arg2 ); // Qt Script
  \endcode

  The classes that a particular QSObjectFactory instance is capable of
  instantiating is returned by classes().

  If the arguments are invalid or any other invalid operation happens,
  you can use throwError() to issue a Qt Script error.

  All objects created from this function are deleted by the engine when
  the engine registers it as not being referenced anymore, unless the
  object at this time has a parent, which will then be responsible for
  ownership.
*/
QObject *QSObjectFactory::create( const QString &, const QSArgumentList &,
				  QObject * )
{
    return 0;
}


/*!
  \overload
*/
void QSObjectFactory::registerClass( const QString &className,
				     QObject *staticDescriptor )
{
    registerClass( className, className, staticDescriptor );
}


/*!
  Registers that this object factory provides the class \a className.

  This function is called once for each of the script classes that a
  subclass of QSObjectFactory wishes to expose to the scripting
  engine.

  The name \a className is the name that the class has within
  the scripting engine. \a cppClassName is the name of the
  C++ class that is actually instantiated when \a className is
  passed to the create() function. For instance:

  \code
  // in script
  var x = new MyObject();

  // in C++
  QObject *create( const QString &name, ... ) {
      if( name == "MyObject" )
          return new MyScriptObject()
  }
  \endcode

  Here we would call registerClass as follows:
  \code
  registerClass( "MyObject", "MyScriptObject" );
  \endcode

  Providing QString::null in \a cppClassName will tell the interpreter
  that the class is not instantiable.

  The \a staticDescriptor is used to describe the static part of a class.
  The properties, signals and slots of the \a staticDescriptor object
  will be accessible as \a className 's static part.
*/
void QSObjectFactory::registerClass( const QString &className,
				     const QString &cppClassName,
				     QObject *staticDescriptor )
{
    if( className.isEmpty() ) {
#if defined( QT_RANGE_CHECK )
	qWarning( "QSObjectFactory::registerClass(), className is empty" );
#endif
	return;
    }
    if( staticDescriptor )
	d->staticDescriptors[className] = staticDescriptor;
    if( !cppClassName.isNull() ) {
#if defined QT_RANGE_CHECK
	if( !QMetaObject::metaObject( cppClassName ) )
	    qWarning( "QSObjectFactory::registerClass(), unknown class: "
		      + cppClassName + ",\nCompletion will not work!" );
#endif
	d->instanceDescriptors[className] = cppClassName;
    }
}


/*!
  \internal
  This function should only be used by the interpreter.
*/
QMap<QString,QObject*> QSObjectFactory::staticDescriptors() const
{
    return d->staticDescriptors;
}


/*!
  \internal
  This function should only be used by the interpreter.
*/
QMap<QString,QString> QSObjectFactory::instanceDescriptors() const
{
    return d->instanceDescriptors;
}

