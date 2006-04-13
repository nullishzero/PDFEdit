#ifndef __OPTIONWINDOW_H__
#define __OPTIONWINDOW_H__

#include <qwidget.h>
#include <qdict.h>
#include <qmap.h>
class QLabel;
class QTabWidget;
class QGridLayout;

namespace gui {

class Menu;
class Option;
/** Pointer to running Options window or NULL if none active. Instance is in optionwindow.cc */
class OptionWindow;
extern OptionWindow *opt;

class OptionWindow : public QWidget {
//ADDED functions begin
//ADDED functions end
 Q_OBJECT
public slots:
 void apply();
 void ok();
public:
 /** invoke option dialog. Ensure only one copy is running at time
 @param msystem Menu system to 
 */
 static void optionsDialog(Menu *msystem) {
  if (opt) { //the dialog is already active
   opt->setActiveWindow();
  } else { //create new dialog
   opt=new OptionWindow(msystem);
   opt->show();
  }
 }
 ~OptionWindow();
protected:
 void closeEvent(QCloseEvent *e);
private:
 OptionWindow(Menu *msystem,QWidget *parent=0, const char *name=0);
 void init();
 QWidget* addTab(const QString name);
 void addOption(QWidget *otab,const QString &caption,Option *opt);
 void addOption(QWidget *otab,const QString &caption,const QString &key);
 void addOptionBool(QWidget *otab,const QString &caption,const QString &key,bool defValue=false);
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
 /** Menu system (for toolbar list ... ) */
 Menu *menuSystem;
};

} // namespace gui

#endif
