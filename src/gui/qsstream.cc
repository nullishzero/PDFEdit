/** @file
 QObject wrapper around CStream<br>
 Export some functions to scripting.<br>
 See doc/user/scripting.xml or kernel documentation for more informations about these functions
 @author Martin Petricek
*/

#include "qsstream.h"

namespace gui {

/** Construct wrapper with given CStream */
QSStream::QSStream(boost::shared_ptr<CStream> _cs) : QSCObject ("Stream") {
 obj=_cs;
}

/** destructor */
QSStream::~QSStream() {
}

/** Call CStream::getStringRepresentation(ret); return ret */
QString QSStream::getText() {
 std::string text;
 obj->getStringRepresentation(text);
 return text;
}

/** get CStream held inside this class. Not exposed to scripting */
boost::shared_ptr<CStream> QSStream::get() {
 return obj;
}

} // namespace gui

//todo: incomplete
