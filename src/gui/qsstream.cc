/** @file
 QObject wrapper around CStream<br>
 Export some functions to scripting.<br>
 See doc/user/scripting.xml or kernel documentation for more informations about these functions
 @author Martin Petricek
*/

#include "qsstream.h"

namespace gui {

/** Construct wrapper with given CStream */
QSStream::QSStream(boost::shared_ptr<CStream> _cs) : QSIProperty (_cs,"Stream") {
}

/** destructor */
QSStream::~QSStream() {
}

} // namespace gui

//todo: incomplete
