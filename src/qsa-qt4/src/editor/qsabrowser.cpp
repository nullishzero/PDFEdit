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

#include "qsabrowser.h"
#include "qsaeditor.h"

#include "quickinterpreter.h"
// #include "quickdebugger.h"

#include "qsproject.h"
//Added by qt3to4:
#include <QKeyEvent>
#include <QEvent>

#include <stdio.h>

QSAEditorBrowser::QSAEditorBrowser( Editor *e )
    : EditorBrowser( e )
{
}

void QSAEditorBrowser::showHelp( const QString &str )
{
    qFatal( "QSAEditorBrowser::showHelp( %s )", str.latin1() );
#if 0
    if ( ( (QSAEditor*)curEditor)->isDebugging() ) {
	QString s = str.simplifyWhiteSpace();
	static QString legalChars = "abcdefghijklmnopqrstuvwxyzABSCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_";
	while ( s.length() > 0 ) {
	    if ( legalChars.find( s[0] ) == -1 )
		s.remove( 0, 1 );
	    else
		break;
	}
	while ( s.length() > 0 ) {
	    if ( legalChars.find( s[ (int)s.length() - 1 ] ) == -1 )
		s.remove( s.length() - 1, 1 );
	    else
		break;
	}

	if ( s[ (int)s.length() - 1 ] == ';' )
	    s.remove( s.length() - 1, 1 );

	QString type, val;
	QSProject::ideQSAInterpreter()->debuggerEngine()->watch( s, type, val );
	if ( !val.isEmpty() && !type.isEmpty() ) {
	    QWidget *w = curEditor->topLevelWidget();
	    QObject *o = w->child( "QSA_debugger_variableview", "QSAVariableView" );
	    if ( o )
		( (QSAVariableView*)o )->addWatch( s );
	}
    } else {
    }
#endif
}

bool QSAEditorBrowser::eventFilter( QObject *o, QEvent *e )
{
#if 0
    if ( !( (QSAEditor*)curEditor )->isDebugging() )
	return EditorBrowser::eventFilter( o, e );

    if ( e->type() == QEvent::KeyPress ) {
	QKeyEvent *ke = (QKeyEvent*)e;
	switch ( ke->key() ) {
	case Qt::Key_Up:
	case Qt::Key_Down:
	case Qt::Key_Left:
	case Qt::Key_Right:
	case Qt::Key_PageUp:
	case Qt::Key_PageDown:
	case Qt::Key_Home:
	case Qt::Key_End:
	case Qt::Key_F9:
	case Qt::Key_F10:
	case Qt::Key_F11:
	case Qt::Key_F5:
	    break;
	default:
	    return true;
	}
    }
#endif
    printf( "QSAEditorBrowser::eventFilter(), skipped" );
    return EditorBrowser::eventFilter( o, e );
}
