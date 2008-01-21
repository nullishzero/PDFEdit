/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko,
 *                                              Miroslav Jahoda,
 *                                              Jozef Misutka,
 *                                              Martin Petricek
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit
 */
#ifndef __HELPWINDOW_H__
#define __HELPWINDOW_H__

#include "qtcompat.h"
#include <qwidget.h>
class Q_TextBrowser;
class QString;
class QLineEdit;

namespace gui {

/**
 class representing help window.<br>
 Help window is basically just a limited HTML browser
 \brief Help window
*/
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
private:
 /** Rich text browser with hypertext navigation. Core of this help window */
 Q_TextBrowser *help;
 /** Name of file loaded into help */
 QString fileName;
 /** Line edit control used to show/edit name of current file */
 QLineEdit *url;
};

}

#endif
