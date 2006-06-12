#ifndef __UTIL_H__
#define __UTIL_H__
/** @file
 \brief Utility functions header
*/

class QColor;
class QString;
class QStringList;
class QWidget;

/** macro returning greater of two values */
#define MAX(x,y) ((x)>=(y)?(x):(y))

/** macro returning lesser of two values */
#define MIN(x,y) ((x)<=(y)?(x):(y))

#ifndef __GNUC__
/** Turn off __attribute__ for non-gcc compilers */
#define  __attribute__(x) /* Turn off __attribute__ for non-gcc compilers */ 
#endif

namespace util {

void fatalError(const QString &message);
QStringList explode(char separator,const QString &line,bool escape=false);
QString htmlEnt(const QString &str);
QString loadFromFile(const QString &name);
void printList(const QStringList &l);
void escapeSlash(QString &line);
void consoleLog(const QString &message,const QString &fileName);
QString getUntil(char separator,QString &line,bool escape=false);
void setDebugLevel(const QString &param);
QString countString(int count,QString singular,QString plural);
QColor mixColor(const QColor &oldColor,double weight,const QColor &newColor);
void colorMod(QWidget* widget,QColor fg,double weight_fg,QColor bg,double weight_bg);
QStringList countList(unsigned int count,unsigned int start=0);

} //namespace util


#endif
