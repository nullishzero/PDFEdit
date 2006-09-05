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

#include "qsinterpreter.h"
#include "qsscript.h"
#include "qsobjectfactory.h"
#include "qswrapperfactory.h"
#include "../kernel/quickdebugger.h"
#include "../kernel/quickinterpreter.h"
#include "../kernel/quickobjects.h"
#include "../kernel/quickdispatchobject.h"
#include "../engine/qsenv.h"
#include "../engine/qsfuncref.h"

#include <qapplication.h>
#include <qmessagebox.h>
#include <qptrlist.h>
#include <qcleanuphandler.h>

#if defined (QT_THREAD_SUPPORT)
#  include <private/qmutexpool_p.h>
#  include <qthread.h>
#  if QT_VERSION >= 0x030300
extern Q_EXPORT Qt::HANDLE qt_get_application_thread_id();
#  endif
#endif

#ifdef QSA_EXPIRES
static void qsa_eval_check();
#endif

class QSInterpreterPrivate
{
public:
    QSInterpreterPrivate() :
	project( 0 ),
	interpreter( 0 ),
	errorMode( QSInterpreter::Notify ),
	timeoutInterval(-1)
    {
    }

    ~QSInterpreterPrivate()
    {
	delete interpreter;
    }

    QSProject *project;
    QuickInterpreter *interpreter;
    QSStackTrace stackTrace;
    QSInterpreter::ErrorMode errorMode;
    QPtrList<QSObjectFactory> objectFactories;
    QPtrList<QSWrapperFactory> wrapperFactories;
    int timeoutInterval;
};

QuickInterpreter *get_quick_interpreter( QSInterpreter *ip )
{
    return ip->d->interpreter;
}

#define startInterpreter() if( d->project ) d->project->evaluate();
#define stopInterpreter()


/*!
  \class QSStackFrame qsinterpreter.h

  \brief The QSStackFrame class provides information about a function call
  in the stack trace.

  A QSStackFrame object only makes sense when grouped together in a
  QSStackTrace to provide a trace of the execution up to where an
  error occurred.

  The QSStackFrame contains four values: file name, line number,
  function name, and context.
*/


/*!
  \fn QSStackFrame::QSStackFrame( const QString &, const QString &,
                                  int line, QObject * )
  \internal
*/


/*!
  \fn QString QSStackFrame::function() const

  Returns the name of the function that this stack frame describes.
*/


/*!
  \fn QString QSStackFrame::scriptName() const

  Returns the name of the script in which the function is located if
  the script name was specified when the function was evaluated;
  otherwise returns QString::null.
*/


/*!
  \fn int QSStackFrame::line() const

  Returns the line number within the script.
*/


/*!
  \fn QObject *QSStackFrame::context() const

  Returns the context of the function if it was evaluated in the
  context of a QObject; otherwise returns 0.
*/


/*!
  \class QSStackTrace qsinterpreter.h

  \brief The QSStackTrace class describes a stack trace after an error
  has occurred in the interpreter.

  The QSStackTrace contains a list of QSStackFrame objects that
  together describe the call stack of the interpreter at the point
  when an error occurred.
*/


/*!
  \class QSInterpreter qsinterpreter.h

  \brief The QSInterpreter class provides the public API for the Qt Script
  for Applications script engine.

  This class (implemented in \c libqsa) provides the functionality
  required to make Qt/C++ applications scriptable with Qt Script.

  For convenience a single instance of the QSInterpreter class exists
  in an application; it is available as
  QSInterpreter::defaultInterpreter().

  The functions evaluate(), call(), addTransientObject(), and clear()
  provide the basic functionality of the interpreter. Any string containing
  valid Qt Script code can be executed using evaluate(), and any state
  built up during evaluation is kept between calls. The function
  call() can be used to call script functions from C++. The function
  addTransientObject() will add an object to the interpreter until the
  interpreter is cleared. Calling clear() will clear the state of the
  interpreter and remove all transient objects.

  The function checkSyntax() provides syntax checking without having
  to execute the context of the code.

  QSInterpreter provides several functions for script introspection.
  These functions are: classes(), functions(), and variables().

  If an error occurs, for example, during the execution of a script,
  the error() signal is emitted. The error behavior depends on the
  errorMode() which is set with setErrorMode(). When the interpreter
  stops execution because of an error, the hadError(), errorMessage(),
  and stackTrace() functions can be used to provide error information.

  It is possible to run QSInterpreter in a separate thread when
  compiling against Qt 3.3 or later. This allows you to have multiple
  interpreters running simultaneously. Interpreters can interact with
  QObjects in the same thread they are running. Note that normal
  restrictions for threading in Qt still apply, such as interacting
  with objects on other threads and the GUI thread.

  When an object has been made accessible to the interpreter, such as
  through the functions \l addTransientObject() or \l evaluate() all
  its signals, slots and properties will be made accessible from script.
  QSA provides a series of automatic conversions between C++ types
  and Qt Script types. These are listed below.

  The following table describes which script types are supported as
  function arguments when calling a C++ slot from QSA.

  \table
  \row \i <b>C++ Type</b> \i <b>Qt Script Type</b>
  \row \i \c QByteArray      \i \c ByteArray
  \row \i \c QByteArray*     \i \c ByteArray, \c undefined
  \row \i \c QColor          \i \c Color
  \row \i \c QColor*         \i \c Color, \c undefined
  \row \i \c QColorGroup     \i \c ColorGroup
  \row \i \c QColorGroup*    \i \c ColorGroup, \c undefined
  \row \i \c QFont           \i \c Font
  \row \i \c QFont*          \i \c Font, \c undefined
  \row \i \c QObject*        \i \c QObject, \c undefiend
  \row \i \c QObjectList     \i \c Array (of QObjects)
  \row \i \c QObjectList*    \i \c Array (of QObjects), \c undefined
  \row \i \c QPalette        \i \c Palette
  \row \i \c QPalette*       \i \c Palette, \c undefined
  \row \i \c QPixmap         \i \c Pixmap
  \row \i \c QPixmap         \i \c Pixmap, \c undefined
  \row \i \c QPoint          \i \c Point
  \row \i \c QPoint*         \i \c Point, \c undefined
  \row \i \c QRect           \i \c Rect
  \row \i \c QRect*          \i \c Rect, \c undefined
  \row \i \c QSize 	     \i \c Size
  \row \i \c QSize*  	     \i \c Size, \c undefined
  \row \i \c QString         \i \c String, \c Number, \c undefined
  \row \i \c QString*        \i \c String, \c Number, \c undefined
  \row \i \c QStringList     \i \c String, Array
  \row \i \c QStringList*    \i \c String, \c Array, \c undefined
  \row \i \c QValueList<int> \i \c Array
  \row \i \c QValueList<int>* \i \c Array
  \row \i \c QVariant        \i \c All types except QObject* and wrapped pointer.
  \row \i \c QVariant*       \i \c All types except QObject* and wrapped pointer.
  \row \i \c bool            \i \c Boolean, \c Number (0 is FALSE), \c String ("", "0" and "false" is false)
  \row \i \c char            \i \c Number, \c String (first character), \c undefined
  \row \i \c char*           \i \c String, \c undefined
  \row \i \c double          \i \c Number, \c Boolean, \c undefined
  \row \i \c float           \i \c Number, \c Boolean, \c undefined
  \row \i \c int             \i \c Number, \c Boolean, \c undefined
  \row \i \c long            \i \c Number, \c Boolean, \c undefined
  \row \i \c void*           \i Wrapped pointer, \c undefined
  \row \i \c short           \i \c Number, \c Boolean, \c undefined
  \row \i \c uchar           \i \c Number, \c String (first character), \c undefined
  \row \i \c uint     	     \i \c Number, \c Boolean, \c undefined
  \row \i \c ulong           \i \c Number, \c Boolean, \c undefined
  \row \i \c ushort          \i \c Number, \c Boolean, \c undefined
  \endtable

  The following table describes which C++ types are converted to Qt Script
  types when used as properties or used as return values from slots.

  \table
  \row \i <b>C++ Type</b>    \i <b>Qt Script Type</b>
  \row \i \c QObject*           \i \c QObject
  \row \i \c QString            \i \c String
  \row \i \c QStringList        \i \c Array
  \row \i \c QVariant           \i \c Variant or matching type (Font, Color, etc)
  \row \i \c bool               \i \c Boolean
  \row \i \c double             \i \c Number
  \row \i \c int                \i \c Number
  \row \i \c uint               \i \c Number
  \row \i \c void*              \i Wrapped pointer
  \endtable

  Note that

  See the \link qsa.book Manual\endlink for more explanations and
  examples.
*/


/*! \enum QSInterpreter::ErrorMode

  The ErrorMode enum describes what happens when an error occurs
  while parsing or executing script code.

  \value Notify Notifies the user that an error occurred via a message
  box. If the application is running in console mode (QApplication::Tty),
  the notification will be output to \c stderr.

  \value Nothing No notification is sent to the user. The interpreter
  still stops execution and emits the \l error() signal.
*/


/*!
  \enum QSInterpreter::ClassFlags

  The ClassFlags enum specifies which classes should be made available
  for introspection.

  \value AllClasses All available classes in the interpreter, including those
  declared in QObject contexts, are matched.

  \value GlobalClasses Only classes declared in the global scope are
  matched.
*/


/*!
  \enum QSInterpreter::FunctionFlags

  The FunctionFlags enum describes matching rules and formatting for
  function introspection.

  \value FunctionNames Returns function names only.

  \value FunctionSignatures Returns the functions with signatures.

  \value IncludeMemberFunctions Matches member functions also, when
  the context to match in is a script class.
*/


/*! \fn bool QSInterpreter::isRunning() const
  Return TRUE if the interpreter is currently evaluating code;
  otherwise returns FALSE.
*/


/*!
  \fn void QSInterpreter::error( const QString &message,
  const QString &scriptName, int lineNumber )

  This signal is emitted if an error occurs when running or parsing a
  script. \a message contains the error message from the interpreter,
  \a scriptName contains the script name (if known) in which the error
  occurred, and \a lineNumber contains the line number at which the
  error occurred.
*/

/*!
  \fn void QSInterpreter::timeout( int elapsedTime )

  This signal is emitted on intervals specified by \l timeoutInterval

  The \a elapsedTime parameter describes the number of milliseconds
  the interpreter has been running.
*/


/*!
  \fn void QSInterpreter::error( const QString &message,
  QObject *context, const QString &scriptName, int lineNumber )

  This signal is emitted if an error occurs when running or parsing a
  script. \a message contains the error message from the interpreter,
  \a context is a pointer to the QObject context in which the error
  occurred or 0, if the context is the global context, \a scriptName
  contains the script name (if known) in which the error occurred, and
  \a lineNumber contains the line number at which the error occurred.
*/


/*
  \internal
*/
QSInterpreter::QSInterpreter( QSProject *project, const char *name )
    : QObject( project, name )
{
    d = new QSInterpreterPrivate;
    d->project = project;
    init();
}


/*!
  Constructs a QSInterpreter that runs without a project.

  The \a parent and \a name parameters are passed on to the QObject
  base class.

  There's a default instance accessible with
  QSInterpreter::defaultInterpreter().
*/
QSInterpreter::QSInterpreter( QObject *parent, const char *name )
    : QObject( parent, name )
{
    d = new QSInterpreterPrivate;
    init();
}

static int qsa_mutex_thingy = 1;

/*!
  \internal
*/
void QSInterpreter::init()
{
#ifdef QSA_EXPIRES
    qsa_eval_check();
#endif

#if defined (QT_THREAD_SUPPORT) && QT_VERSION >= 0x030300
    QMutexLocker locker( qt_global_mutexpool ? qt_global_mutexpool->get(&qsa_mutex_thingy) : 0 );
#endif

    running = FALSE;
    d->interpreter = new QuickInterpreter( TRUE );
    connect( d->interpreter, SIGNAL( runtimeError() ),
	     this, SLOT( runtimeError() ) );
    connect( d->interpreter, SIGNAL( parseError() ),
	     this, SLOT( parseError() ) );
}

/*! Destructor
  \internal
*/

QSInterpreter::~QSInterpreter()
{
    for( QSObjectFactory *objs = d->objectFactories.first();
	 objs; objs = d->objectFactories.next() )
	objs->setInterpreter( 0 );

    for( QSWrapperFactory *wraps = d->wrapperFactories.first();
	 wraps; wraps = d->wrapperFactories.next() )
	wraps->setInterpreter( 0 );

    d->objectFactories.setAutoDelete( TRUE );
    d->wrapperFactories.setAutoDelete( TRUE );

    delete d;
}

static QSInterpreter *def_inter = 0;
static QSingleCleanupHandler<QSInterpreter> def_cleanup;

/*!
  Returns the default interpreter.

  The default interpreter runs without a project.

  This function will automatically create the interpreter if it
  doesn't already exist.
*/

QSInterpreter* QSInterpreter::defaultInterpreter()
{
    if ( !def_inter ) {
	def_inter = new QSInterpreter();
	def_cleanup.set( &def_inter );
    }
    return def_inter;
}


/*!
  \property QSInterpreter::errorMode

  \brief What happens when there is an error:
*/

void QSInterpreter::setErrorMode( ErrorMode m )
{
    d->errorMode = m;
}

QSInterpreter::ErrorMode QSInterpreter::errorMode() const
{
    return d->errorMode;
}


/*!
  Makes the QObject \a object available to the scripting engine. All child
  named objects of \a object are also made available, recursivly.

  Transient objects added to the interpreter are not persistent. This
  means that when the interpreter is cleared, or when a project is
  re-evaluated, the transient objects are removed.

  Use QSProject::addObject() to add persistent objects to the
  interpreter.

  Note on threading; If the interpreter is running in the non-GUI thread,
  \a object cannot be a QWidget subclass.

  \warning Every object passed to this function must have a unique
  name.  If you want to reuse names then you need to call clear() first

  \sa QSProject::addObject()
*/
void QSInterpreter::addTransientObject( QObject *object )
{
#if defined (QT_THREAD_SUPPORT) && QT_VERSION >= 0x030300
    if (object
	&& object->inherits("QWidget")
	&& qt_get_application_thread_id() != QThread::currentThread()) {
	qWarning("QSInterpreter::evaluate(), GUI object %s [%s] not allowed in non GUI thread",
		 object->name(), object->className());
	return;
    }
#endif
    d->interpreter->addTopLevelObject( object );
}


/*!
  Executes the string of Qt Script in \a code and returns any value
  produced by that \a code.

  This function executes the code passed in as \a code. The code can
  use and reference code (functions, classes, variables, etc.) which
  have been passed to this function previously or which are defined in
  the current project, if present. Also, application objects which have
  been added via addObject() can be accessed.

  If \a context is 0 (the default), the code is executed as global
  code. If a \a context is given, the code is executed in the context
  of that object.

  Interpreters that belong to a project are subject to re-evaluation,
  since the code which has been passed previously into evaluate() gets
  lost when calling one of these functions. This happens when the project
  or the scripts in it are modified.

  \a scriptName is used for error reporting and debugging.
*/
QSArgument QSInterpreter::evaluate( const QString &code, QObject *context,
				    const QString &scriptName )
{
#if defined (QT_THREAD_SUPPORT) && QT_VERSION >= 0x030300
    if (context
	&& context->inherits("QWidget")
	&& qt_get_application_thread_id() != QThread::currentThread()) {
	qWarning("QSInterpreter::evaluate(), GUI object %s [%s] not allowed in non GUI thread",
		 context->name(), context->className());
	return QSArgument();
    }
#endif
    running = TRUE;
    startInterpreter();
    QSArgument v = d->interpreter->execute( context, code, scriptName );
    running = FALSE;
    stopInterpreter();
    return v;
}


/*!
  Calls the function \a function with the given \a arguments. The
  arguments are first converted into Qt Script datatypes.

  Functions which were passed to evaluate() in previous calls or which
  are defined in the current project, can be called from this
  function.

  If \a context is 0 (the default), the function is called in the
  global scope. If a \a context is given, the function is called in
  the scope of that object.

  Interpreters that belong to a project are subject to re-evaluation,
  since the code which has been passed previously into evaluate() gets
  lost when calling one of these functions. This happens when the project
  or the scripts in it are modified.
*/
QSArgument QSInterpreter::call( const QString &function,
				const QSArgumentList &arguments,
				QObject *context )
{
#if defined (QT_THREAD_SUPPORT) && QT_VERSION >= 0x030300
    if (context
	&& context->inherits("QWidget")
	&& qt_get_application_thread_id() != QThread::currentThread()) {
	qWarning("QSInterpreter::evaluate(), GUI object (%s [%s]) not allowed in non GUI thread",
		 context->name(), context->className());
	return QSArgument();
    }
#endif
    running = TRUE;
    startInterpreter();
    if( function.isEmpty() ) {
#if defined QT_RANGE_CHECK
	qWarning( "QSInterpreter::call(), function name is empty" );
#endif
        return QSArgument();
    }
    QSArgument v = d->interpreter->call( context, function, arguments );
    running = FALSE;
    stopInterpreter();
    return v;
}


/*!
  Checks whether the script code \a code is free of syntax errors.
  Returns TRUE if the code is free of syntax errors; otherwise returns
  FALSE.
*/
bool QSInterpreter::checkSyntax( const QString &code )
{
    bool ok = d->interpreter->checkSyntax( code );
    if (!ok)
        parseError();
    return ok;
}


/*!
  Returns the current execution context of the interpreter. This is
  either a QObject pointer or 0.
*/
QObject *QSInterpreter::currentContext() const
{
    QSObject o = d->interpreter->env()->currentScope();
    if ( o.isA( d->interpreter->wrapperClass() ) )
	return d->interpreter->wrapperClass()->shared( &o )->objects[0];
    return 0;
}


/*!
  Returns the current project of the interpreter or 0 if there is
  no project.
*/
QSProject *QSInterpreter::project() const
{
    return d->project;
}


/*! \internal */
QuickInterpreter *QSInterpreter::interpreter() const
{
    return d->interpreter;
}


/*!
  Returns the list of objects currently available for scripting in
  this interpreter.
*/
QObjectList QSInterpreter::presentObjects() const
{
    startInterpreter();
    QObjectList *lst = interpreter()->topLevelObjects();
    stopInterpreter();
    return lst ? *lst : QObjectList();
}


/*!
  Clears the state of the interpreter.

  When the interpreter is cleared, all declarations parsed using the
  function QSInterpreter::evaluate() are removed. The state of all
  variables will also be cleared.

  Clearing the interpreter will also remove any transient
  objects. Transient objects are those added with the function
  QSInterpreter::addTransientObject() or by evaluating code in the
  context of a QObject using QSInterpreter::evaluate();

  This function does not clear persistent application objects added
  by the function QSProject::addObject().
*/
void QSInterpreter::clear()
{
    if( running ) {
#if defined( QT_RANGE_CHECK )
	qWarning( "QSinterpreter::clear(), cannot clear while executing" );
#endif
	return;
    }
    d->interpreter->clear();
    if( d->project ) {
	d->project->invalidateProject();
    }
}


/*!
  Returns all the functions in the global context.

  If \a flags includes \c FunctionSignatures, then each function name
  returned will be of the following form:
  \code
  functionName( typeOfArg1, typeOfArg2, ... )
  \endcode
  Otherwise just the names will be returned (which is the default).

  \sa classes(), variables()
*/
QStringList QSInterpreter::functions( FunctionFlags flags ) const
{
    return functions( QString::null, flags );
}


/*!
  Returns all the script functions in the context \a context (this can
  be for example, a class or a form). If \a context is empty, the
  functions of the global context (global functions) are returned.

  \a context can be fully qualified.

  If \a flags includes \c FunctionSignatures, then each function name
  returned will be of the following form:
  \code
  functionName( typeOfArg1, typeOfArg2, ... )
  \endcode
  Otherwise just the names will be returned (which is the default).

  If \a flags includes \c IncludeMemberFunctions and \a context
  represents a class declared in script, this function will return
  both static and non-static functions; otherwise it only returns
  static functions.

  \sa classes(), variables()
*/
QStringList QSInterpreter::functions( const QString &context,
				      uint flags ) const
{
    startInterpreter();
    QSObject obj = d->interpreter->object( context );
    QStringList retVal =
	d->interpreter->functionsOf( obj,
				     flags & FunctionSignatures,
				     FALSE,
				     flags & IncludeMemberFunctions );
    stopInterpreter();
    return retVal;
}


/*!
  Returns all script functions which have been defined in the
  context \a context.

  If \a flags includes \c FunctionSignatures, then each function name
  returned will be of the following form:
  \code
  functionName( typeOfArg1, typeOfArg2, ... )
  \endcode
  Otherwise just the names will be returned (which is the default).

  \sa classes(), variables()
*/
QStringList QSInterpreter::functions( QObject *context,
				      FunctionFlags flags ) const
{
    if( !context ) {
#if defined( QT_RANGE_CHECK )
	qWarning( "QSInterpreter::functions: context is null" );
#endif
	return QStringList();
    }
    startInterpreter();
    QStringList lst;
    QSObject obj = d->interpreter->wrap( context );
    if ( !obj.isUndefined() )
	lst = d->interpreter->functionsOf( obj, flags & FunctionSignatures );
    stopInterpreter();
    return lst;
}


/*!
  Returns the classes in the interpreter.

  If \a flags is \c AllClasses (the default), all the classes in the
  interpreter are returned, including those declared in object
  contexts. If \a flags is \c GlobalClasses, only those classes
  declared in the global context are returned.

  \sa functions(), variables()
*/
QStringList QSInterpreter::classes( ClassFlags flags ) const
{
    startInterpreter();
    QStringList lst;
    if( flags == AllClasses ) {
	lst = d->interpreter->classes();
    } else {
	QSObject obj = d->interpreter->object( QString::null );
	lst = d->interpreter->classesOf( obj );
    }
    stopInterpreter();
    return lst;
}


/*!
  Returns all the classes declared in the fully qualified context \a
  context.

  \sa functions(), variables()
*/
QStringList QSInterpreter::classes( const QString &context ) const
{
    startInterpreter();
    QSObject obj = d->interpreter->object( context );
    QStringList lst = d->interpreter->classesOf( obj );
    stopInterpreter();
    return lst;
}


/*!
  Returns all the classes declared in the context \a context.

  \sa functions(), variables()
*/
QStringList QSInterpreter::classes( QObject *context ) const
{
    if( !context ) {
#if defined( QT_RANGE_CHECK )
	qWarning( "QSInterpreter::classesOf: context is null" );
#endif
	return QStringList();
    }
    startInterpreter();
    QSObject obj = d->interpreter->wrap( context );
    QStringList lst;
    if ( !obj.isUndefined() )
	lst = d->interpreter->classesOf( obj );
    stopInterpreter();
    return lst;
}


/*!
  Returns all the variables declared in the fully qualified
  context \a context.

  \sa functions(), classes()
*/
QStringList QSInterpreter::variables( const QString &context ) const
{
    startInterpreter();
    QSObject obj = d->interpreter->object( context );
    QStringList lst = d->interpreter->variablesOf( obj );
    stopInterpreter();
    return lst;
}


/*!
  Returns all the variables declared in the contenxt \a context.

  \sa functions(), classes()
*/
QStringList QSInterpreter::variables( QObject *context ) const
{
    if( !context ) {
#if defined( QT_RANGE_CHECK )
	qWarning( "QSInterpreter::variablesOf: context is null" );
#endif
	return QStringList();
    }
    startInterpreter();
    QSObject obj = d->interpreter->wrap( context );
    QStringList lst;
    if ( !obj.isUndefined() )
	lst = d->interpreter->variablesOf( obj );
    stopInterpreter();
    return lst;
}


/*!
  Returns all the variables declared in the global context.

  \sa functions(), classes()
*/
QStringList QSInterpreter::variables() const
{
    return variables( QString::null );
}

/*!
  Returns TRUE if the class \a className exists; otherwise returns FALSE.

  The class name can be a fully qualified in the form:
  \code
  myclass.innerClass
  \endcode
*/
bool QSInterpreter::hasClass(const QString &className) const
{
    startInterpreter();
    bool result;
    if (className.isEmpty()) {
#if defined( QT_CHECK_STATE )
	qWarning( "QSInterpreter::hasClass(): class name is empty" );
#endif
	result = FALSE;
    } else {
	result = d->interpreter->hasClass(className);
    }
    stopInterpreter();
    return result;
}

/*!
  Returns TRUE if the variable \a variable exists; otherwise returns FALSE.

  The variable name can be fully qualified in the form:
  \code
  myobject.otherobject.var
  \endcode
*/
bool QSInterpreter::hasVariable(const QString &variable) const
{
    startInterpreter();
    bool result;
    if (variable.isEmpty()) {
#if defined( QT_CHECK_STATE )
	qWarning( "QSInterpreter::hasVariable(): variable name is empty" );
#endif
	result = FALSE;
    } else {
	result = d->interpreter->hasVariable(variable);
    }
    stopInterpreter();
    return result;
}

/*!
  Returns TRUE if the function \a function exists; otherwise returns FALSE.

  The function can be a fully qualified in the form:
  \code
  myclass.function
  \endcode
*/
bool QSInterpreter::hasFunction(const QString &function) const
{
    startInterpreter();
    bool result;
    if (function.isEmpty()) {
#if defined( QT_CHECK_STATE )
	qWarning( "QSInterpreter::hasFunction(): function name is empty" );
#endif
	result = FALSE;
    } else {
	result = d->interpreter->hasFunction(function);
    }
    stopInterpreter();
    return result;
}

/*!
  Adds the variable \a variableName to the scope \a context. The
  variable is given the value \a arg. If no context is specified, the
  global scope is used.

  The variable will persist until the interpreter is cleared. It is for that
  reason not wise to use this function on an interpreter that belongs to
  a QSProject

  \sa QSProject, addTransientObject()
*/
void QSInterpreter::addTransientVariable(const QString &variableName, const QSArgument &arg,
					 QObject *context)
{
    if (variableName.isEmpty()) {
#if defined( QT_CHECK_STATE )
	qWarning( "QSInterpreter::setTransientVariable(): variable name is empty");
#endif
	return;
    }
    if (arg.type() == QSArgument::VoidPointer) {
#if defined( QT_CHECK_STATE )
	qWarning( "QSInterpreter::setTransientVariable(): value cannot be void*");
#endif
	return;
    }
    d->interpreter->setVariable(context, variableName, arg);
}

/*!
  Returns the value of the \a variableName in the scope of \a context.
  If the \a context is 0 (the default) the global scope is used. The
  variable name can be fully qualified in the form:

  \code
  a.b.c
  \endcode
*/
QSArgument QSInterpreter::variable(const QString &variableName, QObject *context) const
{
    if (variableName.isEmpty()) {
#if defined( QT_CHECK_STATE )
	qWarning( "QSInterpreter::variable(): variable name is empty");
#endif
	return QSArgument();
    }
    return d->interpreter->variable(context, variableName);
}

/*!
  \internal
*/
void QSInterpreter::parseError()
{
    int sourceId = d->interpreter->debuggerEngine()->sourceId();
    QString msg = d->interpreter->errorMessages().first();
    QString script = d->interpreter->nameOfSourceId( sourceId );
    int line = d->interpreter->errorLines().first();
    runtimeError(msg, script, line);
}


/*!
  \internal
*/
void QSInterpreter::runtimeError()
{
    int l = d->interpreter->debuggerEngine()->lineNumber();
    if ( l < 0 )
	l = d->interpreter->errorLines().first();
    runtimeError( d->interpreter->errorMessages().first(),
		  d->interpreter->nameOfSourceId( d->interpreter->debuggerEngine()->sourceId() ),
		  l );
}


/*!
  \internal
*/
void QSInterpreter::runtimeError( const QString &message,
				  const QString &scriptName,
				  int lineNumber )
{
    emit error( message, scriptName, lineNumber );
    QObject *ctx = d->interpreter->objectOfSourceId( d->interpreter->debuggerEngine()->sourceId() );

    emit error( message, ctx, scriptName, lineNumber );

    if( errorMode() == Notify ) {
	if( qApp->type() == QApplication::Tty
#if defined (QT_THREAD_SUPPORT) && QT_VERSION >= 0x030300
	    || qt_get_application_thread_id() != QThread::currentThread()
#endif
	    ) {
	    qDebug( "Error in script: '%s', line: %d\n  %s\n",
		    scriptName.latin1(), lineNumber, message.latin1() );
	} else {
	    QMessageBox::critical( qApp->mainWidget(), QString::fromLatin1("Error"),
				   QString::fromLatin1("The following error occurred in "
                                           "line <b>%1</b> of  <b>%2</b> while executing "
                                           "the script:<pre><font color=red>%3</font></pre>")
                                           .arg(lineNumber).arg(scriptName).arg(message));
	}
    }
}


/*!
    Informs the interpreter that an error has occurred. The error is
    treated like a normal Qt Script error. The error message is passed
    in \a message.
*/
void QSInterpreter::throwError( const QString &message )
{
    d->interpreter->env()->throwError( message );
}


/*!
  Adds the object factory \a factory to the interpreter.

  Using this function will clear the state of the interpreter, and
  will clear any transient objects.
*/
void QSInterpreter::addObjectFactory( QSObjectFactory *factory )
{
    factory->setInterpreter( this );
    if( factory->interpreter() != this ) {
#if defined( QT_CHECK_STATE )
	qWarning( "QSInterpreter::addObjectFactory(), failed to set interpreter\n" );
#endif
	return;
    }

    if( factory->staticDescriptors().count() == 0 &&
	factory->instanceDescriptors().count() == 0 ) {
#if defined( QT_CHECK_STATE )
	qWarning( "QSInterpreter::addObjectFactory(), factory has no registered classes\n" );
#endif
	return;
    }

    d->interpreter->dispatchObjectFactory()->addObjectFactory( factory );
    d->objectFactories.append( factory );
    if( d->project )
	d->project->invalidateProject();
    else
	clear();
}


/*!
  \internal
 */
void QSInterpreter::removeObjectFactory( QSObjectFactory *factory )
{
    d->interpreter->dispatchObjectFactory()->removeObjectFactory( factory );
    d->objectFactories.removeRef( factory );
}


/*!
  Adds the wrapper factory \a factory to the interpreter.

  Using this function will clear the state of the interpreter, and
  will clear any transient objects.
*/
void QSInterpreter::addWrapperFactory( QSWrapperFactory *factory )
{
    factory->setInterpreter( this );
    if( factory->interpreter() != this ) {
#if defined( QT_CHECK_STATE )
	qWarning( "QSInterpreter::addWrapperFactory(), failed to set interpreter\n" );
#endif
	return;
    }

    if( factory->wrapperDescriptors().count() == 0 ) {
#if defined( QT_CHECK_STATE )
	qWarning( "QSInterpreter::addWrapperFactory(), factory has no registered classes\n" );
#endif
	return;
    }
    d->interpreter->dispatchObjectFactory()->addWrapperFactory( factory );
    d->wrapperFactories.append( factory );
    if( d->project )
	d->project->invalidateProject();
    else
	clear();
}


/*!
  \internal
*/
void QSInterpreter::removeWrapperFactory( QSWrapperFactory *factory )
{
    d->interpreter->dispatchObjectFactory()->removeWrapperFactory( factory );
    d->wrapperFactories.removeRef( factory );
}


/*!
  Returns the error message that was last reported if there was an
  error; otherwise returns QString::null
*/
QString QSInterpreter::errorMessage() const
{
    return hadError()
	? d->interpreter->errorMessages().first()
	: QString::null ;
}


/*!
  Returns the stack trace describing the call stack of the last
  reported error if there was an error; otherwise returns an empty stack
  trace.
*/
QSStackTrace QSInterpreter::stackTrace() const
{
    QSStackTrace st;
    if( !hadError() )
	return st;

    QValueList<QuickDebuggerStackFrame> qtrace = d->interpreter->debuggerEngine()->backtrace();
    for( QValueList<QuickDebuggerStackFrame>::ConstIterator it = qtrace.begin();
	 it != qtrace.end(); ++it ) {
	QSStackFrame frame( (*it).function,
			    d->interpreter->nameOfSourceId( (*it).sourceId ),
			    (*it).line,
			    d->interpreter->objectOfSourceId( (*it).sourceId )
			    );
	st << frame;
    }
    return st;
}


/*!
  Returns TRUE if the interpreter had an error during the last
  execution; otherwise returns FALSE.
*/
bool QSInterpreter::hadError() const
{
    return d->interpreter->hadError();
}


bool operator==( const QSStackFrame &a, const QSStackFrame &b )
{
    return a.line() == b.line() &&
       a.function() == b.function() &&
       a.scriptName() == b.scriptName() &&
       a.context() == b.context();
}


/*!
  Adds the Qt Script function \a qtscriptFunction (fully qualified)
  as a transient signal handler for the C++ signal \a signal of the
  object \a sender.

  Example:
  \code
  interpreter->addTransientSignalHandler( myButton, SIGNAL( clicked() ), "classA.obj.calculate" );
  \endcode

  \sa removeTransientSignalHandler()
*/
void QSInterpreter::addTransientSignalHandler( QObject *sender,
				  const char *signal,
				  const char *qtscriptFunction )
{
    QuickInterpreter *ip = interpreter();
    QString func = QString::fromLatin1( qtscriptFunction );
    func.left( func.find( '(' ) );
    QSObject senderObj = ip->wrap( sender );

    QSObject obj = ip->object( func );
    if( !obj.isFunction() ) {
	qDebug( "QSInterpreter::addTransientSignalHandler(): '%s' not a function",
		qtscriptFunction );
	return;
    }

    QSObject base = QSFuncRefClass::refBase( obj );
    QSMember member = QSFuncRefClass::refMember( obj );
    QSWrapperShared *sh = ip->wrapperClass()->shared( &senderObj );
    if (!sh->setEventHandler( ip, QString::fromLatin1(signal + 1), 0, member.name(), base )) {
#if defined( QT_CHECK_STATE )
	qWarning("QSInterpreter::addTransientSignalHandler(), "
		 "failed to add signal handler: '%s' to '%s'", signal+1, qtscriptFunction);
#endif
    }
}


/*!
  Removes the connection between the signal \a signal of the object
  \a sender and the fully qualified signal handler \a
  qtscriptFunction.

  \sa addTransientSignalHandler()
*/
void QSInterpreter::removeTransientSignalHandler( QObject *sender, const char *signal,
				     const char *qtscriptFunction )
{
    QuickInterpreter *ip = interpreter();
    QString func = QString::fromLatin1( qtscriptFunction );
    func.left( func.find( '(' ) );
    QSObject senderObj = ip->wrap( sender );

    QSObject obj = ip->object( func );
    QSObject base = QSFuncRefClass::refBase( obj );
    QSMember member = QSFuncRefClass::refMember( obj );
    QSWrapperShared *sh = ip->wrapperClass()->shared( &senderObj );
    if (!sh->removeEventHandler( QString::fromLatin1(signal + 1), 0, member.name(), base )) {
#if defined( QT_CHECK_STATE )
	qWarning("QSInterpreter::removeTransientSignalHandler(), "
		 "failed to remove signal handler: '%s' to '%s'", signal+1, qtscriptFunction);
#endif
    }
}


/*!
  \property QSInterpreter::timeoutInterval

  \brief Describes the number of milliseconds between each time the \l
  timeout() signal will be emitted by the interpreter.

  The timeout can be used to perform monitoring of the interpreter,
  such as forcing it to terminate if it has not terminated after 10
  seconds. A negative value, the default, will turn off the \l
  timeout() signal.

  \sa timeout()
*/

void QSInterpreter::setTimeoutInterval(int msecs)
{
    d->timeoutInterval = msecs;
    if (d->timeoutInterval < 0) {
	interpreter()->setTimeoutTriggerEnabled(FALSE);
    } else {
	interpreter()->setTimeoutTriggerEnabled(TRUE);
	QSTimeoutTrigger *trigger = interpreter()->timeoutTrigger();
	trigger->setInterval(msecs);
	connect(trigger, SIGNAL(timeout(int)), this, SIGNAL(timeout(int)));
    }
}

int QSInterpreter::timeoutInterval() const
{
    return d->timeoutInterval;
}


/*!
  Stops a running interpreter by throwing an error.
*/
void QSInterpreter::stopExecution()
{
    throwError(QString::fromLatin1("Execution terminated"));
}


/*!
  Convenience function for describing the stack trace.

  Each stack frame has the following format:

  \code
  function(scriptName:lineNumber) context
  \endcode
 */
QString QSStackTrace::toString() const
{
   QStringList lst;
   const char *ctx;
   QString str;
   for( QSStackTrace::ConstIterator it = begin();
	it != end(); ++it ) {
       if( (*it).context() )
	   ctx = str.sprintf( "%s [%s]",
			      (*it).context()->name(),
			      (*it).context()->metaObject()->className() ).latin1();
       else if (!(*it).scriptName().isNull())
	   ctx = "(global context)";
       else
           ctx = "<unknown>";
       lst << QString().sprintf( "  %s(%s:%d) %s",
				 (*it).function().latin1(),
				 (*it).scriptName().isNull()
				 ? "<unknown>"
				 : (*it).scriptName().latin1(),
				 (*it).line(),
				 ctx );
   }
   return lst.join( QString::fromLatin1("\n") );
}


#ifdef QSA_EXPIRES
#include <qsettings.h>
#include <qregexp.h>
void qsa_eval_check()
{
    QSettings settings;
    settings.insertSearchPath(QSettings::Windows, "/Trolltech");
    QString key = QString("/QSA/%1/expdt").arg(QSA_VERSION_STRING);

    bool ok;
    bool showWarning = FALSE;
    QString str = settings.readEntry(key, QString::null, &ok);
    if (ok) {
	QRegExp exp("(\\w\\w\\w\\w)(\\w\\w)(\\w\\w)");
	Q_ASSERT(exp.isValid());
	if (exp.search(str)>=0) {
	    int a = exp.cap(1).toInt(0,16);
	    int b = exp.cap(2).toInt(0,16);
	    int c = exp.cap(3).toInt(0,16);
	    showWarning = (QDate::currentDate() > QDate(a, b, c));
	} else {
	    showWarning = TRUE;
	}
    } else {
	QDate date = QDate::currentDate().addMonths(1);
	QString str = QString().sprintf("%.4x%.2x%.2x", date.year(), date.month(), date.day());
	settings.writeEntry(key, str);
    }

    if (showWarning) {
	QMessageBox::warning(0,
			     QObject::tr("End of Evaluation Period" ),
			     QObject::tr("The evaluation period of QSA has expired.\n\n"
					 "Please check http://www.trolltech.com/products/qsa"
					 "for updates\n"
					 "or contact sales@trolltech.com for further information"),
			      QMessageBox::Ok,
			      QMessageBox::NoButton);
    }
}
#endif
