#ifndef __UTIL_H__
#define __UTIL_H__
#include <qstring.h>
#include <qstringlist.h>

/** macro returning greater of two values */
#define max(x,y) ((x)>=(y)?(x):(y))

/** macro returning lesser of two values */
#define min(x,y) ((x)<=(y)?(x):(y))


void fatalError(const QString message);
QStringList explode(char separator,const QString line);

#endif
