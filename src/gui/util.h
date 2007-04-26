/*                                                                              
 * PDFedit - free program for PDF document manipulation.                        
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko, 
 *                                              Miroslav Jahoda,       
 *                                              Jozef Misutka, 
 *                                              Martin Petricek                                             
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit                                                                      
 */ 
#ifndef __UTIL_H__
#define __UTIL_H__
/** @file
 \brief Utility functions header
*/

#include <string>

class QColor;
class QString;
class QStringList;
class QWidget;
class QVariant;

/** macro returning greater of two values */
#define MAX(x,y) ((x)>=(y)?(x):(y))

/** macro returning lesser of two values */
#define MIN(x,y) ((x)<=(y)?(x):(y))

#ifndef __GNUC__
/** Turn off __attribute__ for non-gcc compilers */
#define  __attribute__(x) /* Turn off __attribute__ for non-gcc compilers */ 
#endif

namespace util {


/**
 Context of unicode to/from 8bit conversion
 (PDF, console, filename ...)
*/
enum CharsetContext {
 PDF, /** PDF */
 CON, /** Console/terminal */
 NAME, /** File name */
 UTF8, /** Use UTF8 (content of pdfedit help/resource files) */
};

void fatalError(const QString &message);
QStringList explode(char separator,const QString &line,bool escape=false);
QString htmlEnt(const QString &str);
QString loadFromFile(const QString &name);
bool saveToFile(const QString &name,const QString &content);
void printList(const QStringList &l);
void escapeSlash(QString &line);
void consoleLog(const QString &message,const QString &fileName);
QString getUntil(char separator,QString &line,bool escape=false);
void setDebugLevel(const QString &param);
QString countString(int count,QString singular,QString plural);
QColor mixColor(const QColor &oldColor,double weight,const QColor &newColor);
void colorMod(QWidget* widget,QColor fg,double weight_fg,QColor bg,double weight_bg);
QStringList countList(unsigned int count,unsigned int start=0);
QVariant varFromDoubleArray(double *d,int count);
int varToDoubleArray(const QVariant &v,double *out,int out_size);

//Charset encoding related functions
QStringList supportedEncodings();
void setDefaultCharset(const QString &charsetName);
QString convertToUnicode(const std::string &str, CharsetContext ctx);
std::string convertFromUnicode(const QString &str, CharsetContext ctx);

} //namespace util


#endif
