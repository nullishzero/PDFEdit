#ifndef __SETTINGS_H__
#define __SETTINGS_H__
#include "config.h"
#include <qsettings.h>
#include <qsplitter.h>
#include <qstringlist.h> 
#include <qstring.h>

/** Class managing settings and also loading configurable menus, toolbars and keyboard shortcuts */

class Settings : public QObject {
 Q_OBJECT
public:
 ~Settings();
 /** get Instance of Settings. Ensures only one instance of Settings is avalable */
 static Settings* getInstance() {
  static Settings* globalSettings=NULL;
  if (!globalSettings) globalSettings=new Settings();
  return globalSettings;
 }
 void saveWindow(QWidget *win,const QString name); 
 void restoreWindow(QWidget *win,const QString name);
 void saveSplitter(QSplitter *spl,const QString name);
 void restoreSplitter(QSplitter *spl,const QString name);
 QStringList readPath(const QString &name);
 QStringList readList(const QString &name,const QString separator=",");
public slots:
 void flushSettings();
 QString readExpand(const QString &key,const QString defValue=QString::null);
 QString read(const QString &key,const QString defValue=QString::null);
 bool readBool(const QString &key,bool defValue=false);
 int readNum(const QString &key,int defValue=0);
 void write(const QString &key,const QString &value);
 void write(const QString &key, int value);
 QString expand(QString s);
 QString getFullPathName( QString nameOfPath , QString fileName );
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
 QSettings *staticSet;
};

/** One object for application, holding all global settings. */
extern Settings *globalSettings;

#endif
