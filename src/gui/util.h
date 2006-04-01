#ifndef __UTIL_H__
#define __UTIL_H__
#include <qstring.h>
#include <qstringlist.h>
#include <iproperty.h>
#include <qobject.h> 

using namespace pdfobjects;

/** macro returning greater of two values */
#define max(x,y) ((x)>=(y)?(x):(y))
//todo: velkym pismenama nebo jako template fce

/** macro returning lesser of two values */
#define min(x,y) ((x)<=(y)?(x):(y))


void fatalError(const QString message);
QStringList explode(char separator,const QString line);
QString htmlEnt(const QString &str);
QString loadFromFile(QString name);
QString getTypeName(PropertyType typ);

#endif
