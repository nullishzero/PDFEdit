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
#ifndef __QSPAGE_H__
#define __QSPAGE_H__

#include "qscobject.h"
#include <boost/shared_ptr.hpp>
#include <qobject.h>
#include <qrect.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qvariant.h>
#include <vector>
class QRect;
namespace pdfobjects {
 class CPage;
 class CContentStream;
}

namespace gui {

class QSAnnotation;
class QSContentStream;
class QSDict;
class QSPdfOperatorStack;

using namespace pdfobjects;

/*= This type of object represents one page in document. */
/** \brief QObject wrapper around CPage */
class QSPage : public QSCObject {
 Q_OBJECT
public:
 virtual ~QSPage();
 QSPage(const QSPage &source);
 QSPage(boost::shared_ptr<CPage> _page,BaseCore *_base);
 boost::shared_ptr<CPage> get() const;
public slots:
 /*- Move specified content stream in this page one level up in painting order. */
 void moveAbove(QSContentStream* cs);
 void moveAbove(QObject* cs);
 /*- Move specified content stream in this page one level down in painting order. */
 void moveBelow(QSContentStream* cs);
 void moveBelow(QObject* cs);
 /*- Move content stream with specified index in this page one level up in painting order. */
 void moveAbove(int csi);
 /*- Move content stream with specified index in this page one level down in painting order. */
 void moveBelow(int csi);
 /*- Add copy of given annotation to this page */
 void addAnnotation(QSAnnotation* an);
 void addAnnotation(QObject *an);
 /*-
  Set transform matrix of this page.
  Expects array with 6 real numbers as the matrix.
 */
 void setTransformMatrix(QVariant tMatrix);
 /*-
  Add new content stream to page, created from provided stack of PDF operators.
  They must form a valid content stream
  Content stream is prepended before other content streams
 */
 void prependContentStream(QSPdfOperatorStack* opStack);
 void prependContentStream(QObject* opStack);
 /*-
  Add new content stream to page, created from provided stack of PDF operators.
  They must form a valid content stream
  Content stream in appended after other content streams
 */
 void appendContentStream(QSPdfOperatorStack* opStack);
 void appendContentStream(QObject* opStack);
 /*-
  For given font name (case sensitive) return it's ID if present on page.
  If the font is not present, returns NULL
 */
 QString getFontId(const QString &fontName);
 /*-
  Return list of all font id's and base names from resource dictionary of a page.
  For each font id and name pair there are two consecutive elements in returned array,
  first one containing font id and second one containing the name,
  so number of elements in output array is always even.
  If parametr onlyNames is true, id's are ommited and only list of names is returned instead.
 */
 QStringList getFontIdsAndNames(bool onlyNames=false);
 /*-
  Add new Type 1 font to this page resource dictionary with specified font name
  It is supposed that you insert standard system font name that should be avaiable to all PDF viewers.
 */
 void addSystemType1Font(const QString &fontName);
 /*-
  Returns content stream with given number from this page
  Use data fetched by loadContentStreams method, if it wasn't called, it is called before returning the stream
 */
 QSContentStream* getContentStream(int streamNumber);
 /*-
  Removes content stream with given number from page.
 */
 void removeContentStream(int streamNumber);
 /*-
  Returns number of content streams in this page
  Use data fetched by loadContentStreams method, if it wasn't called, it is called before returning the count
 */
 int getContentStreamCount();
 /*-
  Return change with given number as content stream
 */
 QSContentStream* getChange(int changeNumber);
 /*-
  Return number of changes
 */
 int getChangeCount();
 /*-
  Get all content streams from page and store them.
  Get the streams with getContentStreamCount and getContentStream functions.
  Usually it is not necessary to call this method, as these funtions will call
  it automatically on first need, but you may call it explicitly to reload the
  streams stored in this object from the page
 */
 void loadContentStreams();
 /*- Returns page dictionary */
 QSDict* getDictionary();
 /*- Return text representation of this page */
 QString getText();
 /*-
  Return media box of this page as array (x1,y1,x2,y2).
  The mediabox is a rectangle from (x1,y1) to (x2,y2)
 */
 QVariant mediabox();/*Variant=double[]*/
 /*- Set media box of this page to given rectangle - from (x1,y1) to (x2,y2) */
 void setMediabox(double x1,double y1,double x2,double y2);
 /*- Set media box of this page to given rectangle */
 void setMediabox(QRect rc);
private:
 /** Object held in class*/
 boost::shared_ptr<CPage> obj;
 /** Vector with content streams */
 std::vector<boost::shared_ptr<CContentStream> > streams;
 /** Number of content streams in vector. -1 mean no streams yet parsed */
 int numStreams;
};

} // namespace gui

#endif
