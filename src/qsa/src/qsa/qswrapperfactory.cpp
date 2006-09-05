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

#include "qswrapperfactory.h"
#include "qsinterpreter.h"
#include "quickinterpreter.h"
#include "qsenv.h"


class QSWrapperFactoryPrivate
{
public:
    QSInterpreter *ip;
    QMap<QString,QString> descriptors;
};

/*!
    \class QSWrapperFactory qswrapperfactory.h
    \brief The QSWrapperFactory class provides a method for Qt Script to
    extend QObject C++ types and to automatically wrap non-QObject C++
    types.

    If a C++ object which is available to Qt Script returns a pointer
    of a non-QObject type, by default Qt Script can't wrap it
    dynamically. It can store the pointer and pass it on, but no
    functions can be called on the object. The QSWrapperFactory class
    provides a means of making non-QObject types properly accessible
    from Qt Script.

    The way QSWrapperFactory achieves this is that when a non-QObject pointer
    is returned, the Qt Script interpreter tries to find a wrapper for
    that type. A wrapper is a QObject which can wrap the non-QObject
    type and which makes the non-QObject type's slots, properties,
    etc. available to Qt Script.

    In some cases even a QObject might need to be wrapped. For
    example, we might want to use a QObject subclass's functions which
    are not declared as slots in Qt Script. Qt Script only makes the
    slots, signals and properties of QObjects available. With
    QSWrapperFactory it is possible to define a QObject which extends another
    QObject with additional slots, properties, and signals.

    To make use of this method, wrap the classes you want to
    make available or want to extend (see the example below). You then
    create a QSWrapperFactory (wrapper factory) subclass. You
    register your wrapper factory (or factories) with the Qt Script
    interpreter. When the interpreter needs an instance of a class, it
    checks the registered wrapper factories to see if the class has a
    suitable wrapper.

    Call registerWrapper() for each class that this instance of
    QSWrapperFactory can wrap. Reimplement create() to create a
    wrapper for a certain type. Finally, create an instance of the
    QSWrapperFactory and add it to an interpreter.

    The following code is an example of how to make a non-QObject C++ type known to
    Qt Script. We create a wrapper for a class called Process that
    provides a function called exec(). We then create a wrapper
    factory which we'll register with the interpreter. You can wrap as
    many classes as you like and make them all available through a
    single wrapper factory (although you can use as many wrapper
    factories as you like).

    The created wrapper is deleted by the interpreter as at some point
    after that the interpreter determines that there are no more
    references to the wrapped type from script.

    \code
    class ProcessWrapper : public QObject
    {
	Q_OBJECT
    public:
	ProcessWrapper( Process *p ) : proc( p ) {}
    public slots:
	void exec( const QString &args ) { proc->exec( args ); }
    private:
	Process *proc;
    };

    class WrapperFactory : public QSWrapperFactory
    {
    public:
	WrapperFactory()
	{
	    registerWrapper( "Process", "ProcessWrapper" );
	}

	QObject *create( const QString &className, void *ptr ) {
	    if ( className == "Process" )
		return new ProcessWrapper( (Process*)ptr );
	    return 0;
	}
    };
    \endcode

    Somewhere in your code, before executing any QSA related
    functionality, you must instantiate an instance of our new
    wrapper.

    \code
    interpreter->addWrapperFactory( new WrapperFactory() );
    \endcode

    Now, whenever Qt Script comes across a Process pointer, it will
    look through its wrapper factories to find one that provides a
    wrapper for the Process class and wrap it (e.g. with
    ProcessWrapper). So the script writer will now be able to use all
    the slots, properties, and signals that ProcessWrapper provides.

    To \e extend a QObject C++ type, you do exactly the same process: i.e.
    wrap the class in a class that exposes the required functionality,
    put the wrapper in a wrapper factory, and register the wrapper
    factory with the interpreter.
*/


/*!
    \fn QObject *QSWrapperFactory::create( const QString &className, void *ptr )

    This function is called by the Qt Script interpreter to request a
    wrapper for the type \a className. \a ptr is a pointer to the
    instance of the type \a className. The \a ptr will be passed to
    its related wrapper function and an instance of the wrapper will
    be returned.

    Reimplement this function to make your wrappers available from
    your wrapper factory.

    If \a ptr is invalid or any other invalid operation occurs, you
    can use throwError() to issue a Qt Script error.
*/


/*!
    This constructor creates a wrapper factory. To make the wrapper factory
    available to an interpreter, use QSInterpreter::addWrapperFactory()

    \sa QSInterpreter::addWrapperFactory()
*/
QSWrapperFactory::QSWrapperFactory()
    : d( new QSWrapperFactoryPrivate() )
{
    d->ip = 0;
}


/*!
  \internal
*/
QSWrapperFactory::~QSWrapperFactory()
{
    if( interpreter() )
	interpreter()->removeWrapperFactory( this );
    delete d;
}


/*!
    Informs the interpreter that an error has occurred. The error is
    treated like a normal Qt Script error. The error message is passed
    in \a message.
*/
void QSWrapperFactory::throwError( const QString &message )
{
    if( d->ip )
	d->ip->throwError( message );
#if defined( QT_CHECK_STATE )
    else
	qWarning( "QSWrapperFactory::throwError(), factory does not have interpreter" );
#endif
}


/*!
  Returns the interpreter that this wrapper factory is creating
  objects for.
*/

QSInterpreter *QSWrapperFactory::interpreter() const
{
    return d->ip;
}


/*!
  \internal
*/
void QSWrapperFactory::setInterpreter( QSInterpreter *inter )
{
    if( inter && d->ip ) {
#if defined( QT_CHECK_STATE )
	qWarning( "QSWrapperFactory::setInterpreter(), already has an interpreter" );
#endif
	return;
    }
    d->ip = inter;
}


/*!
  Registers that this factory provides a wrapper for the type \a className.
  The \a cppClassName variable tells the engine which C++ class is
  used to represent it. This is used for better completion. \cppClassName can
  be null (the default) in which case the engine assumes that the script class
  and C++ implementation both have the same name.
*/
void QSWrapperFactory::registerWrapper( const QString &className,
					const QString &cppClassName )
{
    if( className.isEmpty() ) {
#if defined( QT_CHECK_STATE )
	qWarning( "QSWrapperFactory::registerWrapper(), className is empty" );
#endif
	return;
    }
    d->descriptors[className] = cppClassName;
}


/*!
  \internal
*/
QMap<QString,QString> QSWrapperFactory::wrapperDescriptors() const
{
    return d->descriptors;
}
