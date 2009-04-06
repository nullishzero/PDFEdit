/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006-2009  PDFedit team: Michal Hocko,
 *                                        Jozef Misutka,
 *                                        Martin Petricek
 *                   Former team members: Miroslav Jahoda
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (in doc/LICENSE.GPL); if not, write to the 
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 * MA  02111-1307  USA
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit
 */
/** @file
 StaticSettings - class handling static settings stored in one file.
 Format of the file is almost identical to "ini file", but the format
 is enhanced to allow indentation and comments to be put into the file
 and also the encoding of the file is always assumed to be utf-8.
 It can read almost all files that QSettings can read, but not all
 files this can read can be read by QSettings
 @author Martin Petricek
*/

#include "staticsettings.h"
#include <qfile.h>
#include <qstringlist.h>
#include <qregexp.h>
#include <qtextstream.h>

namespace gui {

/**
 Default constructor
*/
StaticSettings::StaticSettings() {
 //Empty
}

/**
 Default destructor
*/
StaticSettings::~StaticSettings() {
 //Empty
}

/**
 Convenience overloaded function providing file parameter
 separated to file and directory name
 \see tryLoad(const QString &)
 @param dirName directory name
 @param fileName file name
*/
bool StaticSettings::tryLoad(const QString &dirName,const QString &fileName) {
 return tryLoad(dirName+"/"+fileName);
}

/**
 Try to load settings from given file in given directory.
 If succesful, return true and any settings loaded so far
 are reset and replaced by settings from given file
 If unsuccessful, return false
 @param fileName file name
*/
bool StaticSettings::tryLoad(const QString &fileName) {
 QFile f(fileName);
 if (!f.open(IO_ReadOnly)) return false; //File can't be opened
 QTextStream cfg(&f);
 cfg.setEncoding(QTextStream::UnicodeUTF8);
 QString line;
 QString mainKey="";
 QRegExp nextHead("\\[(.*)(\\]|#|;)");
 nextHead.setMinimal(true);
 set.clear();
 for(;;) {
  //Read till we get EOF
  line=cfg.readLine();
  if (line.isNull()) return true;
  line=line.stripWhiteSpace();
  if (line=="") continue;		//Empty line
  if (line[0]=='#' || line[0]==';') continue;	//Comment line
  if (line[0]=='[') {
   //Possibly heading with group name
   int pos=nextHead.search(line);
   if (pos<0) continue;			//Bad/invalid heading
   if (nextHead.cap(2)!="]") continue;	//Bad/invalid heading
   mainKey=nextHead.cap(1)+"/";
   continue;
  }
  //TODO: check for extra comment at end or something like that
  QString key=line.section('=',0,0).stripWhiteSpace();
  QString value=line.section('=',1);
  if (value.isNull()) continue; //Bad line format
  value=value.stripWhiteSpace();
  set.insert(mainKey+key,value);
 }
}


/**
 Return list of all keys starting with given prefix followed by "/"
 Warning: this function is not efficient, as it traverse all the settings to find the result
 @param prefix Prefix of items
 @return list of keys with this prefix
*/
QStringList StaticSettings::entryList(const QString &prefix) {
 QStringList ret;
 QMap<QString,QString>::Iterator it;
 QString pr=prefix+"/";
 int len=pr.length();
 for (it=set.begin();it!=set.end();++it) {
  QString key=it.key();
  if (key.startsWith(pr)) {
   ret+=key.mid(len);
  }
 }
 return ret;
}

/**
 Read settings with given key from configuration file and return as QString
 @param key Key to read from settings
 @param defValue default value to use if key not found in settings.
 @return Value of given setting
*/
QString StaticSettings::readEntry(const QString &key,const QString &defValue/*=QString::null*/) {
 if (!set.contains(key)) return defValue;
 return set[key];
}

} // namespace gui
