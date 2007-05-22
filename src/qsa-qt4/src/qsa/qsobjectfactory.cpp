/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech AS. All rights reserved.
**
** This file is part of the QSA of the Qt Toolkit.
**
** For QSA Commercial License Holders (non-open source):
** 
** Licensees holding a valid Qt Script for Applications (QSA) License Agreement
** may use this file in accordance with the rights, responsibilities and
** obligations contained therein.  Please consult your licensing agreement or
** contact sales@trolltech.com if any conditions of the Licensing Agreement are
** not clear to you.
** 
** Further information about QSA licensing is available at:
** http://www.trolltech.com/products/qsa/licensing.html or by contacting
** info@trolltech.com.
** 
** 
** For Open Source Edition:  
** 
** This file may be used under the terms of the GNU General Public License
** version 2 as published by the Free Software Foundation and appearing in the
** file LICENSE.GPL included in the packaging of this file.  Please review the
** following information to ensure GNU General Public Licensing requirements
** will be met:  http://www.trolltech.com/products/qt/opensource.html 
** 
** If you are unsure which license is appropriate for your use, please review
** the following information:
** http://www.trolltech.com/products/qsa/licensing.html or contact the 
** sales department at sales@trolltech.com.

**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qsobjectfactory.h"
#include "qsinterpreter.h"
#include "quickinterpreter.h"
#include "qsenv.h"

#include <qmetaobject.h>
#include <QList>

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
  QSObjectFactory, reimplement the create() function and register
  the provided classes using the registerClass() function.

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
    QList<QObject*> objects = d->staticDescriptors.values();
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
  constructor are passed in the \a arguments list. See QVariant for
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

  If the arguments are invalid or any other invalid operation happens,
  you can use throwError() to issue a Qt Script error.

  All objects created from this function are deleted by the engine when
  the engine registers it as not being referenced anymore, unless the
  object at this time has a parent, which will then be responsible for
  ownership.
*/
QObject *QSObjectFactory::create( const QString &name, const QVariantList &,
				  QObject * )
{
    throwError("QSObjectFactory::create() not implemented for " + name);
    return 0;
}


/*!
  Registers that this object factory provides the class \a className.

  This function is called once for each of the script classes that a
  subclass of QSObjectFactory wishes to expose to the scripting
  engine.

  The name \a className is the name that the class has within the
  scripting engine. \a metaObject is used by the interpreter to
  describe QObjects of returned for \a className when using syntax
  completion.

  is the name of the C++ class that is
  actually instantiated when \a className is passed to the create()
  function. For instance:

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
  registerClass( "MyObject", &MyScriptObject::staticMetaObject );
  \endcode

  Providing a null-pointer for \a metaObject will tell the interpreter
  that the class is not instantiable.

  The \a staticDescriptor is used to describe the static part of a class.
  The properties, signals and slots of the \a staticDescriptor object
  will be accessible as \a className 's static part.

  For instance, if you want to expose the entire Qt namespace to QSA, it can be done
  with the following code:

  \code
  class QtNamespace: public QObject
  {
    public:
        const QMetaObject *metaObject()
        {
            return &QObject::staticQtMetaObject;
        }
  };

  class QtNamespaceProvider: public QSObjectFactory
  {
    public:
        QtNamespaceProvider()
        {
            registerClass("Qt", 0, new QtNamespace());
        }
  };
  \endcode

  When QtNamespaceProvider is registered with an interpreter, scripts in the
  interpreter can access the Qt namespace through the class name "Qt". (Note that
  the QtNamespace class cannot have a Q_OBJECT macro, as this macro also declares
  the metaObject() method.)
*/
void QSObjectFactory::registerClass( const QString &className,
				     const QMetaObject *metaObject,
				     QObject *staticDescriptor )
{
    if( className.isEmpty() ) {
	qWarning( "QSObjectFactory::registerClass(), className is empty" );
	return;
    }

    if( staticDescriptor ) {
	    d->staticDescriptors[className] = staticDescriptor;
        if (staticDescriptor->metaObject())
            QSInterpreter::registerMetaObject(staticDescriptor->metaObject());
    }
    

    if(metaObject) {
        QSInterpreter::registerMetaObject(metaObject);
	    d->instanceDescriptors[className] = metaObject->className();
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

