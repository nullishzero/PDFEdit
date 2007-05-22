/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qglobal.h>

#if !defined(Q_CC_GNU)
#pragma warning(disable : 4996)
#endif

#include "configutils.h"

#include <qfile.h>
#include <qdir.h>
#include <qstringlist.h>
#include <qregexp.h>
#include <qcoreapplication.h>
#include <stdlib.h>
#ifdef Q_WS_WIN
#include <windows.h>
#endif

static void dumpMessages()
{
    qWarning( "%s", qPrintable(messages().join( QLatin1String("\n") )) );
}

int main( int argc, char **argv )
{
#ifndef QT_VERSION
#  define QT_VERSION 0x01
#endif
#if QT_VERSION < 0x040001
    printf("\nThis version of QSA requires Qt 4.0.1 or later, using: %x.%x.%x\n\n",
           (QT_VERSION>>16) & 0xff,
           (QT_VERSION>>8) & 0xff,
           QT_VERSION & 0xff);
    return 100;
#endif

    QCoreApplication qapp( argc, argv );


    QStringList antiConfigs;
    QStringList configs;
    bool buildIde = true;
    bool buildEditor = true;
    bool buildNewEditor = false;

#if QT_VERSION >= 0x040100
    int arg_count = qapp.arguments().size();
    QStringList args = qapp.arguments();
#else
    int arg_count = qapp.argc();
    QStringList args;
    for (int i=0; i<arg_count; ++i)
        args << qapp.argv()[i];
#endif

    for ( int i = 1; i < arg_count; ++i ) {
        QString arg = args.at(i);
        if ( arg.startsWith( QLatin1String("--") ) )
            arg = arg.mid( 1 );
        if ( arg == QLatin1String("-help") ) {
            ; // handled by the configure script
        } else if ( arg == QLatin1String("-prefix") ) {
             if (i+1 < arg_count) {
	        qsa_prefix = new QString(args.at(++i));
             } else {
                qWarning( "-prefix option requires path argument" );
                return 2;
             }
        } else if ( arg == QLatin1String("-no-ide") ) {
            buildIde = false;
            configs << QLatin1String("noide");
        } else if (arg == QLatin1String("-debug")) {
            configs << QLatin1String("debug");
            antiConfigs << QLatin1String("release") << QLatin1String("debug_and_release");
        } else if (arg == QLatin1String("-release")) {
            configs << QLatin1String("release");
            antiConfigs << QLatin1String("debug") << QLatin1String("debug_and_release");
        } else if (arg == QLatin1String("-no-editor")) {
            buildEditor = false;
            configs << QLatin1String("noeditor");
        } else if (arg == QLatin1String("-new-editor")) {
            configs << QLatin1String("neweditor");
            buildNewEditor = true;
        } else if (arg == QLatin1String("-no-gui")) {
            configs << QLatin1String("nogui");
        } else {
            qWarning( "Unknown option: %s", qPrintable(arg) );
            return 1;
        }
    }

    qtDir = new QString( QString::fromLocal8Bit(qgetenv("QTDIR")) );
    if ( qtDir->isEmpty() ) {
        qWarning( "%s: QTDIR not set", qPrintable(args.at(0)) );
        dumpMessages();
        return 1;
    }
    *qtDir += QLatin1String("/");

    if ( !checkLicense() ) {
        dumpMessages();
        return 1;
    }

    if( !writeQSConfig( buildIde, buildEditor, buildNewEditor ) ) {
        dumpMessages();
        return 1;
    }

    if (!writeExtraConfigs(configs)) {
        dumpMessages();
        return 1;
    }

    if (!writeQsaPrfFile(buildIde || buildEditor)) {
        dumpMessages();
        return 1;
    }

    if ( !qsa_prefix || qsa_prefix->isEmpty() )
        copyQSAHeaders();

    installDocs();

    runQMake( configs, antiConfigs, qsa_prefix ? *qsa_prefix : QString());

    if( errors ) {
        message(QLatin1String("\nThere were errors during configure!"));
    } else if( warnings ) {
        if (qdoc_warning) {
            message(QLatin1String("\nFailed to install documentation"));
        }
        if (qmake_warning) {
            message(QLatin1String("\nFailed to run qmake on top level .pro file\n"));
        }

        message(QLatin1String("\nThere were errors during configure, but these"
            "\ndo not appear to be fatal, so you should still be"
            "\nable to build QSA."
            "\nRun your make tool to build QSA."));
    } else {
        message(QLatin1String("\n"
            "Configuration completed successfully\n"
            "Run your make tool to build QSA"));
    }

    dumpMessages();
    return errors;
}

