#ifndef __QSPAGE_H__
#define __QSPAGE_H__

#include <qobject.h>
#include <cpage.h>
#include "qscobject.h"
#include <qvariant.h>
class QString;
class QRect;

namespace gui {

class QSContentStream;
class QSDict;
class Base;

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
