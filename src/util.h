#ifndef __UTIL_H__
#define __UTIL_H__
#include <qstring.h>
#include <qstringlist.h>

void fatalError(const QString message);
QStringList explode(char separator,const QString line);

#endif
