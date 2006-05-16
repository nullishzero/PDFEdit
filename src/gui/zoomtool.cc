/** @file
 ZoomTool - Toolbutton allowing to change zoom level
 @author Martin Petricek
*/

#include <utils/debug.h>
#include "zoomtool.h"
#include <qstring.h>
#include <qcombobox.h>
#include <qvalidator.h>
#include "settings.h"

namespace gui {

/**
 Default constructor of ZoomTool
 @param parent Toolbar containing this control
 @param name Name of this widget (passed to QWidget constructor)
*/
ZoomTool::ZoomTool(QWidget *parent/*=0*/,const char *name/*=NULL*/) : QWidget (parent,name) {
 zoomList=new QComboBox(this,"Zoom_select");
 ival=new QRegExpValidator(QRegExp("\\d?\\d?\\d?\\d%?"),this);
 zoomList->setEditable(true);
 zoomList->setValidator(ival);
 zoomList->setInsertionPolicy(QComboBox::NoInsertion);
 zoomList->insertItem("25%");
 zoomList->insertItem("50%");
 zoomList->insertItem("75%");
 zoomList->insertItem("100%");
 zoomList->insertItem("150%");
 zoomList->insertItem("200%");
 zoomList->insertItem("400%");
 QObject::connect(zoomList,SIGNAL(activated(const QString&)),this,SLOT(selectZoom(const QString&)));
}

/** default destructor */
ZoomTool::~ZoomTool() {
 delete zoomList;
 delete ival;
}


/**
 return size hint of this control
 @return size hint from inner Zoom list
*/
QSize ZoomTool::sizeHint() const {
 return zoomList->sizeHint();
}

/**
 Called on resizing of property editing control
 Will simply set the same fixed size to inner Zoom list
 @param e resize event
*/
void ZoomTool::resizeEvent (QResizeEvent *e) {
 zoomList->setFixedSize(e->size());
}

/**
 Called on setting new zoom from this control
 @param newZoom
*/
void ZoomTool::selectZoom(const QString &newZoom) {
 QString theZoom=newZoom;
 if (theZoom.endsWith("%")) theZoom.truncate(theZoom.length()-1);
 float zoom=theZoom.toFloat()/100.0;
 guiPrintDbg(debug::DBG_DBG,"Zoom level set: " << zoom);
 //Maintain some reasonable limits
 if (zoom<0.01) zoom=0.01;
 if (zoom>40) zoom=40;
 emit zoomSet(zoom);
}

/**
 Called to update control after selecting a new Zoom outside of this control
 @param Zoom selected Zoom
 */
void ZoomTool::updateZoom(float zoom) {
 int percent=(int)(zoom*100.0);
 zoomList->setCurrentText(QString::number(percent)+"%");
}

} // namespace gui
