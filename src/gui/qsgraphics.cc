/** @file
 QObject wrapper around CGraphics<br>
 Export some functions to scripting.<br>
 See doc/user/scripting.xml or kernel documentation for more informations about these functions
 @author Martin Petricek
*/

#include "qsgraphics.h"
#include <cobject.h>
#include <cgraphics.h>

namespace gui {

using namespace pdfobjects;

/** Construct wrapper with given CGraphic */
QSGraphics::QSGraphics(CGraphic *gfx,Base *_base) : QSCObject ("CGraphic",_base) {
 obj=gfx;
}

/** Copy constructor */
QSGraphics::QSGraphics(QSGraphics &source) : QSCObject ("CGraphic",source.base) {
 obj=source.obj;
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
