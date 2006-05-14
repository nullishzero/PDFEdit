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
QSPage::QSPage(boost::shared_ptr<CPage> _page,Base *_base) : QSCObject ("Page",_base) {
 obj=_page;
 numStreams=-1;
}

/** Copy constructor */
QSPage::QSPage(const QSPage &source) : QSCObject ("Page",source.base) {
 obj=source.obj;
 numStreams=-1;
}

/** destructor */
QSPage::~QSPage() {
}

/** Call CPage::getDictionary */
QSDict* QSPage::getDictionary() {
 return new QSDict(obj->getDictionary(),base);
}

/** Call CPage::getText(ret); return ret */
QString QSPage::getText() {
 std::string text;
 obj->getText(text);
 return text;
}

/**
 Return stream with given number from page.
 Get the streams from CPage and store for later use if necessary
 @param streamNumber number of stream to get
 @return ContentStream with given number, or NULL if number is outside range
*/
QSContentStream* QSPage::getContentStream(int streamNumber) {
 if (numStreams<0) getContentStreams();
 if (streamNumber<0) return NULL;		//Stream number outside range
 if (streamNumber>=numStreams) return NULL;	//Stream number outside range
 return new QSContentStream(streams[streamNumber],base);
}

/**
 Return number of content streams in page
 Get the streams from CPage and store for later use if necessary
 @return number of streams in this Page
*/
int QSPage::getContentStreamCount() {
 if (numStreams<0) getContentStreams();
 return numStreams;
}

/** Call CPage::getContentStreams(), store result */
void QSPage::getContentStreams() {
 obj->getContentStreams(streams);
 //Store number of streams
 numStreams=streams.size(); 
}

/** get CPage held inside this class. Not exposed to scripting */
boost::shared_ptr<CPage> QSPage::get() const {
 return obj;
}

} // namespace gui

//todo: incomplete
