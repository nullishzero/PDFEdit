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

#include "configutils.h"

#include <qfile.h>
#include <qprocess.h>
#include <qdir.h>
#include <qstringlist.h>
#include <qregexp.h>
#include <qapplication.h>
#include <stdlib.h>
#ifdef Q_WS_WIN
#include <windows.h>
#endif

static const char *app = 0;

static void dumpMessages()
{
    qWarning( messages().join( "\n" ) );
}

static void dumpMessagesStdout()
{
    printf("%s\n", messages().join( "\n" ).ascii() );
}


int main( int argc, char **argv )
{
    app = argv[0];
    QApplication qapp( argc, argv, FALSE );

    QStringList antiConfigs;
    QStringList configs;
    QString prefix;
    bool buildIde = TRUE;
    for ( int i = 1; i < qapp.argc(); i++ ) {
	QString arg = qapp.argv()[i];
	if ( arg.startsWith( "--" ) )
	    arg = arg.mid( 1 );
	if ( arg == "-help"  ) {
	    ; // handled by the configure script
	} else if ( arg == "-thread" ) {
	    configs << "thread";
	} else if ( arg == "-prefix" ) {
	    if ( i + 1 < qapp.argc() ) {
		prefix = qapp.argv()[++i];
	    } else {
		qWarning( "-prefix option requires path argument" );
		exit( 2 );
	    }
	} else if ( arg == "-no-ide" ) {
	    buildIde = FALSE;
	    configs << "noide";

        } else if (arg == "-release") {
            configs << "release";
            antiConfigs << "debug";

        } else if (arg == "-qmake") {
	    if ( i + 1 < qapp.argc() ) {
                setQMake(qapp.argv()[++i]);
	    } else {
		qWarning( "-qmake option requires argument" );
		exit( 2 );
	    }

        } else if (arg == "-debug") {
            configs << "debug";
            antiConfigs << "release";

	} else {
	    qWarning( "Unknown option: %s", qapp.argv()[i] );
	    exit( 1 );
	}
    }

    // these are here in case .qmake.cache is missing
#if defined(QT_MODULE_XML)
    configs.append( "xml" );
#endif
#if defined(QT_MODULE_TABLE)
    configs.append( "table" );
#endif
#if defined(QT_MODULE_SQL)
    configs.append( "sql" );
#endif
#if defined(QT_MODULE_NETWORK)
    configs.append( "network" );
#endif

    qtDir = new QString( getenv( "QTDIR" ) );
    if ( qtDir->isEmpty() ) {
	qWarning( "%s: QTDIR not set", app );
	dumpMessages();
	return 1;
    }
    *qtDir += "/";

    if( !writeQSConfig( buildIde ) ) {
	dumpMessages();
	return 1;
    }

    runQMake( configs, antiConfigs, prefix );

    int retVal = processes > 0 ? qapp.exec() : 0;
    if( retVal || errors ) {
	message( "\nThere were errors during configure!" );
        dumpMessages();
    } else if( warnings ) {
	message( "\nThere were errors during configure, but these"
		 "\ndo not appear to be fatal, so you should still be"
		 "\nable to build QSA."
		 "\nRun your make tool to build QSA." );
        dumpMessages();
    } else {
	message( "\n"
		"Configuration completed successfully\n"
		"Run your make tool to build QSA" );
        dumpMessagesStdout();
    }

    return retVal || errors;
}
