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

#include "qseditor.h"
#include "qsinterpreter.h"
#include "qsproject.h"
#include "qsscript.h"

#include "../editor/qsacompletion.h"
#include "../editor/qsaeditor.h"
#include "../editor/qsaeditorinterface.h"

#include <qapplication.h>
#include <qobjectlist.h>
#include <qmetaobject.h>
#include <qsizepolicy.h>
#include <qtextedit.h>

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

  For some applications \e {QSA Workbench} is too complex and it might
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

  The \a parent and \a name arguments are passed to the QWidget
  constructor.
*/

QSEditor::QSEditor( QWidget *parent, const char *name )
    : QWidget( parent, name )
{
    d = new QSEditorPrivate;
    init();
}

/*! \internal
 */

void QSEditor::init()
{
    active = FALSE;
    d->eIface = new QSAEditorInterface();
    QWidget *editor = d->eIface->editor( FALSE, this );
    Q_ASSERT( editor->inherits( "QSAEditor" ) );
    d->editor = (QSAEditor*) editor;
    d->editorWidget = editor->parentWidget();

    setFocusPolicy( WheelFocus );
    setFocusProxy( editor );
    setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    connect( d->editor, SIGNAL( textChanged() ),
	     this, SIGNAL( textChanged() ) );
    connect( this, SIGNAL( textChanged() ),
	     this, SLOT( modifyEditor() ) );
    editor->setGeometry( 0, 0, width(), height() );
    editor->show();

    d->isUpdater = FALSE;
    d->modified = FALSE;

    QObjectList *l = queryList( "QWidget" );
    for ( QObject *o = l->first(); o; o = l->next() )
	o->installEventFilter( this );
    delete l;
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
    d->editor->setText( text );
}


/*!
  Returns the text of the editor.
*/
QString QSEditor::text() const
{
    return d->editor->text();
}


/*!
  Updates the script with the text in the editor.

  When the text in the editor is changed, the changes must be
  committed before they are reflected in the script.
*/
void QSEditor::commit()
{
    if( d->source ) {
	d->isUpdater = TRUE; // Avoid recursion...
	d->source->setCode( text() );
	d->isUpdater = FALSE;
	d->modified = FALSE;
    }
}


/*!
  Reverts the contents in the editor back to the contents
  of the underlying script.

  */
void QSEditor::revert()
{
    setText( d->source->code() );
    d->modified = FALSE;
}


/*!
  Returns TRUE if the text in the editor is different from the
  text in the underlying script. This is the case whenever the
  user edited the script and the changes have not been committed
  or reverted.
*/
bool QSEditor::isModified() const
{
    return d->modified;
}


/*!
  Activates the editor. Returns TRUE if it succeeds;
  otherwise returns FALSE.

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
    active = TRUE;
    return TRUE;
}


/*!
  Releases (deactivates) the editor.

  See the class description for information on active editors.
*/
void QSEditor::release()
{
    active = FALSE;
}


/*!
  \fn bool QSEditor::isActive() const
  Returns TRUE if the editor is active; otherwise returns FALSE.

  See the class description for information on active editors.
*/


/*!
  Returns TRUE if undo is available; otherwise returns FALSE.
*/
bool QSEditor::isUndoAvailable() const
{
    return d->editor->isUndoAvailable();
}


/*!
  Returns TRUE if redo is available; otherwise returns FALSE.
*/
bool QSEditor::isRedoAvailable() const
{
    return d->editor->isRedoAvailable();
}

/*!
  Undo the last editor operation.
*/
void QSEditor::undo()
{
    d->editor->undo();
}


/*!
  Redo the last editor operation.
*/
void QSEditor::redo()
{
    d->editor->redo();
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
  result. If \a cs is TRUE, the search is case sensitive. If \a wo
  is TRUE, only whole words are searched. If \a forward is TRUE, the
  search is executed forward, otherwise backwards. If \a startAtCursor is
  TRUE, the search starts at the current cursor position, otherwise it
  starts at the beginning of the document.

  Returns TRUE if \a expr was found, otherwise returns FALSE.
*/
bool QSEditor::find( const QString &expr, bool cs, bool wo,
		     bool forward, bool startAtCursor )
{
    return d->eIface->find( expr, cs, wo, forward, startAtCursor );
}


/*!
  Replaces the string \a find in the editor with \a replace. If
  \a cs is TRUE, the search is case sensitive. If \a wo is TRUE,
  only whole words are searched. If \a forward is TRUE, the search is
  executed forward, otherwise backwards. If \a startAtCursor is TRUE, the
  search starts at the current cursor position, otherwise it starts at
  the beginning of the document. If \a replaceAll is TRUE, all found
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
  \reimp
*/
QSize QSEditor::sizeHint() const
{
    return d->editorWidget ? d->editorWidget->sizeHint() : QSize();
}


/*!
  \reimp
*/
QSize QSEditor::minimumSizeHint() const
{
    return d->editorWidget ? d->editorWidget->minimumSizeHint() : QSize();
}


/*!
  \reimp
*/
void QSEditor::resizeEvent( QResizeEvent *e )
{
    if( d->editorWidget ) {
	QWidget::resizeEvent( e );
	d->editorWidget->setGeometry( 0, 0, width(), height() );
    }
}


/*!
  \reimp
*/
bool QSEditor::eventFilter( QObject *o, QEvent *e )
{
    if ( e->type() == QEvent::FocusIn )
	activate();
    return QWidget::eventFilter( o, e );
}


/*!
  Returns the pointer to the real editor widget which is a
  QTextEdit. This might be 0 if an error occurred while initializing
  the editor.
*/
QTextEdit *QSEditor::textEdit() const
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
	return FALSE;
    }

    if( script->project()->editor( script ) &&
	script->project()->editor( script ) != this ) {
#if defined( QT_RANGE_CHECK )
	qWarning( "QSEditor::setScript(), script already has an editor" );
#endif
	return FALSE;
    }


    if( d->source && d->source->project() != script->project() ) {
#if defined( QT_RANGE_CHECK )
	qWarning( "QSEditor::setScript(), new script cannot belong to different projects" );
#endif
	return FALSE;
    }

    if( d->interpreter && d->interpreter->project() != script->project() ) {
#if defined( QT_RANGE_CHECK )
	qWarning( "QSEditor::setScript(), Interpreter differs from that in source" );
#endif
	return FALSE;
    }

    if( d->source )
	disconnect( d->source, SIGNAL( codeChanged() ),
		    this, SLOT( scriptChanged() ) );

    script->project()->registerEditor( this );

    d->source = script;
    d->interpreter = script->project()->interpreter();
    ( (QSACompletion*) d->editor->completionManager() )->setContext( script->context() );
    setText( script->code() );
    d->editor->setInterpreter( script->project()->interpreter() );

    connect( script, SIGNAL( codeChanged() ),
	     this, SLOT( scriptChanged() ) );
    connect( script, SIGNAL( destroyed() ),
	     this, SLOT( scriptDestroyed() ) );

    d->modified = FALSE;

    return TRUE;
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
    d->modified = TRUE;
}


/*!
  Sets the interpreter that this editor will do completion for to \a interpreter
  with the contetext ( \c this object ) as \a context

  This function is only for use when the editor is used outside a project. If
  the editor has been associated with a project with the functions
  QSProject::createEditor() or QSEditor::setScript() this function
  will not have any effect.

  \sa QSProject::createEditor(), setScript()
 */
bool QSEditor::setInterpreter( QSInterpreter *interpreter, QObject *context )
{
    if( !interpreter )
	return FALSE;
    if( d->source ) {
	if( d->source->project()->interpreter() != interpreter ) {
#if defined( QT_RANGE_CHECK )
	    qWarning( "QSEditor::setInterpreter(), Editor belongs to another project" );
#endif
	    return FALSE;
	}
    } else {
	d->interpreter = interpreter;
	d->editor->setInterpreter( interpreter );
    }
    ( (QSACompletion*) d->editor->completionManager() )->setContext( context );
    return TRUE;
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
