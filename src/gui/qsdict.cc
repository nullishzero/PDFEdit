/** @file
 QObject wrapper around CDict
 Export some functions to scripting.<br>
 See doc/user/scripting.xml or kernel documentation for more informations about these functions
 @author Martin Petricek
*/

#include "qsdict.h"
#include <cobject.h>

namespace gui {

using namespace pdfobjects;

/** Construct wrapper with given CDict */
QSDict::QSDict(CDict *_dict) : QSCObject ("Dict") {
 obj=_dict;
}

/** destructor */
QSDict::~QSDict() {
}

/** get CDict held inside this class. Not exposed to scripting */
CDict* QSDict::get() {
 return obj;
}

/** call CDict::getStringRepresentation(ret); return ret */
QString QSDict::getText() {
 std::string text;
 obj->getStringRepresentation(text);
 return text;
}

} // namespace gui

//todo: incomplete
