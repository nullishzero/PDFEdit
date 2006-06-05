#ifndef __QSPAGE_H__
#define __QSPAGE_H__

#include "qscobject.h"
#include <qobject.h>
#include <boost/shared_ptr.hpp>
#include <vector>
namespace pdfobjects {
 class CPage;
 class CContentStream;
}
class QString;
class QRect;
class QVariant;

namespace gui {

class QSContentStream;
class QSAnnotation;
class QSDict;
class Base;
class QSPdfOperatorStack;

using namespace pdfobjects;

/*= This type of object represents one page in document. */
/** QObject wrapper around CPage */
class QSPage : public QSCObject {
 Q_OBJECT
public:
 virtual ~QSPage();
 QSPage(const QSPage &source);
 QSPage(boost::shared_ptr<CPage> _page,Base *_base);
 boost::shared_ptr<CPage> get() const;
public slots:
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
 */
 void addContentStream(QSPdfOperatorStack* opStack);
 void addContentStream(QObject* opStack);
 /*-
  Return list of all font id's and base names from resource dictionary of a page.
  For each font id and name pair there are two consecutive elements in returned array,
  first one containing font id and second one containing the name,
  so number of elements in output array is always even.
 */
 QStringList getFontIdsAndNames();
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
  Returns number of content streams in this page
  Use data fetched by loadContentStreams method, if it wasn't called, it is called before returning the count
 */
 int getContentStreamCount();
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
 /*- Return media box of this page as array (x1,y1,x2,y2) */
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
