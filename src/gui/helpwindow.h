#ifndef __HELPWINDOW_H__
#define __HELPWINDOW_H__

#include <qwidget.h>
class QTextBrowser;
class QString;
class QLineEdit;

namespace gui {

class HelpWindow : public QWidget {
Q_OBJECT
public:
 HelpWindow(const QString &pageName=QString::null,QWidget *parent=0,const char *name=0);
 ~HelpWindow();
public slots:
 void goUrl();
 void setFile(const QString &fName);
 void index();
 void setPage(QString name);
protected:
 void closeEvent(QCloseEvent *e);
private:
 /** Rich text browser with hypertext navigation. Core of this help window */
 QTextBrowser *help;
 /** Name of file loaded into help */
 QString fileName;
 /** Line edit control used to show/edit name of current file */
 QLineEdit *url;
};

}

#endif
