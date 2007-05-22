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
    QObject( project )
{
    setObjectName(name);

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
  this name is the same as the object name of the context (\c context()->objectName().)
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

  This function returns true on success (i.e. the \a functionName was
  valid); otherwise returns false.
*/
bool QSScript::addFunction( const QString &functionName,
			    const QString &functionBody )
{
    static QString letters = QString::fromLatin1("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_");
    static QString numbers = QString::fromLatin1("0123456789");
    for ( int i = 0; i < (int)functionName.length(); ++i ) {
	if ( i == 0 && letters.indexOf( functionName[i] ) == -1 ||
	     letters.indexOf( functionName[i] ) == -1 && numbers.indexOf( functionName[i] ) == -1 ) {
#if defined(QT_CHECK_RANGE)
	    qWarning( "QSProject::addFunction: '%s' is not a valid function name",
		      functionName.toLatin1().constData() );
#endif
	    return false;
	}
    }

    QString str = QString::fromLatin1("function %1()\n{\n%2\n}")
                  .arg(functionName)
                  .arg(functionBody);
    d->code += str;
    emit codeChanged();
    return true;
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
