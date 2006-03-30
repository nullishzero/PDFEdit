#ifndef __QSGRAPHICS_H__
#define __QSGRAPHICS_H__

#include <qobject.h>
#include <cobject.h>
#include <cgraphics.h>
#include "qsdict.h"
#include "qscobject.h"

using namespace pdfobjects;

class QSGraphics : public QSDict {
 Q_OBJECT
public:
 QSGraphics(CGraphic *gfx);
 virtual ~QSGraphics();
 CGraphic* get();
private:
 /** Object held in class*/
 CGraphic *obj;
};

#endif
