/** @file
 QObject wrapper around CPage (Page in document)<br>
 Export some functions to scripting.<br>
 See doc/user/scripting.xml or kernel documentation for more informations about these functions
 @author Martin Petricek
*/

#include "qspage.h"
#include "qsdict.h"
#include <qstring.h>
#include <qrect.h>
#include <qvariant.h>
#include "qscontentstream.h"
#include <cpage.h>

namespace gui {

using namespace std;

/** List of fonts returned ftom CPage::getFontIdsAndNames */
typedef vector<pair<string,string> > FontList;

/**
 Construct wrapper with given CPage
 @param _page CPage shared pointer
 @param _base scripting base
*/
QSPage::QSPage(boost::shared_ptr<CPage> _page,Base *_base) : QSCObject ("Page",_base) {
 obj=_page;
 numStreams=-1;
}

/**
 Copy constructor
 @param source Source page to copy
*/
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
 if (numStreams<0) loadContentStreams();
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
 if (numStreams<0) loadContentStreams();
 return numStreams;
}

/** Call CPage::getContentStreams(), store result */
void QSPage::loadContentStreams() {
 obj->getContentStreams(streams);
 //Store number of streams
 numStreams=streams.size(); 
}

/** Call CPage::getMediabox() */
QVariant QSPage::mediabox() {
 //QStringList will be "autoconverted" to Array in QSA
 Rectangle r=obj->getMediabox();
 QValueList<QVariant> rect;
 rect.append(r.xleft);
 rect.append(r.yleft);
 rect.append(r.xright);
 rect.append(r.yright);
 return QVariant(rect);
}

/** Call CPage::setMediabox() */
void QSPage::setMediabox(double x1,double y1,double x2,double y2) {
 Rectangle r(x1,y1,x2,y2);
 obj->setMediabox(r);
}

/** Call CPage::setMediabox() */
void QSPage::setMediabox(QRect rc) {
 //Note that Rectangle uses double coordinates, while Qrect use int coordinates.
 //However, media box is often array of int, so this may not be a problem
 Rectangle r(rc.left(),rc.top(),rc.right(),rc.bottom());
 obj->setMediabox(r);
}

/**
 Return list of font id's and names,
 for each font id and name return two elements in output array (id followed by name)
 @return list of font id's and names
*/
QStringList QSPage::getFontIdsAndNames() {
 FontList fonts;
 //Format is like "pair<R13, Helvetica>"
 obj->getFontIdsAndNames(fonts);
 QStringList ret;
 FontList::iterator it;
 for( it=fonts.begin();it!=fonts.end();++it) { // for each font
  ret+=it->first;
  ret+=it->second;
 }
 return ret;
}

/**
 Add new Type 1 font to this page resource dictionary
 \see CPage::addSystemType1Font
 @param fontName name of font
*/
void QSPage::addSystemType1Font(const QString &fontName) {
 obj->addSystemType1Font(fontName);
}

/** get CPage held inside this class. Not exposed to scripting */
boost::shared_ptr<CPage> QSPage::get() const {
 return obj;
}

} // namespace gui

//todo: incomplete
