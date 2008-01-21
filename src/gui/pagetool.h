/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko,
 *                                              Miroslav Jahoda,
 *                                              Jozef Misutka,
 *                                              Martin Petricek
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit
 */
#ifndef __PAGETOOL_H__
#define __PAGETOOL_H__

#include <qwidget.h>

class QResizeEvent;
class QLineEdit;
class QIntValidator;

namespace gui {

class QSPage;

/**
 Toolbutton showing current page number and allowing to change it<br>
 Can be placed in toolbar in place of ordinary button
 \brief Toolbar widget for changing page
*/
class PageTool : public QWidget {
Q_OBJECT
public:
 PageTool(QWidget *parent=0,const char *name=NULL);
 ~PageTool();
 QSize sizeHint() const;
public slots:
 void updatePage(int page);
 void updatePage(const QSPage &p,int page);
signals:
 /**
  Signal emitted when user changes the page
  @param page New page
 */
 void pageSet(int page);
protected:
 void resizeEvent (QResizeEvent *e);
 void updatePage();
 void selectPage(const QString &newPage);
protected slots:
 void selectPage();
protected:
 /** Page selection editbox */
 QLineEdit *pageNum;
 /** Validator for zoom values */
 QIntValidator *ival;
 /** Current page */
 int curPage;
};

} // namespace gui

#endif
