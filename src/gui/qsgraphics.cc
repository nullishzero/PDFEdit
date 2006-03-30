/** @file
 QObject wrapper around CGraphics
*/

#include "qsgraphics.h"

/** Construct wrapper with given CDict */
QSGraphics::QSGraphics(CGraphic *gfx) : QSDict (gfx) {
}

/** destructor */
QSGraphics::~QSGraphics() {
}

/** get CGraphics held inside this class. Not exposed to scripting */
CGraphic* QSGraphics::get() {
 return obj;
}

//todo: incomplete
