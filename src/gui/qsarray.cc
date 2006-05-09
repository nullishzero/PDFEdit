/** @file
 QObject wrapper around CArray<br>
 Export some functions to scripting.<br>
 See doc/user/scripting.xml or kernel documentation for more informations about these functions
 @author Martin Petricek
*/

#include "qsarray.h"
#include <cobject.h>

namespace gui {

using namespace pdfobjects;

/** Construct wrapper with given CArray */
QSArray::QSArray(boost::shared_ptr<CArray> _array,Base *_base) : QSIProperty (_array,"Array",_base) {
}

/** Copy constructor */
QSArray::QSArray(QSArray &source) : QSIProperty (source.obj,"Array",source.base) {
}

/** destructor */
QSArray::~QSArray() {
}

/** get CArray held inside this class. Not exposed to scripting */
boost::shared_ptr<CArray> QSArray::get() {
 return boost::dynamic_pointer_cast<CArray>(obj);
}

/** call CArray::getStringRepresentation(ret); return ret */
QString QSArray::getText() {
 CArray *array=dynamic_cast<CArray*>(obj.get());
 std::string text;
 array->getStringRepresentation(text);
 return text;
}

} // namespace gui

//todo: incomplete
