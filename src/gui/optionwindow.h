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

class OptionWindow : public QWidget {
 Q_OBJECT
public slots:
 void apply();
 void ok();
public:
 static void optionsDialog(Menu *msystem);
 ~OptionWindow();
protected:
 void closeEvent(QCloseEvent *e);
private:
 OptionWindow(Menu *msystem,QWidget *parent=0, const char *name=0);
 void init();
 QWidget* addTab(const QString name);
 void addOption(QWidget *otab,const QString &caption,Option *opt);
 void addOption(QWidget *otab,const QString &caption,const QString &key,const QString &defValue=QString::null);
 void addOptionCombo(QWidget *otab,const QString &caption,const QString &key,const QStringList &values);
 void addOptionInt(QWidget *otab,const QString &caption,const QString &key,int defValue=0);
 void addOptionBool(QWidget *otab,const QString &caption,const QString &key,bool defValue=false);
 void addOptionFloat(QWidget *otab,const QString &caption,const QString &key);
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

void applyLookAndFeel(bool notify=false);

} // namespace gui

#endif
