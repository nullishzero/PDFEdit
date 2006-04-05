#ifndef __OPTIONWINDOW_H__
#define __OPTIONWINDOW_H__

#include <qwidget.h>
#include <qlabel.h>
#include <qdict.h>
#include <qmap.h>
#include <qtabwidget.h> 
#include <qlayout.h>
#include "option.h"

/** Pointer to running Options window or NULL if none active. Instance is in optionwindow.cc */
class OptionWindow;
extern OptionWindow *opt;

class OptionWindow : public QWidget {
 Q_OBJECT
public slots:
 void apply();
 void ok();
public:
 /** invoke option dialog. Ensure only one copy is running at time */
 static void optionsDialog() {
  if (opt) { //the dialog is already active
   opt->setActiveWindow();
  } else { //create new dialog
   opt=new OptionWindow();
   opt->show();
  }
 }
 ~OptionWindow();
protected:
 void closeEvent(QCloseEvent *e);
private:
 OptionWindow(QWidget *parent=0, const char *name=0);
 void init();
 QWidget* addTab(const QString name);
 void addOption(QWidget *otab,const QString &caption,Option *opt);
 void addOption(QWidget *otab,const QString &caption,const QString &key);
 void addOptionBool(QWidget *otab,const QString &caption,const QString &key);
 void addOptionFloat(QWidget *otab,const QString &caption,const QString &key);
 void addOptionInt(QWidget *otab,const QString &caption,const QString &key);
 void addWidget(QWidget *otab,QWidget *elem);
 void addText(QWidget *otab,const QString &text);
 void finishTab(QWidget *otab);
private:
 /** List of property names */
 QStringList *list;
 /** Dictionary wit property items */
 QDict<Option> *items;
 /** Dictionary wit property labels */
 QDict<QLabel> *labels;
 /** Main tab widget */ 
 QTabWidget *tab;
 /** Grid layout for the tab */
 QMap<QWidget*,QGridLayout*> gridl;
 /** Number of objects in the tab */
 QMap<QWidget*,int> nObjects;
};

#endif
