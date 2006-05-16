#ifndef __PAGETOOL_H__
#define __PAGETOOL_H__

#include <qwidget.h>

class QLineEdit;
class QIntValidator;

namespace gui {

class QSPage;

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
