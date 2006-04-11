#ifndef __UTIL_H__
#define __UTIL_H__
#include <qstring.h>
#include <qstringlist.h>
#include <iproperty.h>
#include <qobject.h> 

/** macro returning greater of two values */
#define MAX(x,y) ((x)>=(y)?(x):(y))

/** macro returning lesser of two values */
#define MIN(x,y) ((x)<=(y)?(x):(y))

void fatalError(const QString message);
QStringList explode(char separator,const QString line);
QString htmlEnt(const QString &str);
QString loadFromFile(QString name);
QString getTypeName(pdfobjects::PropertyType typ);
void printList(QStringList l);

#endif
