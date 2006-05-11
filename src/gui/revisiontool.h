#ifndef __REVISIONTOOL_H__
#define __REVISIONTOOL_H__

#include <qwidget.h>

class QComboBox;

namespace pdfobjects {
 class CPdf;
}

namespace gui {

using namespace pdfobjects;

class RevisionTool : public QWidget {
Q_OBJECT
public:
 RevisionTool(QWidget *parent=0,const char *name=NULL);
 ~RevisionTool();
 QSize sizeHint() const;
public slots:
 void selectRevision(int revision);
 void setDocument(CPdf *newDocument);
protected:
 void resizeEvent (QResizeEvent *e);
protected:
 /** Revision selection list */
 QComboBox *revList;
 /** Reference to current PDF document */
 CPdf *document;
};

} // namespace gui

#endif
