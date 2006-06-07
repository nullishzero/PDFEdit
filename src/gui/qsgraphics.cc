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

/** Construct wrapper with given CGraphic
 @param gfx CGraphic object
 @param _base scripting base
 */
QSGraphics::QSGraphics(CGraphic *gfx,BaseCore *_base) : QSCObject ("Graphic",_base) {
 obj=gfx;
}

/**
 Copy constructor
 @param source Source object to copy
 */
QSGraphics::QSGraphics(QSGraphics &source) : QSCObject ("Graphic",source.base) {
 obj=source.obj;
}

/** destructor */
QSGraphics::~QSGraphics() {
}

/** get CGraphics held inside this class. Not exposed to scripting */
CGraphic* QSGraphics::get() const {
 return obj;
}

} // namespace gui

//todo: incomplete
