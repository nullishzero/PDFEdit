#ifndef CONFIGUTILS_H
#define CONFIGUTILS_H

#include <qstring.h>
#include <qstringlist.h>
#include <qdir.h>

extern QString *qtDir;
extern int processes;
extern int errors;
extern int warnings;
extern bool autoShutdown;


void message( const QString &msg );
QStringList messages();

void runQMake( const QStringList &configs, const QString &prefix );
void mkDir( const QString &dir );
void copy( const QString &source, const QString &target );
void symLink( const QString &source, const QString &target );
bool writeQSConfig( bool buildIde );
void rmDirRecursive( const QDir &dir );



#endif
