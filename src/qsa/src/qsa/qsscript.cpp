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

#include "qsproject.h"
#include "qsscript.h"

/*!
  \class QSScript qsscript.h

  \brief The QSScript class provides encapsulation of a script
  written in Qt Script.

  A script is a container for a script written in Qt Script. It always
  belongs to a project and it can only be created using the
  QSProject::createScript() functions. The QSScript can be associated
  with a QObject which will serve as the context ( \c this ) when the
  script is evaluated. QSScripts and their contexts, if any, are
  associated by name as follows:

  \code
  script->name() == script->context()->name()
  \endcode

  Use the functions setCode(), addCode(), and addFunction() to modify
  the contents of the script. Use the function code() to retreive the
  contents in the script. When the script is changed, the signal
  codeChanged() is emitted.
 */

class QSScriptPrivate
{
public:
    QSScriptPrivate() :
	context( 0 ),
	project( 0 )
    {
    }

    QString name;
    QString code;
    QObject *context;
    QSProject *project;
};


/*!
  \fn void QSScript::codeChanged()

  This signal is emitted whenever the scriptcode has changed.
*/


/*!
  \internal

  Creates a new script in \a project with the name \a name,
  context \a context and code \a code.
*/
QSScript::QSScript( QSProject *project,
		    const QString &name,
		    const QString &code,
		    QObject *context )
    :
    QObject( project, name.local8Bit() )
{
    d = new QSScriptPrivate;
    d->code = code;
    d->name = name;
    d->project = project;

    if( context ) {
	d->context = context;
	connect( context, SIGNAL( destroyed() ),
		 this, SLOT( objectDestroyed() ) );
    }
}

/*!
  \internal
*/
QSScript::~QSScript()
{
    delete d;
}


/*!
  Returns the name of the script. If the script has a context,
  this name is the same as the name of the object \c context()->name().
*/
QString QSScript::name() const
{
    return d->name;
}


/*!
  Returns the scriptcode.
*/
QString QSScript::code() const
{
    return d->code;
}


/*!
  Returns the context of the script if the script has context;
  otherwise returns 0.

  The name of the context \c context()->name() is to be the same
  name as this script.
*/
QObject *QSScript::context() const
{
    return d->context;
}

/*!
  \internal
*/
void QSScript::setContext( QObject *context )
{
    Q_ASSERT( !d->context );
    connect( context, SIGNAL( destroyed() ),
	     this, SLOT( objectDestroyed() ) );
    d->context = context;
}


/*!
  Returns the project that this script belongs to.
*/
QSProject *QSScript::project() const
{
    return d->project;
}


/*!
  Sets the code in this script to \a code.
  This function will emit codeChanged().
*/
void QSScript::setCode( const QString &code )
{
    d->code = code;
    emit codeChanged();
}


/*!
  Appends the code \a code to the code in this script.
  This function will emit codeChanged()
*/
void QSScript::addCode( const QString &code )
{
    d->code += code;
    emit codeChanged();
}

/*!
  A function called \a functionName is added to this script.
  The name must match the regular expression
  \c{/^[A-Za-z_][A-Za-z_0-9]*$/}.

  The code for the function can be passed in the \a functionBody
  parameter. If \a functionBody was specified, its text will appear in
  the editor; if the \a functionBody is empty a new empty function
  will be created.

  This function returns TRUE on success (i.e. the \a functionName was
  valid); otherwise returns FALSE.
*/
bool QSScript::addFunction( const QString &functionName,
			    const QString &functionBody )
{
    static QString letters = QString::fromLatin1("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_");
    static QString numbers = QString::fromLatin1("0123456789");
    for ( int i = 0; i < (int)functionName.length(); ++i ) {
	if ( i == 0 && letters.find( functionName[i] ) == -1 ||
	     letters.find( functionName[i] ) == -1 && numbers.find( functionName[i] ) == -1 ) {
#if defined(QT_CHECK_RANGE)
	    qWarning( "QSProject::addFunction: '%s' is not a valid function name",
		      functionName.latin1() );
#endif
	    return FALSE;
	}
    }

    QString str = QString::fromLatin1("function %1()\n{\n%2\n}")
                  .arg(functionName)
                  .arg(functionBody);
    d->code += str;
    emit codeChanged();
    return TRUE;
}


/*!
  \internal
*/
void QSScript::objectDestroyed()
{
    const QObject *object = QObject::sender();
    if( object == d->context ) {
	delete this;
    }
}
