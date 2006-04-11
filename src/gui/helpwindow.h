#ifndef __HELPWINDOW_H__
#define __HELPWINDOW_H__


#include <qtextbrowser.h> 
#include <qwidget.h>

namespace gui {

class HelpWindow : public QWidget {
Q_OBJECT
public:
 HelpWindow(const QString &fileName,QWidget *parent=0,const char *name=0);
 ~HelpWindow();
protected:
 void closeEvent(QCloseEvent *e);
private:
 /** Rich text browser with hypertext navigation. Core of this help window */
 QTextBrowser *help;
};

}

#endif
