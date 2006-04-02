/** @file
 QObject wrapper around CPage
*/

#include "qspage.h"

/** Construct wrapper with given CPage */
QSPage::QSPage(boost::shared_ptr<CPage> _page) : QSCObject ("Page") {
 obj=_page;
}

/** destructor */
QSPage::~QSPage() {
}


/** Call CPage::getDictionary */
QSDict *QSPage::getDictionary() {
 return new QSDict(obj->getDictionary().get());
}

/** Call CPage::getPageNumber */
unsigned int QSPage::getPageNumber() {
 return (unsigned int)obj->getPageNumber();
}

/** Call CPage::setPageNumber(num) */
void QSPage::setPageNumber(unsigned int num) {
 obj->setPageNumber(num);
}

/** Call CPage::getText(ret); return ret */
QString QSPage::getText() {
 std::string text;
 obj->getText(text);
 return text;
}

/** get CPage held inside this class. Not exposed to scripting */
boost::shared_ptr<CPage> QSPage::get() {
 return obj;
}

//todo: incomplete
