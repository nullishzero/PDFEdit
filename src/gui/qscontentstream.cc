/** @file
 QObject wrapper around CContentStream (content stream contained in a page)<br>
 Export some functions to scripting.<br>
 See doc/user/scripting.xml or kernel documentation for more informations about these functions
 @author Martin Petricek
*/

#include "qscontentstream.h"

namespace gui {

/** Construct wrapper with given CContentStream */
QSContentStream::QSContentStream(boost::shared_ptr<CContentStream> _cs) : QSCObject ("ContentStream") {
 obj=_cs;
}

/** destructor */
QSContentStream::~QSContentStream() {
}

/** Call CContentStream::getStringRepresentation(ret); return ret */
QString QSContentStream::getText() {
 std::string text;
 obj->getStringRepresentation(text);
 return text;
}

/** get CContentStream held inside this class. Not exposed to scripting */
boost::shared_ptr<CContentStream> QSContentStream::get() {
 return obj;
}

} // namespace gui

//todo: incomplete
