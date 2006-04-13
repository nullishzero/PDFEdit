#ifndef __UTIL_H__
#define __UTIL_H__

class QString;
class QStringList;

/** macro returning greater of two values */
#define MAX(x,y) ((x)>=(y)?(x):(y))

/** macro returning lesser of two values */
#define MIN(x,y) ((x)<=(y)?(x):(y))

void fatalError(const QString message);
QStringList explode(char separator,const QString line);
QString htmlEnt(const QString &str);
QString loadFromFile(QString name);
void printList(QStringList l);

#endif
