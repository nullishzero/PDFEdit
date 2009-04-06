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
#ifndef __SETTINGS_H__
#define __SETTINGS_H__

#include <qobject.h>
class QSettings;
class QSplitter;
class QStringList;
class QString;

namespace gui {

class StaticSettings;

/**
 Class managing settings, saving and loading single settings or window states
 \brief Saving and loading settings
 */
class Settings : public QObject {
 Q_OBJECT
public:
 ~Settings();
 static Settings* getInstance();
 void saveWindow(QWidget *win,const QString &name);
 void restoreWindow(QWidget *win,const QString &name);
 void saveSplitter(QSplitter *spl,const QString &name);
 void restoreSplitter(QSplitter *spl,const QString &name);
 QStringList readPath(const QString &name,const QString &prefix="path/");
 QStringList readList(const QString &name,const QString &separator=",");
public slots:
 void flush();
 QString readExpand(const QString &key,const QString &defValue=QString::null);
 QString read(const QString &key,const QString &defValue=QString::null);
 bool readBool(const QString &key,bool defValue=false);
 int readNum(const QString &key,int defValue=0);
 void write(const QString &key,const QString &value);
 void write(const QString &key, int value);
 void remove(const QString &key);
 void removeAll(const QString &key);
 QString expand(QString s);
 QString getFullPathName(const QString &nameOfPath,QString fileName=QString::null,const QString &prefix=QString::null);
signals:
 /** Signal emitted when any setting is changed. Key of changed settings is sent.
     Can be used to monitor settings changes */
 void settingChanged(QString);
private:
 //Constructor is private, use getInstance
 Settings();
 void init();
 void initSettings();
private:
 /** Settings object used to load configuration
  These settings takle precedence before staticSettings */
 QSettings *set;
 /** Settings object used to load static configuration (menu, etc ...)
  These settings are never changed (and never written) */
 StaticSettings *staticSet;
};

/** One object for application, holding all global settings. */
extern Settings *globalSettings;

} // namespace gui

#endif
