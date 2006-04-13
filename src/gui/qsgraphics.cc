/** @file
 QObject wrapper around CGraphics
*/

#include "qsgraphics.h"
#include <cobject.h>
#include <cgraphics.h>

namespace gui {

using namespace pdfobjects;

/** Construct wrapper with given CGraphic */
QSGraphics::QSGraphics(CGraphic *gfx) : QSCObject ("CGraphic") {
}

/** destructor */
QSGraphics::~QSGraphics() {
}

/** get CGraphics held inside this class. Not exposed to scripting */
CGraphic* QSGraphics::get() {
 return obj;
}

} // namespace gui

//todo: incomplete
