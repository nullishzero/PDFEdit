#ifndef __ZOOMTOOL_H__
#define __ZOOMTOOL_H__

#include <qwidget.h>

class QComboBox;
class QValidator;

namespace gui {

class ZoomTool : public QWidget {
Q_OBJECT
public:
 ZoomTool(QWidget *parent=0,const char *name=NULL);
 ~ZoomTool();
 QSize sizeHint() const;
public slots:
 void updateZoom(float zoom);
signals:
 /**
  Signal emitted when user changes the zoom level
  @param zoom New zoom level
 */
 void zoomSet(float zoom);
protected:
 void resizeEvent (QResizeEvent *e);
protected slots:
 void selectZoom(const QString &newZoom);
protected:
 /** Zoom selection editable combo */
 QComboBox *zoomList;
 /** Validator for zoom values */
 QValidator *ival;
};

} // namespace gui

#endif
