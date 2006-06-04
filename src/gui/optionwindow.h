#ifndef __OPTIONWINDOW_H__
#define __OPTIONWINDOW_H__

#include <qwidget.h>
#include <qdict.h>
#include <qmap.h>
class QLabel;
class QTabWidget;
class QFrame;
class QGridLayout;

namespace gui {

class Menu;
class Option;

/**
 Widget for editing program options<br>
 Options are arranged to tabs and it is ensured, that only one dialog at once is active
 (via Private constructor and static method to invoke the dialog, which will focus on
 existing dialog if it exists, instead of creating second one) 
*/
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
 QWidget* addTab(const QString name,bool makeSegments=false);
 void initGridFrame(QWidget *grid);
 void addOption(QWidget *otab,const QString &caption,Option *opt);
 void addOption(QWidget *otab,const QString &caption,const QString &key,const QString &defValue=QString::null);
 void addOptionFile(QWidget *otab,const QString &caption,const QString &key,const QString &defValue=QString::null);
 void addOptionFont(QWidget *otab,const QString &caption,const QString &key,const QString &defValue=QString::null);
 void addOptionCombo(QWidget *otab,const QString &caption,const QString &key,const QStringList &values);
 void addOptionInt(QWidget *otab,const QString &caption,const QString &key,int defValue=0);
 void addOptionBool(QWidget *otab,const QString &caption,const QString &key,bool defValue=false);
 void addOptionFloat(QWidget *otab,const QString &caption,const QString &key);
 void addWidget(QWidget *otab,QWidget *elem);
 void addText(QWidget *otab,const QString &text);
 QWidget* addBreak(QWidget *otab);
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
 /** Master Grid for the tab */
 QMap<QWidget*,QFrame*> masterGrid;
 /** Number of objects in the tab */
 QMap<QWidget*,int> nObjects;
 /** Menu system (for toolbar list ... ) */
 Menu *menuSystem;
};

void applyLookAndFeel(bool notify=false);

} // namespace gui

#endif
