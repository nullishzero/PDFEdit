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

#include "qseditor.h"
#include "qsinterpreter.h"
#include "qsproject.h"
#include <QTextStream>
#include <QList>
#include <QBuffer>
#include <QDataStream>
#include <QDir>
#include <QFile>
#include <QCoreApplication>
#include <QStringList>
#include <QThread>

#include "../engine/qsfuncref.h"
#include "../kernel/quickdebugger.h"
#include "../kernel/quickinterpreter.h"
#include "../kernel/quickobjects.h"

#include <stdlib.h>

bool qsa_is_non_gui_thread() {
    return QCoreApplication::instance() == 0 ||
           QCoreApplication::instance()->thread() != QThread::currentThread();
}

#define getQuickInterpreter() d->interpreter->interpreter()

struct QSSignalHandler
{
    QObject *sender, *receiver;
    QString signal, function;
    bool operator==( const QSSignalHandler &sh ) const {
	return ( sh.sender == sender &&
		 sh.receiver == receiver &&
		 sh.signal == signal &&
		 sh.function == function );
    }
};

class QSProjectPrivate {
public:
    QSProjectPrivate() :
	interpreter( 0 ),
	needsRerun( false ),
	bundleStorage(false)
    {
    }

    QSInterpreter *interpreter;
    QList<QSScript *> scripts;

#ifndef QSA_NO_EDITOR
    QList<QSEditor *> editors;
#endif

    QObjectList objects;
    QList<QSSignalHandler> signalHandlers;
    QString loadName;
    uint needsRerun : 1;
    uint scriptsModified : 1;
    uint bundleStorage : 1; // tells saveInternal/loadInternal to store as binary bundle
};

/*!
  \class QSProject qsproject.h

  \brief The QSProject class provides project management over scripts
  written in Qt Script for Applications.

  The project's interpreter can be accessed through the function
  interpreter().

  Scripts are encapsulated as instances of the class QSScript. QSScript
  objects are created by using the createScript() functions. It is
  possible to query the project for scripts using the functions scripts(),
  scriptNames(), and script().

  It is possible to associate a QSScript with a context object,
  meaning that the script will be evaluated in the context of that
  object. If a script and an object have the same name, they are
  grouped together.

  Objects are added to the project with addObject() and removed from
  the project with removeObject(). To query the project's objects, use
  the object() and objects() functions. Note that objects added to the
  project become persistent, meaning that when the interpreter is
  cleared, they will still be scriptable. This is in contrast to
  objects added to the interpreter using the
  QSInterpreter::addTransientObject() function, which will not be
  available after the interpreter is cleared.

  A project can be stored on disk and loaded again using the load()
  and save() functions. It is also possible to write the project to a
  data buffer that can be used in conjunction with other applications
  by using the loadFromData() and saveToData() functions.

  The project also manages editors for the scripts it holds. Editors
  can be created using createEditor(), and can be queried using the
  editors(), editor(), and activeEditor() functions. When text changes
  in the interpreter, the editorsModified() signal is emitted. Before
  the changes are reflected in the script, the commitEditorContents()
  function must be called. To revert the editors to the code in the
  scripts use revertEditorContents().

  When the editors are committed, the scripts will be out of sync with
  the state of the interpreter. The scriptsModified() signal is
  emitted to indicate this situation.

  Several actions in the interpreter will trigger re-evaluation of the
  project; consequently, the interpreter will be cleared and the
  scripts re-evaluated. Such actions include modifying scripts and
  removing objects. For this reason it is unwise to have executing
  script code (such as calling application functionality) outside of
  functions when using a QSProject. Non-executing script code (such as
  declaration of global variables) is safe.

  The addSignalHandler() and removeSignalHandler() functions can
  be used to connect and disconnect signals and slots from C++ to
  Qt Script.

  Note on threading; The QSProject class cannot be used outside
  the GUI thread.

  Objects of the QSProject class require an exising QApplication prior
  to be instantiated.

  See the \l {Getting Started with QSA} for more explanations and
  examples.

  \sa QSScript, QSEditor, QSInterpreter
*/

/*! \enum QSProject::StorageMode

  The StorageMode enum describes the format of the project files when
  they are stored with QSProject::save()

  \value Bundle The project file is saved as one binary bundle. This
  format is easily portable, but the scripts cannot be edited in an
  external editor (This was the default for QSA 1.0).

  \value TextFiles The project file stores only the script file name,
  and saves the script file as a separate file. This allows editing of
  script files with an external editor. This is the default for QSA
  1.1 and later.
*/


/*!
  \fn QSProject::editorTextChanged()

  This signal is emitted whenever text changes in one of the editors
  that the project currently manages.
*/

/*!
  \fn QSProject::projectChanged()

  This signal is emitted whenever the project has changed in such a
  way that it is out of sync with the interpreter and needs to be
  re-evaluated before the interpreter is used.
*/

/*!
  \fn QSProject::projectEvaluated()

  This signal is emitted whenever the project has been evaluated and
  the interpreter back in sync with the scripts in the project.
*/


/*!
  Constructs a QSProject with parent \a parent being
  passed to the QObject constructor. The constructor will
  set the project's object name to \a name.
*/
QSProject::QSProject( QObject *parent, const char *name )
    : QObject( parent )
{
    setObjectName(name);

    if (qsa_is_non_gui_thread()) {
        qWarning("QSProject::QSProject(), project cannot be used in non GUI thread");
    }
    d = new QSProjectPrivate;
    d->interpreter = new QSInterpreter( this );
    connect( this, SIGNAL( projectChanged() ),
        this, SLOT( invalidateProject() ) );
}


/*!
  \internal
*/
QSProject::~QSProject()
{
    delete d;
}


/*!
  Reads and parses the Qt Script for Applications project contained in
  the file, \a projectFile.

  If an error occurs while running or parsing the script, actions
  according to the errorMode() are taken and the error() signal is
  emitted.

  After load() is called, functions and classes defined in this
  project can be called and used through the evaluate() and call()
  functions.

  If the project has signal/slot connections, for example, a form that
  connects widget signals to Qt Script slots, then the connections
  are established as part of the open() process, if the objects are
  already known to the interpreter via QSProject::addObject() or
  QSInterpreter::addTransientObject().
*/
bool QSProject::load( const QString &projectFile )
{
    QFile file( projectFile );
    d->loadName = projectFile;
    if( file.open( QIODevice::ReadOnly ) ) {
	QDataStream input( &file );
	return loadInternal( &input );
    }
#if defined(QT_CHECK_RANGE)
    qWarning( "Could not open project file" );
#endif
    return false;
}


/*!
  Reads and parses the Qt Script for Applications project from \a
  projectData.

  If an error occurs while running or parsing the script, actions
  according to the errorMode() are taken and the error() signal is
  emitted.

  After loadFromData() is called, functions and classes defined in this
  project can be called and used through the evaluate() and call()
  functions.

  If the project has signal/slot connections, for example, a form that
  connects widget signals to Qt Script slots, then the connections
  are established as part of the process.
*/
bool QSProject::loadFromData( QByteArray *projectData )
{
    QDataStream stream( projectData, QIODevice::ReadOnly );
    return loadInternal( &stream );
}


/*!
  Saves the scripts project to the file \a projectFile

  The default for \a projectFile is QString::null, in which
  case the filename used during load will be used.

  \sa load()
*/
bool QSProject::save( const QString &projectFile )
{
    QString fileName = projectFile.isNull() ? d->loadName : projectFile;
    QFile file( fileName );
    if( !file.open( QIODevice::WriteOnly ) ) {
#if defined(QT_CHECK_RANGE)
	qWarning( "QSProject::save(), could not open file for writing" );
#endif
	return false;
    }

    QDataStream output( &file );
    return saveInternal( &output );
}


/*!
  Writes the the whole project to the data block \a data.

  This is useful for keeping the project data with a document. For
  example, you can take the data returned by this function and store
  it in a document's file. Later, when the document is opened, you can
  extract the data from the document and pass it to the overload of
  openFromData() that takes a QByteArray to reconstruct the project.

  \sa loadFromData()
*/
bool QSProject::saveToData( QByteArray *data )
{
    QDataStream stream( data, QIODevice::WriteOnly );
    bool old = d->bundleStorage;
    d->bundleStorage = true;
    bool result = saveInternal( &stream );
    d->bundleStorage = old;
    return result;
}


/*!
  Returns the interpreter which is used to evaluate code
  for this project.
*/
QSInterpreter* QSProject::interpreter() const
{
    return d->interpreter;
}


/*! \fn QSEditor *QSProject::activeEditor() const
  Returns the editor which is currently editing the project; if no
  editor is active returns 0.
*/
#ifndef QSA_NO_EDITOR
QSEditor *QSProject::activeEditor() const
{
    for (int i=0; i<d->editors.count(); ++i) {
        QSEditor *e = d->editors[i];
        if (e->isActive())
            return e;
    }
    return 0;
}
#endif


/*!
  Returns the editor for the script \a script if the editor
  exists; otherwise returns 0.
 */
#ifndef QSA_NO_EDITOR
QSEditor *QSProject::editor( QSScript *script ) const
{
    if( d->scripts.indexOf( script ) < 0 )
	    return 0;

    QSEditor *editor = 0;
    for (int i=0; i<d->editors.count(); ++i) {
        QSEditor *current = d->editors[i];
	    if (current->script() == script)
	        editor = current;
    }

    return editor;
}
#endif


/*!
  Creates a new editor for the script \a script with the parent
  \a parent and object name \a name.

  If \a script does not belong to the current project, 0 is returned.

  If an editor already exists for the given script, the existing editor
  is returned.
*/
#ifndef QSA_NO_EDITOR
QSEditor *QSProject::createEditor( QSScript *script, QWidget *parent, const char *name )
{
    if( d->scripts.indexOf( script ) < 0 )
	    return 0;

    QSEditor *ed = editor( script );
    if( ed )
	return ed;

    ed = new QSEditor( parent, name );
    ed->setScript( script ); // registerEditor is called from setScript
    return ed;
}
#endif


/*!
  Returns a list of all the editors for the scripts in this
  project.
*/
#ifndef QSA_NO_EDITOR
QList<QSEditor *> QSProject::editors() const
{
    return d->editors;
}
#endif


/*!
  Returns a list of all the scripts in this project.
*/
QList<QSScript *> QSProject::scripts() const
{
    return d->scripts;
}


/*!
  Returns the names of all the scripts in this project.
 */
QStringList QSProject::scriptNames() const
{
    QStringList lst;
    QListIterator<QSScript *> it( d->scripts );
    QSScript *s;
    while(it.hasNext() && (s = it.next()))
        lst << s->name();

    return lst;
}


/*!
  Returns the script called \a name if it exists in the
  project; otherwise returns 0.
*/
QSScript *QSProject::script( const QString &name ) const
{
    QListIterator<QSScript *> it( d->scripts );
    QSScript *s;
    while(it.hasNext() && (s = it.next())) {
        if( s->name() == name )
            return s;
    }
    return 0;
}


/*!
  Returns the script that has \a context as context if it
  exists in the project; otherwise returns 0.
*/
QSScript *QSProject::script( QObject *context ) const
{
    QListIterator<QSScript *> it( d->scripts );
    QSScript *s;
    while(it.hasNext() && (s = it.next())) {
        if( s->context() == context )
            return s;
    }
    return 0;
}


/*!
  Returns the persistent object in the project called \a name.
*/
QObject *QSProject::object( const QString &name ) const
{
    for (int i=0; i<d->objects.size(); ++i) {
        QObject *object = d->objects[i];
        if (object->objectName() == name)
            return object;
    }
    return 0;
}


/*!
  Returns the list of persistent objects in the project.
*/
QObjectList QSProject::objects() const
{
    return d->objects;
}


/*!
  Creates a script with context \a context and source code \a code.

  The context is added to this project's list of objects.

  If a script already exists for this context, 0 is returned.
*/
QSScript *QSProject::createScript( QObject *context, const QString &code )
{
    if( !context ) {
#if defined(QT_CHECK_RANGE)
	qWarning( "QSProject::createScript(), context is null" );
#endif
	return 0;
    }

#if defined(QT_CHECK_RANGE)
    if ( strcmp( context->name(), "unnamed" ) == 0 )
	qWarning( "QSProject::addObject: unnamed object" );
#endif

    return createScriptInternal( context->objectName(), code, context );
}


/*!
  Creates a script called \a name and source code \a code without
  a context.

  If an object exists in the project with \a name as its object name,
  the object is associated with the created script and the script will
  be evaluated in the context of that object.

  If a script named \a name already exists, 0 is returned.
*/
QSScript *QSProject::createScript( const QString &name, const QString &code )
{
    if( name.isEmpty() ) {
#if defined(QT_CHECK_RANGE)
	qWarning( "QSProject::createScript(), name cannot be empty!" );
#endif
	return 0;
    }
    return createScriptInternal( name, code, object(name) );
}


/*!
  Makes the QObject \a object available to the scripting engine. All child
  objects of \a object are also made available, recursivly.

  If no object in the parent hierarchy of \a object has been added via
  addObject(), \a object will be made available as a toplevel object to
  the programmer and will be accessible via \c Application.object_name
  (where \c object_name is the value of \i object's QObject::objectName()
  property).

  If an object in the parent hierarchy of \a object has been made available
  via addObject(), \a object will not be made available as a
  toplevel object. It is then accessible through \c
  parent1.parent2.object_name in the scripting language, assuming that \c
  parent1 has previously been made available via addObject().

  Objects added with this function are persistent. This means that
  when the interpreter is cleared, either by calling QSInterpreter::clear()
  or by modifying the scripts, these objects will remain available
  to the scripting engine.

  If a script exists in the project that has the same name as \a object,
  the script will be evaluated with \a object as context.

  Note on threading; If the interpreter is running in the non-GUI thread,
  \a object cannot be a QWidget subclass.

  \warning Every object passed to this function must have a unique
  object name.

  \sa QSInterpreter::addTransientObject(), removeObject(),
  clearObjects(), object()
*/
void QSProject::addObject( QObject *object )
{
    if (!object) {
#if defined(QT_CHECK_RANGE)
        qWarning( "QSProject::addObject(), object is null" );
#endif
        return;
    }
#if defined(QT_CHECK_RANGE)
    if ( strcmp( object->name(), "unnamed" ) == 0 ) {
        qWarning( "QSProject::addObject: unnamed object" );
        return;
    }
#endif
    if (object->isWidgetType() && qsa_is_non_gui_thread()) {
        qWarning("QSInterpreter::evaluate(), GUI object %s [%s] not allowed in non GUI thread",
            qPrintable(object->objectName()), object->metaObject()->className());
        return;
    }

    if (getQuickInterpreter()->hasTopLevelParent(object)) {
#ifndef QT_NO_DEBUG
        qWarning("QSProject::addObject(): "
                 "An ancestor of %s [%s] is already present in the interpreter",
		 object->objectName().toLatin1().constData(), object->metaObject()->className());
#endif
        return;
    }

    connect( object, SIGNAL( destroyed() ),
        this, SLOT( objectDestroyed() ) );
    d->objects.append( object );
    QSScript *s = script(object->objectName());
    if( s )
        s->setContext( object );

    getQuickInterpreter()->addTopLevelObject( object );
    emit projectChanged();
}


/*!
  Removes the persistent QObject \a object from the list of
  available objects for scripting.

  Note that only persistent objects can be removed with
  this function. Objects added to the interpreter using
  QSInterpreter::addTransientObject can not be removed with this
  function.

  If \a object is the context of a script, the script will
  also be removed from the project.

  \sa addObject(), clearObjects(), object()
*/
void QSProject::removeObject( const QObject *object )
{
    if( !object ) {
#if defined(QT_CHECK_RANGE)
	    qWarning( "QSProject::removeObject(), object is null" );
#endif
	    return;
    }
    QList<QSSignalHandler>::Iterator it = d->signalHandlers.begin();
    while ( it != d->signalHandlers.end() ) {
	    QList<QSSignalHandler>::Iterator it2 = it;
	    QSSignalHandler sigh = *it2;
	    ++it;
	    if ( sigh.sender == object || sigh.receiver == object )
	        d->signalHandlers.erase( it2 );
    }
    disconnect( object, SIGNAL( destroyed() ),
		this, SLOT( objectDestroyed() ) );
    d->objects.removeAll(const_cast<QObject *>(object));
    QSScript *s = script(object->objectName());
    if( s )
	    d->scripts.removeAll( s );

    emit projectChanged();
}


/*!
  Removes all application objects from the list of available
  objects.

  QSProject does not take ownership of the objects, so the objects
  are not deleted.

  \sa addObject(), removeObject(), object(), QSInterpreter::addTransientObject()
*/
void QSProject::clearObjects()
{
    while( d->objects.count() )
	    removeObject( d->objects.last() );
    emit projectChanged();
}

/*!
  Adds the Qt Script function \a qtscriptFunction in the context of
  \a receiver as a signal handler for the C++ signal \a signal of the
  object \a sender.

  Example:
  \code
  project->addSignalHandler(myButton, SIGNAL(clicked()), document, "startCalculation");
  \endcode

  \sa removeSignalHandler()
*/
void QSProject::addSignalHandler( QObject *sender,
				  const char *signal,
				  QObject *receiver,
				  const char *qtscriptFunction )
{
    QString func( QString::fromLatin1(qtscriptFunction) );
    func.left( func.indexOf( '(' ) );
    QSSignalHandler sigh;
    sigh.sender = sender;
    sigh.receiver = receiver;
    sigh.signal = signal;
    sigh.function = func;
    if (d->signalHandlers.contains(sigh)) {
#ifdef QT_CHECK_RANGE
	qWarning("Signal handler already present\n  "
		 "sender: %s, signal: %s, receiver: %s, slot: %s",
		 sender->name(), signal+1,
		 receiver ? receiver->name() : "(null)", qtscriptFunction);
#endif
	return;
    }

    evaluate();
    QuickInterpreter *ip = getQuickInterpreter();
    QSObject function_ref;

    if (receiver) {
        QSObject receiver_object = ip->wrap(receiver);
        function_ref = receiver_object.get(qtscriptFunction);
    } else { // same approach as below
	function_ref = ip->object( func );
    }

    if(!function_ref.isValid() || !function_ref.isFunction()) {
#ifdef QT_CHECK_RANGE
        qWarning("QSProject::addSignalHandler(): '%s' not a function", qtscriptFunction);
        return;
#endif
    }

    bool added = ip->addConnection(sender, signal, function_ref);

    if (!added) {
#ifdef QT_CHECK_RANGE
	    qWarning("QSProject::addSignalHandler(): failed to add signal handler '%s' to '%s'",
		    signal+1, qtscriptFunction);
#endif
	    return;
    }

    d->signalHandlers << sigh;
    connect( sender, SIGNAL( destroyed() ), this, SLOT( objectDestroyed() ) );
    if (receiver)
	connect( receiver, SIGNAL( destroyed() ), this, SLOT( objectDestroyed() ) );
    emit projectChanged();
}


/*!
  Adds the Qt Script function \a qtscriptFunction (fully qualified)
  as a signal handler for the C++ signal \a signal of the object \a
  sender.

  Example:
  \code
  project->addSignalHandler(myButton, SIGNAL(clicked()), "a.b.startCalculation");
  \endcode

  \sa removeSignalHandler()
*/
void QSProject::addSignalHandler( QObject *sender,
				  const char *signal,
				  const char *qtscriptFunction )
{
    addSignalHandler(sender, signal, 0, qtscriptFunction);
}


/*!
  Removes the connection between the signal \a signal of the object
  \a sender and the signal handler \a qtscriptFunction in the
  context \a receiver.

  \sa addSignalHandler()
*/
void QSProject::removeSignalHandler( QObject *sender, const char *signal,
                                     QObject *receiver,
                                     const char *qtscriptFunction )
{
    evaluate();
    QString func( QString::fromLatin1(qtscriptFunction) );
    QuickInterpreter *ip = d->interpreter->interpreter();

    QSObject function_ref;

    if (receiver) {
        QSObject receiver_object = ip->wrap(receiver);
        function_ref = receiver_object.get(qtscriptFunction);
    } else {
        function_ref = ip->object(qtscriptFunction);
    }

    if(!function_ref.isValid() || !function_ref.isFunction()) {
        qWarning("QSProject::addSignalHandler(): '%s' not a function", qtscriptFunction);
        return;
    }

    if (!ip->removeConnection(sender, signal, function_ref)) {
        qWarning("QSProject::removeSignalHandler(): "
            "failed to remove signal handler '%s' to '%s'",
            signal+1, qtscriptFunction);
        return;
    }

    QSSignalHandler sigh;
    sigh.sender = sender;
    sigh.receiver = receiver;
    sigh.signal = signal;
    sigh.function = func.left(func.indexOf('('));

    for (QList<QSSignalHandler>::Iterator it = d->signalHandlers.begin();
          it != d->signalHandlers.end(); ++it) {
        if ( sigh == *it ) {
            d->signalHandlers.erase(it);
            break;
        }
    }
}


/*!
  Removes the connection between the signal \a signal of the object
  \a sender and the fully qualified signal handler \a
  qtscriptFunction.

  \sa addSignalHandler()
*/
void QSProject::removeSignalHandler( QObject *sender, const char *signal,
				     const char *qtscriptFunction )
{
    removeSignalHandler(sender, signal, 0, qtscriptFunction);
}


/*!
  Returns true if the project is modified; otherwise returns false.

  A project enters the modified state when the editors and scripts
  become out of sync. This will occur, for example, when a user edits
  a script in an editor. Before the interpreter is used, this value
  should be checked and the application should decide to either commit
  or revert the changes in the editor before the project is evaluated.

  \sa commitEditorContents(), revertEditorContents()
*/
#ifndef QSA_NO_EDITOR
bool QSProject::editorsModified() const
{
    for (int i=0; i<d->editors.count(); ++i) {
        QSEditor *current = d->editors[i];
	    if( current->isModified() )
	        return true;
    }
    return false;
}
#endif


/*!
  Updates the scripts with the content of the editors.

  For each editor that is changed, commit() is called, propagating
  the changes in the editor into the script it is currently editing.

  After a call to commitEditorContents, the project will no longer be
  modified.

  \sa editorsModified(), revertEditorContents()
 */
#ifndef QSA_NO_EDITOR
void QSProject::commitEditorContents()
{
    for (int i=0; i<d->editors.count(); ++i) {
        QSEditor *current = d->editors[i];
	    if( current->isModified() )
	        current->commit();
    }
}
#endif


/*!
  Reverts the text in the editors to the state of the scripts.

  After a call to revertEditorContents, the project will no longer
  be modified.

  \sa editorsModified(), commitEditorContents()
 */
#ifndef QSA_NO_EDITOR
void QSProject::revertEditorContents()
{
    for (int i=0; i<d->editors.count(); ++i) {
        QSEditor *current = d->editors[i];
	    if( current->isModified() )
	        current->revert();
    }
}
#endif


/*!
  \internal

  Evaluates all scripts in the project.

  This function clears the state of the interpreter and reevaluates
  all the scripts in the project if needed. The scripts are evaluated
  in their current state, which means that any change made in an
  editor that is not committed will not be taken into account.

  Reevaluation is needed whenever a script has been added, changed or
  removed. Reevaluation is also needed when editors have been committed, persistent objects are added,
  removed or cleared and when signal handlers are modified.
*/
void QSProject::evaluate()
{
    if( !d->needsRerun )
	return;
    d->needsRerun = false;

    if (qsa_is_non_gui_thread()) {
	qWarning("QSProject::evaluate(), project cannot be used in non GUI thread");
	return;
    }

    d->interpreter->running = true;

    QuickInterpreter *ip = getQuickInterpreter();

    ip->clear();
    initObjects();

#ifndef QSA_NO_EDITOR
    for (int i=0; i<d->editors.count(); ++i) {
        QSEditor *editor = d->editors[i];
	    editor->removeErrorMark();
    }
#endif

    QListIterator<QSScript *> it( d->scripts );
    QSScript *script;
    while(it.hasNext() && (script = it.next())) {
	    ip->execute( script->context(), script->code(), script->name() );
	    if( ip->hadError() ) {
                d->interpreter->running = false;
                return;
            }
    }

    initEventHandlers();

    d->interpreter->running = false;

    emit projectEvaluated();
}


/*!
  Returns true if the script has been modified and is out of sync
  with the state in the interpreter; otherwise returns false.
*/
bool QSProject::scriptsModified() const
{
    return d->needsRerun;
}


/*!
  Sets the mode for storing projects to \a mode.

  \sa StorageMode
*/
void QSProject::setStorageMode(StorageMode mode)
{
    d->bundleStorage = (mode == Bundle);
}


/*!
  Returns the storage mode used for this project.

  \sa StorageMode
*/
QSProject::StorageMode QSProject::storageMode() const
{
    return d->bundleStorage ? Bundle : TextFiles;
}

/*
   \internal
*/
void QSProject::invalidateProject()
{
    d->needsRerun = true;
}


/*! \internal */
void QSProject::initObjects()
{
    QuickInterpreter *ip = getQuickInterpreter();
    for (int i=0; i<d->objects.size(); ++i) {
        QObject *o = d->objects.at(i);
        if (ip->hasTopLevelParent(o)) {
#ifndef QT_NO_DEBUG
            qWarning("QSProject::initObjects(): "
                     "An ancestor of %s [%s] is already present in the interpreter",
                     o->objectName().toLatin1().constData(), o->metaObject()->className());
#endif
            continue;
        }
        ip->addTopLevelObject(o);
    }
}


/*! \internal */
void QSProject::initEventHandlers()
{
    QuickInterpreter *ip = getQuickInterpreter();
    for ( QList<QSSignalHandler>::Iterator it = d->signalHandlers.begin();
	  it != d->signalHandlers.end(); ++it ) {
	QSSignalHandler sigh = *it;
        QSObject function_ref;
	if ( sigh.receiver ) {
            QSObject receiver_object = ip->wrap(sigh.receiver);
            function_ref = receiver_object.get(sigh.function);
	} else {
	    function_ref = ip->object( sigh.function );
	}
        Q_ASSERT(function_ref.isValid());
        Q_ASSERT(function_ref.isFunction());
        ip->addConnection(sigh.sender, sigh.signal.toLatin1().constData(), function_ref);
    }
}


/*!
  \internal
*/
bool QSProject::loadInternal( QDataStream *stream )
{
    qint32 byteCount;
    qint16 major, minor, patch, dot1, dot2;

    (*stream) >> byteCount;
    (*stream) >> major;
    (*stream) >> dot1;
    (*stream) >> minor;
    (*stream) >> dot2;
    (*stream) >> patch;

    major -= '0';
    minor -= '0';
    patch -= '0';

    if (dot1 != '.' || dot2 != '.') {
        qWarning("QSProject::loadInternal(), datastream is not compatible");
        return false;
    }

    if (major != 1 || (minor != 2 && minor != 1 && minor != 0)) {
// #if defined(QT_CHECK_RANGE)
	qWarning("QSProject::loadInternal(), datastream is not compatible, %d.%d.%d",
                 major, minor, patch);
// #endif
	return false;
    }

    int bundles = 0;
    if (major == 1 && minor >= 1)
	(*stream) >> bundles;

    d->bundleStorage = bundles != 0;

    bool loadBundle = (minor == 0 || d->bundleStorage);

    d->scripts.clear();
    d->signalHandlers.clear();

#ifndef QSA_NO_EDITOR
    for (int i=0; i<d->editors.size(); ++i)
        delete d->editors[i];
    d->editors.clear();
#endif // QSA_NO_EDITOR

    int scriptCount;

    (*stream) >> scriptCount;
    QString name, code;
    QObject *context;
    for( int i=0; i<scriptCount; i++ ) {
	(*stream) >> name;
	if (loadBundle) {
	    (*stream) >> code;
	} else {
	    QFile f(name);
	    if (f.open(QIODevice::ReadOnly)) {
		QTextStream str(&f);
		code = str.readAll();
	    } else {
		qWarning("QSProject::loadInternal(), failed to read file '%s'", name.toLatin1().constData());
	    }
	}
	context = object( name );
	createScriptInternal( name, code, context );
    }

    int shCount;
    (*stream) >> shCount;
    for( int j=0; j<shCount; j++ ) {
	QSSignalHandler handler;
    QString senderName, receiverName;
	(*stream) >> senderName >> receiverName;
	handler.sender = object( senderName );
	handler.receiver = !receiverName.isNull() ? object( receiverName ) : 0;

	(*stream) >> handler.signal;
	(*stream) >> handler.function;
	if( handler.sender ) {
	    d->signalHandlers << handler;
	} else
	    qWarning( "QSProject::loadInternal(), unknown sender: '%s'", qPrintable(senderName) );
    }

    emit projectChanged();
    evaluate();

    return true;
}


/*!
  \internal
*/
bool QSProject::saveInternal( QDataStream *stream )
{
    (*stream) << QString(QSA_VERSION_STRING);

    (*stream) << (int)d->bundleStorage;

    (*stream) << d->scripts.count();
    QListIterator<QSScript *> it( d->scripts );
    QSScript *script;
    while(it.hasNext() && (script = it.next())) {
	(*stream) << script->name();
	if (d->bundleStorage) {
	    (*stream) << script->code();
	} else {
	    QFile f(script->name());
	    if (f.open(QIODevice::WriteOnly)) {
		QTextStream str(&f);
		str << script->code();
	    } else {
		qWarning("QSProject::saveInternal(), failed to write file '%s'",
			 script->name().toLatin1().constData());
	    }
	}
    }

    (*stream) << d->signalHandlers.size();
    QList<QSSignalHandler>::ConstIterator handlers = d->signalHandlers.begin();
    while( handlers != d->signalHandlers.end() ) {
	    (*stream) << (*handlers).sender->objectName()
            << ( (*handlers).receiver ? (*handlers).receiver->objectName() : QString() )
		          << (*handlers).signal
		          << (*handlers).function;
	++handlers;
    }

    return true;
}


/*!
  \internal
*/
QSScript *QSProject::createScriptInternal( const QString &name,
					   const QString &code,
					   QObject *context )
{
    Q_ASSERT( !context || context->objectName() == name );
    if( script( name ) ) {
#if defined(QT_CHECK_RANGE)
        qWarning( "QSProject::createScriptInternal: a script with the name '%s' already exists", name.latin1() );
#endif
        return 0;
    }
    QSScript *s = new QSScript( this, name, code, context );
    d->scripts.append( s );
    if( context && d->objects.indexOf( context ) ) {
        d->objects.append( context );
        connect( context, SIGNAL( destroyed() ),
            this, SLOT( objectDestroyed() ) );
    }
    connect( s, SIGNAL( codeChanged() ), this, SIGNAL( projectChanged() ) );
    connect( s, SIGNAL( codeChanged() ), this, SLOT( scriptChanged() ) );
    connect( s, SIGNAL( destroyed() ), this, SLOT( objectDestroyed() ) );
    emit projectChanged();
    return s;
}


/*! This function is called when a new QSEditor has been
  created.
  \internal
*/
#ifndef QSA_NO_EDITOR
void QSProject::registerEditor( QSEditor *editor )
{
    if(d->editors.indexOf(editor) >= 0)
	    return ;
    d->editors.append( editor );
    connect( editor, SIGNAL( destroyed() ),
	     this, SLOT( objectDestroyed() ) );
    connect( editor, SIGNAL( textChanged() ),
	     this, SIGNAL( editorTextChanged() ) );
}
#endif


/*! This function is called when a QSEditor has been
  detroyed.
  \internal
*/
#ifndef QSA_NO_EDITOR
void QSProject::unregisterEditor( QSEditor *editor )
{
    int idx;
    if ((idx = d->editors.indexOf(editor)) < 0)
	    return ;
    d->editors.removeAt(idx);
    disconnect( editor, SIGNAL( destroyed() ),
		this, SLOT( objectDestroyed() ) );
    disconnect( editor, SIGNAL( textChanged() ),
		this, SIGNAL( editorTextChanged() ) );
}
#endif


/*!
  \internal
*/
void QSProject::objectDestroyed()
{
    const QObject *o = QObject::sender();

#ifndef QSA_NO_EDITOR
    if (d->editors.removeAll((QSEditor *)o))
	    return ;
#endif
    if ( d->scripts.removeAll( (QSScript*) o ) ) {
	    emit projectChanged();
	    return;
    }

    removeObject( o );
}


/*
  \internal
*/
void QSProject::scriptChanged()
{
    d->scriptsModified = true;
}


#if 0
/*!
  Returns the name of the project.
*/
QString QSProject::projectName() const
{
    return d->projectName;
}
#endif
