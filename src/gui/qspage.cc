/** @file
 QObject wrapper around CPage (Page in document)<br>
 Export some functions to scripting.<br>
 See doc/user/scripting.xml or kernel documentation for more informations about these functions
 @author Martin Petricek
*/

#include "qspage.h"
#include <qstring.h>
#include "qscontentstream.h"

namespace gui {

/** Construct wrapper with given CPage */
QSPage::QSPage(boost::shared_ptr<CPage> _page) : QSCObject ("Page") {
 obj=_page;
}

/** destructor */
QSPage::~QSPage() {
}

/** Call CPage::getDictionary */
QSDict* QSPage::getDictionary() {
 return new QSDict(obj->getDictionary().get());
}

/** Call CPage::getText(ret); return ret */
QString QSPage::getText() {
 std::string text;
 obj->getText(text);
 return text;
}

/** Call CPage::getContentStream() */
QSContentStream* QSPage::getContentStream() {
 return new QSContentStream(obj->getContentStream());
}

/** get CPage held inside this class. Not exposed to scripting */
boost::shared_ptr<CPage> QSPage::get() {
 return obj;
}

} // namespace gui

//todo: incomplete
