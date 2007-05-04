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

void setQMake( const QString &str );
void runQMake( const QStringList &configs, const QStringList &antiConfigs, const QString &prefix );
void mkDir( const QString &dir );
bool writeQSConfig( bool buildIde );

#endif
