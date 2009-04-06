/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006-2009  PDFedit team: Michal Hocko,
 *                                        Jozef Misutka,
 *                                        Martin Petricek
 *                   Former team members: Miroslav Jahoda
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (in doc/LICENSE.GPL); if not, write to the 
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 * MA  02111-1307  USA
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit
 */
/** @file
 QObject wrapper around CPage (Page in document)<br>
 Export some functions to scripting.<br>
 See doc/user/scripting.xml or kernel documentation for more informations about these functions
 @author Martin Petricek
*/

#include "qspage.h"
#include "qtcompat.h"
#include "qsdict.h"
#include QLIST
#include "qscontentstream.h"
#include "qsannotation.h"
#include "qspdfoperatorstack.h"
#include "util.h"
#include <kernel/cpage.h>

namespace gui {

using namespace std;
using namespace util;

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
 \see CPage::moveAbove
 @param cs Content stream to move one level up painting order
*/
void QSPage::moveAbove(QSContentStream* cs) {
 try {
  obj->moveAbove(cs->get());
 } catch (CObjInvalidOperation &e) {
  base->errorException("Page","moveAbove",tr("Invalid operation"));
 } catch (ReadOnlyDocumentException &e) {
  base->errorException("Page","moveAbove",QObject::tr("Document is read-only"));
 } catch (OutOfRange &e) {
  base->errorException("Page","moveAbove",tr("Out of range"));
 } catch(...) {
  base->errorException("Page","moveAbove",tr("Unknown exception"));
 }
}


/**
 \see CPage::moveBelow
 @param cs Content stream to move one level below in painting order
*/
void QSPage::moveBelow(QSContentStream* cs) {
 try {
  obj->moveBelow(cs->get());
 } catch (CObjInvalidOperation &e) {
  base->errorException("Page","moveBelow",tr("Invalid operation"));
 } catch (ReadOnlyDocumentException &e) {
  base->errorException("Page","moveBelow",QObject::tr("Document is read-only"));
 } catch (OutOfRange &e) {
  base->errorException("Page","moveBelow",tr("Out of range"));
 } catch(...) {
  base->errorException("Page","moveBelow",tr("Unknown exception"));
 }
}

/**
 \see CPage::moveAbove
 @param cs Content stream to move one level up painting order
*/
void QSPage::moveAbove(QObject* cs) {
 QSContentStream *qsc=qobject_cast<QSContentStream*>(cs,"moveAbove",1,"ContentStream");
 if (!qsc) return;//Something invalid passed
 moveAbove(qsc);
}

/**
 \see CPage::moveBelow
 @param cs Content stream to move one level up painting order
*/
void QSPage::moveBelow(QObject* cs) {
 QSContentStream *qsc=qobject_cast<QSContentStream*>(cs,"moveBelow",1,"ContentStream");
 if (!qsc) return;//Something invalid passed
 moveBelow(qsc);
}

/**
 \see CPage::moveAbove
 @param csi Index of content stream to move one level up painting order
*/
void QSPage::moveAbove(int csi) {
 try {
  obj->moveAbove(csi);
 } catch (CObjInvalidOperation &e) {
  base->errorException("Page","moveAbove",tr("Invalid operation"));
 } catch (ReadOnlyDocumentException &e) {
  base->errorException("Page","moveAbove",QObject::tr("Document is read-only"));
 } catch (OutOfRange &e) {
  base->errorException("Page","moveAbove",tr("Out of range"));
 } catch(...) {
  base->errorException("Page","moveAbove",tr("Unknown exception"));
 }
}

/**
 \see CPage::moveBelow
 @param csi Index of content stream to move one level below in painting order
*/
void QSPage::moveBelow(int csi) {
 try {
  obj->moveBelow(csi);
 } catch (CObjInvalidOperation &e) {
  base->errorException("Page","moveBelow",tr("Invalid operation"));
 } catch (ReadOnlyDocumentException &e) {
  base->errorException("Page","moveBelow",QObject::tr("Document is read-only"));
 } catch (OutOfRange &e) {
  base->errorException("Page","moveBelow",tr("Out of range"));
 } catch(...) {
  base->errorException("Page","moveBelow",tr("Unknown exception"));
 }
}

/**
 Set transform matrix of this page
 \see CPage::setTransformMatrix
 @param tMatrix Array with 6 integers
*/
void QSPage::setTransformMatrix(QVariant tMatrix) {
 double tm[6]={0};
 Q_List<QVariant> list=tMatrix.toList();
 Q_List<QVariant>::Iterator it = list.begin();
 int i=0;
 while(it!=list.end()) {
  if (i>=6) break;//We filled all 6 values
  tm[i]=(*it).toDouble();
  ++it;
  ++i;
 }
 try {
  obj->setTransformMatrix(tm);
 } catch (ReadOnlyDocumentException &e) {
  base->errorException("Page","setTransformMatrix",QObject::tr("Document is read-only"));
 }
}

/**
 Add content stream to page, created from new operator stack
 Content stream is prepended before other content streams
 \see CPage::addContentStreamToFront
 @param opStack PDF Operator stack
*/
void QSPage::prependContentStream(QSPdfOperatorStack* opStack) {
 assert(opStack);
 try {
  obj->addContentStreamToFront(opStack->get());
 } catch (ReadOnlyDocumentException &e) {
  base->errorException("Page","prependContentStream",QObject::tr("Document is read-only"));
 }
}

/**
 Add content stream to page, created from new operator stack
 Content stream in appended after other content streams
 \see CPage::addContentStreamToBack
 @param opStack PDF Operator stack
*/
void QSPage::appendContentStream(QSPdfOperatorStack* opStack) {
 assert(opStack);
 try {
  obj->addContentStreamToBack(opStack->get());
 } catch (ReadOnlyDocumentException &e) {
  base->errorException("Page","appendContentStream",QObject::tr("Document is read-only"));
 }
}

/** \copydoc appendContentStream(QSPdfOperatorStack*) */
void QSPage::prependContentStream(QObject* opStack) {
 QSPdfOperatorStack* in=dynamic_cast<QSPdfOperatorStack*>(opStack);
 if (!in) return;
 appendContentStream(in);
}

/** \copydoc prependContentStream(QSPdfOperatorStack*) */
void QSPage::appendContentStream(QObject* opStack) {
 QSPdfOperatorStack* in=dynamic_cast<QSPdfOperatorStack*>(opStack);
 if (!in) return;
 prependContentStream(in);
}

/** Call CPage::getText(ret); return ret */
QString QSPage::getText() {
 std::string text;
 std::string encoding="UTF-8";
 obj->getText(text,&encoding);
 return QString::fromUtf8(text.c_str());
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
 Remove stream with given number from page.
 @param streamNumber number of stream to remove
*/
void QSPage::removeContentStream(int streamNumber) {
 if (numStreams<0) loadContentStreams();
 if (streamNumber<0) return;		//Stream number outside range
 if (streamNumber>=numStreams) return;	//Stream number outside range
 obj->removeContentStream (streamNumber);
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

/**
 Return change with given number as content stream
 @param changeNumber number of change to get
 @return ContentStream with given change, or NULL if number is outside range
*/
QSContentStream* QSPage::getChange(int changeNumber) {
 if (changeNumber<0) return NULL;		//number outside range
 try {
  boost::shared_ptr<CContentStream> c=obj->getChange((size_t)changeNumber);
  return new QSContentStream(c,base);
 } catch(...) {
  //Some error occured
  return NULL;
 }
}

/**
 Return number of changes
 @return number of changes in this Page
*/
int QSPage::getChangeCount() {
 return (int)obj->getChangeCount();
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
 try {
  obj->addAnnotation(an->get());
 } catch (ReadOnlyDocumentException &e) {
  base->errorException("Page","addAnnotation",QObject::tr("Document is read-only"));
 }
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
 libs::Rectangle r=obj->getMediabox();
 Q_List<QVariant> rect;
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
 libs::Rectangle r(x1,y1,x2,y2);
 try {
  obj->setMediabox(r);
 } catch (ReadOnlyDocumentException &e) {
  base->errorException("Page","setMediaBox",QObject::tr("Document is read-only"));
 }
}

/**
 Set mediabox for this page
 \see CPage::getMediabox
 @param rc rectangle specifying media box
*/
void QSPage::setMediabox(QRect rc) {
 //Note that Rectangle uses double coordinates, while Qrect use int coordinates.
 //However, media box is often array of int, so this may not be a problem
 libs::Rectangle r(rc.left(),rc.top(),rc.right(),rc.bottom());
 try {
  obj->setMediabox(r);
 } catch (ReadOnlyDocumentException &e) {
  base->errorException("Page","setMediaBox",QObject::tr("Document is read-only"));
 }
}

/**
 Return list of font id's and names,
 for each font id and name return two elements in output array (id followed by name)
 @param onlyNames if true, id's are ommited and only list of names is returned instead.
 @return list of font id's and names
*/
QStringList QSPage::getFontIdsAndNames(bool onlyNames/*=false*/) {
 CPage::FontList fonts;
 //Format is like "pair<R13, Helvetica>"
 obj->getFontIdsAndNames(fonts);
 QStringList ret;
 CPage::FontList::iterator it;
 for( it=fonts.begin();it!=fonts.end();++it) { // for each font
  if (!onlyNames) ret+=convertToUnicode(it->first,PDF);
  ret+=convertToUnicode(it->second,PDF);
 }
 return ret;
}

/**
 For given font name (case sensitive) return it's ID if present on page.
 If the font is not present, returns NULL
 @param fontName name of desired font
 @return ID of font with given name, or NULL if not found in the page
*/
QString QSPage::getFontId(const QString &fontName) {
 CPage::FontList fonts;
 //Format is like "pair<R13, Helvetica>"
 obj->getFontIdsAndNames(fonts);
 QStringList ret;
 CPage::FontList::iterator it;
 string fontNameString=convertFromUnicode(fontName,PDF);
 for( it=fonts.begin();it!=fonts.end();++it) { // for each font
  if (it->second==fontNameString) return convertToUnicode(it->first,PDF);
 }
 //Not found
 return QString::null;
}

/**
 Add new Type 1 font to this page resource dictionary
 \see CPage::addSystemType1Font
 @param fontName name of font
*/
void QSPage::addSystemType1Font(const QString &fontName) {
 try {
  obj->addSystemType1Font(convertFromUnicode(fontName,PDF));
 } catch (ReadOnlyDocumentException &e) {
  base->errorException("Page","addSystemType1Font",QObject::tr("Document is read-only"));
 }
}

/** get CPage held inside this class. Not exposed to scripting */
boost::shared_ptr<CPage> QSPage::get() const {
 return obj;
}

} // namespace gui
