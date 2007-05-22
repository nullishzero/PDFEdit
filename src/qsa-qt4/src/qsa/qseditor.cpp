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
#include "qsscript.h"
#include <QEvent>
#include <QResizeEvent>

#if defined(QSA_NEW_EDITOR)
#include "../neweditor/qsaeditorinterface.h"
#include "../neweditor/qsaeditor.h"
#else
#include "../editor/qsacompletion.h"
#include "../editor/qsaeditor.h"
#include "../editor/qsaeditorinterface.h"
#include <q3textedit.h>
#endif

#include <qapplication.h>
#include <qobject.h>
#include <qmetaobject.h>
#include <qsizepolicy.h>

class QSEditorPrivate
{
public:
    QSEditorPrivate() :
	source( 0 ),
	interpreter( 0 ),
	editor( 0 ),
	editorWidget( 0 )
    {
    }

    ~QSEditorPrivate()
    {
	delete editor;
	delete eIface;
    }

    QSScript *source;
    QSInterpreter *interpreter;
    QSAEditor *editor;
    QSAEditorInterface *eIface;
    QWidget *editorWidget;

    uint isUpdater : 1;
    uint modified : 1;
};

/*!
  \class QSEditor qseditor.h

  \brief The QSEditor class provides an enhanced editor for editing
  Qt Script source code.

  For some applications \i {QSA Workbench} is too complex and it might
  be more appropriate to embed a simple editor into the application's
  user interface.  QSEditor provides a type of widget component which offers
  normal editor functionality as well as Qt Script syntax
  highlighting, completion, function argument hints, and
  auto-indentation.

  The text in the QSEditor can be set using setText() and retrieved
  using text().

  A QSEditor can be associated with a QSScript object using
  setScript() and the QSScript can then be retrieved by using
  script().

  When text is edited in the QSEditor, it is not directly reflected in
  the underlying script. To write the changes from the editor to the
  script, use commit(). To revert the changes from the editor to the
  text in the script, use revert().

  The QSEditor will automatically become the active editor when it
  gets the focus. The active editor can be retrieved from QSProject.

  Look at the examples/scriptbutton and examples/console examples to
  see an example of the QSEditor
  class.
*/

/*! \fn void QSEditor::textChanged()
  This signal is emitted if the text of the editor has been changed
*/

/*!
  Constructs an editor not linked to any project.

  The \a parent argument is passed to the QWidget constructor. The 
  \a name argument is used as the widget's object name.
*/

QSEditor::QSEditor( QWidget *parent, const char *name )
    : QWidget(parent)
{
    setObjectName(name);
    d = new QSEditorPrivate;
    init();
}

static void qsa_widget_descendants(QObjectList *results, const QObjectList &l) 
{
    Q_ASSERT(results != 0);

    for (int i=0; i<l.count(); ++i) {
        QObject *o = l[i];
        if (o->isWidgetType())
            results->append(o);

        qsa_widget_descendants(results, o->children());
    }
}

/*! \internal
 */


void QSEditor::init()
{
    active = false;
    d->eIface = new QSAEditorInterface();
    QWidget *editor = d->eIface->editor( false, this );
    d->editor = qobject_cast<QSAEditor *>(editor);
    Q_ASSERT(d->editor);
    d->editorWidget = editor->parentWidget();

    setFocusPolicy( Qt::WheelFocus );
    setFocusProxy( editor );
    setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    connect( d->editor, SIGNAL( textChanged() ),
	     this, SIGNAL( textChanged() ) );
    connect( this, SIGNAL( textChanged() ),
	     this, SLOT( modifyEditor() ) );
    editor->setGeometry( 0, 0, width(), height() );
    editor->show();

    d->isUpdater = false;
    d->modified = false;

    QObjectList l;
    qsa_widget_descendants(&l, children());
    for (int i=0; i<l.size(); ++i) {
        QWidget *o = static_cast<QWidget *>(l.at(i));
	o->installEventFilter( this );
    }
}


/*!
  Destructs the editor.
*/
QSEditor::~QSEditor()
{
    delete d;
}


/*!
  Returns the project the editor is associated with; otherwise returns 0.
*/
QSProject *QSEditor::project() const
{
    return d->interpreter ? d->interpreter->project() : 0;
}


/*!
  Sets the editor contents to \a text.
*/
void QSEditor::setText( const QString &text )
{
#ifndef QSA_NEW_EDITOR
    d->editor->setText( text );
#else
    d->editor->setPlainText(text);
#endif
}


/*!
  Returns the text of the editor.
*/
QString QSEditor::text() const
{
#ifndef QSA_NEW_EDITOR
    return d->editor->text();
#else
    return d->editor->toPlainText();
#endif
}


/*!
  Updates the script with the text in the editor.

  When the text in the editor is changed, the changes must be
  committed before they are reflected in the script.
*/
void QSEditor::commit()
{
    if( d->source ) {
	d->isUpdater = true; // Avoid recursion...
	d->source->setCode( text() );
	d->isUpdater = false;
	d->modified = false;
    }
}


/*!
  Reverts the contents in the editor back to the contents
  of the underlying script.

  */
void QSEditor::revert()
{
    setText( d->source->code() );
    d->modified = false;
}


/*!
  Returns true if the text in the editor is different from the
  text in the underlying script. This is the case whenever the
  user edited the script and the changes have not been committed
  or reverted.
*/
bool QSEditor::isModified() const
{
    return d->modified;
}


/*!
  Activates the editor. Returns true if it succeeds;
  otherwise returns false.

  See the class description for information on active editors.
*/
bool QSEditor::activate()
{
    QSProject *proj = project();
    if( proj )
	proj->evaluate();
    QSEditor *ed = ( proj ? proj->activeEditor() : 0 );
    if ( ed )
	ed->release();
    active = true;
    return true;
}


/*!
  Releases (deactivates) the editor.

  See the class description for information on active editors.
*/
void QSEditor::release()
{
    active = false;
}


/*!
  \fn bool QSEditor::isActive() const
  Returns true if the editor is active; otherwise returns false.

  See the class description for information on active editors.
*/


/*!
  Returns true if undo is available; otherwise returns false.
*/
bool QSEditor::isUndoAvailable() const
{
#ifndef QSA_NEW_EDITOR
    return d->editor->isUndoAvailable();
#else
    return d->editor->document()->isUndoAvailable();
#endif
}


/*!
  Returns true if redo is available; otherwise returns false.
*/
bool QSEditor::isRedoAvailable() const
{
#ifndef QSA_NEW_EDITOR
    return d->editor->isRedoAvailable();
#else
    return d->editor->document()->isRedoAvailable();
#endif
}

/*!
  Undo the last editor operation.
*/
void QSEditor::undo()
{
#ifndef QSA_NEW_EDITOR
    d->editor->undo();
#else
    d->editor->document()->undo();
#endif
}


/*!
  Redo the last editor operation.
*/
void QSEditor::redo()
{
#ifndef QSA_NEW_EDITOR
    d->editor->redo();
#else
    d->editor->document()->redo();
#endif
}


/*!
  Cuts the selected text to the clipboard.
*/
void QSEditor::cut()
{
    d->editor->cut();
}


/*!
  Copies the selected text to the clipboard.
*/
void QSEditor::copy()
{
    d->editor->copy();
}


/*!
  Pastes text from the clipboard into the editor.
*/
void QSEditor::paste()
{
    d->editor->paste();
}


/*!
  Selects all the text in the editor.
*/
void QSEditor::selectAll()
{
    d->editor->selectAll();
}


/*!
  Finds the string \a expr in the editor and selects the
  result. If \a cs is true, the search is case sensitive. If \a wo
  is true, only whole words are searched. If \a forward is true, the
  search is executed forward, otherwise backwards. If \a startAtCursor is
  true, the search starts at the current cursor position, otherwise it
  starts at the beginning of the document.

  Returns true if \a expr was found, otherwise returns false.
*/
bool QSEditor::find( const QString &expr, bool cs, bool wo,
		     bool forward, bool startAtCursor )
{
    return d->eIface->find( expr, cs, wo, forward, startAtCursor );
}


/*!
  Replaces the string \a find in the editor with \a replace. If
  \a cs is true, the search is case sensitive. If \a wo is true,
  only whole words are searched. If \a forward is true, the search is
  executed forward, otherwise backwards. If \a startAtCursor is true, the
  search starts at the current cursor position, otherwise it starts at
  the beginning of the document. If \a replaceAll is true, all found
  occurrences of \a find are replaced by \a replace. Otherwise only the
  first occurrence is replaced.
*/
bool QSEditor::replace( const QString &find, const QString &replace, bool cs, bool wo,
			bool forward, bool startAtCursor, bool replaceAll )
{
    return d->eIface->replace( find, replace, cs, wo, forward, startAtCursor, replaceAll );
}


/*!
  Moves the cursor to the line \a line in the editor.
*/
void QSEditor::gotoLine( int line )
{
    d->eIface->gotoLine( line - 1 );
}


/*!
  Indents the current line or selection.
*/
void QSEditor::indent()
{
    d->eIface->indent();
}


/*!
  Re-reads settings for syntax highlighting, indentation, etc. The
  settings are read from the QSettings path
  \c /Trolltech/QSAScriptEditor/.

  The following settings are read for syntax highlighting:

  \list
  \i \c /Comment
  \i \c /Number
  \i \c /String
  \i \c /Type
  \i \c /Label
  \i \c /Standard
  \endlist

  For each of those, the following settings are read:

  \list
  \i \c /family (string)
  \i \c /size (number)
  \i \c /bold (bool)
  \i \c /italic (bool)
  \i \c /underline (bool)
  \i \c /red (number)
  \i \c /green (number)
  \i \c /blue (number)
  \endlist

  The font family for strings is e.g. saved in
  \c /Trolltech/QuickScriptEditor/String/family.

  The following settings for indentation are read:

  \list
  \i \c /indentAutoIndent (bool) - whether auto indentation should be done
  \i \c /indentTabSize (number) - the tab size for indentation
  \i \c /indentIndentSize (number) - the indentation width
  \i \c /indentKeepTabs (bool) - whether to keep tabs or to replace them with spaces
  \endlist

  Code completion can be switched on or off via \c /completion. Wordwrapping can be switched on or off via \c /wordWrap.
  Parentheses matching can be switched on or off via \c /parenMatching.
*/
void QSEditor::readSettings()
{
    d->eIface->readSettings();
}


/*!
  Highlights the line \a line. This can be used e.g., to show an
  error or warning.
*/
void QSEditor::setErrorMark( int line )
{
    d->eIface->setError( line - 1 );
}


/*!
  Removes all the error marks for the editor.
 */
void QSEditor::removeErrorMark()
{
    d->eIface->clearError();
}

/*!
  \internal
*/
QSize QSEditor::sizeHint() const
{
    return d->editorWidget ? d->editorWidget->sizeHint() : QSize();
}


/*!
  \internal
*/
QSize QSEditor::minimumSizeHint() const
{
    return d->editorWidget ? d->editorWidget->minimumSizeHint() : QSize();
}


/*!
  \internal
*/
void QSEditor::resizeEvent( QResizeEvent *e )
{
    if( d->editorWidget ) {
	QWidget::resizeEvent( e );
	d->editorWidget->setGeometry( 0, 0, width(), height() );
    }
}


/*!
  \internal
*/
bool QSEditor::eventFilter( QObject *o, QEvent *e )
{
    if ( e->type() == QEvent::FocusIn )
	activate();
    return QWidget::eventFilter( o, e );
}


/*!
  Returns the pointer to the real editor widget which is a
  Q3TextEdit. This might be 0 if an error occurred while initializing
  the editor.
*/
#if defined(QSA_NEW_EDITOR)
QTextEdit *QSEditor::textEdit() const
#else
Q3TextEdit *QSEditor::textEdit() const
#endif
{
    return d->editor;
}


/*!
  Returns the script that the editor is editing if any; otherwise
  returns 0.
 */
QSScript *QSEditor::script() const
{
    Q_ASSERT( d->source );
    return d->source;
}


/*!
  Sets the current script of the editor to be \a script.
 */
bool QSEditor::setScript( QSScript *script )
{
    if( !script ) {
	return false;
    }

    if( script->project()->editor( script ) &&
	script->project()->editor( script ) != this ) {
#if defined( QT_RANGE_CHECK )
	qWarning( "QSEditor::setScript(), script already has an editor" );
#endif
	return false;
    }


    if( d->source && d->source->project() != script->project() ) {
#if defined( QT_RANGE_CHECK )
	qWarning( "QSEditor::setScript(), new script cannot belong to different projects" );
#endif
	return false;
    }

    if( d->interpreter && d->interpreter->project() != script->project() ) {
#if defined( QT_RANGE_CHECK )
	qWarning( "QSEditor::setScript(), Interpreter differs from that in source" );
#endif
	return false;
    }

    if( d->source )
	disconnect( d->source, SIGNAL( codeChanged() ),
		    this, SLOT( scriptChanged() ) );

    script->project()->registerEditor( this );

    d->source = script;
    d->interpreter = script->project()->interpreter();
#if defined(QSA_NEW_EDITOR)
    d->editor->setContextObject(script->context());
#else
    ( (QSACompletion*) d->editor->completionManager() )->setContext( script->context() );
#endif
    setText( script->code() );
    d->editor->setInterpreter( script->project()->interpreter() );

    connect( script, SIGNAL( codeChanged() ),
	     this, SLOT( scriptChanged() ) );
    connect( script, SIGNAL( destroyed() ),
	     this, SLOT( scriptDestroyed() ) );

    d->modified = false;

    return true;
}


void QSEditor::scriptDestroyed()
{
    delete this;
}

void QSEditor::scriptChanged()
{
    Q_ASSERT( d->source );
    if( !d->isUpdater )
	setText( d->source->code() );
}

void QSEditor::modifyEditor()
{
    d->modified = true;
}


/*!
  Sets the interpreter that this editor will do completion for to \a interpreter
  with the context ( \c this object ) as \a context

  This function is only for use when the editor is used outside a project. If
  the editor has been associated with a project with the functions
  QSProject::createEditor() or QSEditor::setScript() this function
  will not have any effect.

  \sa QSProject::createEditor(), setScript()
 */
bool QSEditor::setInterpreter( QSInterpreter *interpreter, QObject *context )
{
    if( !interpreter )
	return false;
    if( d->source ) {
	if( d->source->project()->interpreter() != interpreter ) {
#if defined( QT_RANGE_CHECK )
	    qWarning( "QSEditor::setInterpreter(), Editor belongs to another project" );
#endif
	    return false;
	}
    } else {
	d->interpreter = interpreter;
        d->editor->setInterpreter( interpreter );
    }
#if defined(QSA_NEW_EDITOR)
    d->editor->setContextObject(context);
#else
    ( (QSACompletion*) d->editor->completionManager() )->setContext( context );
#endif
    return true;
}


/*!
  Returns the interpreter that this editor uses for completion if it
  has been set; otherwise returns 0.

  The interpreter can be set using functions
  QSProject::createEditor(), setScript() or setInterpreter().
*/
QSInterpreter *QSEditor::interpreter() const
{
    return d->interpreter;
}
