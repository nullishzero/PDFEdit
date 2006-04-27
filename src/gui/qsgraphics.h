#ifndef __QSGRAPHICS_H__
#define __QSGRAPHICS_H__

#include <qobject.h>
#include "qscobject.h"
#include <cobject.h>
#include <cgraphics.h>

namespace gui {

using namespace pdfobjects;

class QSGraphics : public QSCObject {
 Q_OBJECT
public:
 QSGraphics(CGraphic *gfx);
 QSGraphics(QSGraphics &source);
 virtual ~QSGraphics();
 CGraphic* get();
private:
 /** Object held in class*/
 CGraphic *obj;
};

} // namespace gui

#endif
