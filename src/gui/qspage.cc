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
#include <qvaluelist.h>
#include "qscontentstream.h"
#include "qsannotation.h"
#include "qspdfoperatorstack.h"
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
QSPage::QSPage(boost::shared_ptr<CPage> _page,BaseCore *_base) : QSCObject ("Page",_base) {
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

/**
 Set transform matrix of this page
 \see CPage::setTransformMatrix
 @param tMatrix Array with 6 integers
*/
void QSPage::setTransformMatrix(QVariant tMatrix) {
 double tm[6]={0};
 QValueList<QVariant> list=tMatrix.toList();
 QValueList<QVariant>::Iterator it = list.begin();
 int i=0;
 while(it!=list.end()) {
  if (i>=6) break;//We filled all 6 values
  tm[i]=(*it).toDouble();
  ++it;
  ++i;
 }
 obj->setTransformMatrix(tm);
}

/** 
 Add content stream to page, created from new operator stack
 \see CPage::addContentStream
 @param opStack PDF Operator stack
*/
void QSPage::addContentStream(QSPdfOperatorStack* opStack) {
 assert(opStack);
 obj->addContentStream(opStack->get());
}

/** \copydoc addContentStream(QSPdfOperatorStack*) */
void QSPage::addContentStream(QObject* opStack) {
 QSPdfOperatorStack* in=dynamic_cast<QSPdfOperatorStack*>(opStack);
 if (!in) return;
 addContentStream(in);
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

//TODO: get list of annotations

/**
 Add copy of given annotation to this page
 @param an Annotation to add
*/
void QSPage::addAnnotation(QSAnnotation* an) {
 obj->addAnnotation(an->get());
}

/** \copydoc addAnnotation(QSAnnotation*) */
void QSPage::addAnnotation(QObject *an) {
 QSAnnotation* annot=dynamic_cast<QSAnnotation*>(an);
 if (!annot) return;
 addAnnotation(annot);
}

/**
 Return mediabox from Page
 \see CPage::getMediabox
*/
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

/**
 Set mediabox for this page
 \see CPage::getMediabox
 @param x1 X coordinate of upper left media box corner
 @param y1 Y coordinate of upper left media box corner
 @param x2 X coordinate of lower right media box corner
 @param y2 Y coordinate of lower right media box corner
*/
void QSPage::setMediabox(double x1,double y1,double x2,double y2) {
 Rectangle r(x1,y1,x2,y2);
 obj->setMediabox(r);
}

/**
 Set mediabox for this page
 \see CPage::getMediabox
 @param rc rectangle specifying media box
*/
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
