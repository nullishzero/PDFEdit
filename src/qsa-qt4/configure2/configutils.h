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
#ifndef CONFIGUTILS_H
#define CONFIGUTILS_H

#include <qstring.h>
#include <qstringlist.h>
#include <qdir.h>

extern QString *qtDir;
extern QString *qsa_prefix;

extern int errors;
extern int warnings;
extern bool autoShutdown;

extern bool qdoc_warning;
extern bool qmake_warning;

void message( const QString &msg );
QStringList messages();

QString licenseFile();
bool isUsLicense();

void runQMake( const QStringList &configs, const QStringList &antiConfigs, const QString &prefix );
void copyQSAHeaders();
void installDocs();
void mkDir( const QString &dir );
void copy( const QString &source, const QString &target );
void symLink( const QString &source, const QString &target );
bool checkLicense();
bool writeQSConfig(bool buildIde, bool buildEditor, bool buildNewEditor);
uint convertor( const QString &list );
void rmDirRecursive( const QDir &dir );
bool writeExtraConfigs(const QStringList &configs);
bool writeQsaPrfFile(bool include_qt3_support);

#endif
