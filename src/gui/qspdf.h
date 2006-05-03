#ifndef __QSPDF_H__
#define __QSPDF_H__

#include <qstring.h>
#include <qobject.h>
#include <cobject.h>
#include <cpdf.h>
#include "qspage.h"
#include "qsdict.h"
#include "qscobject.h"

namespace gui {

using namespace pdfobjects;

/*= This type of object represents entire document. */
class QSPdf : public QSCObject {
 Q_OBJECT
public:
 QSPdf(CPdf *_pdf);
 virtual ~QSPdf();
 CPdf* get();
public slots:
 /*-
  Save this document under different name.
  Does not modify name of file in editor.
  Does not check for file existence - it will overwrite the file without warning if it already exists
  Return true if document was saved, false if it failed to save for any reason
 */
 bool saveAs(QString name);
 /*- Get document dictionary */
 QSDict* getDictionary();
 /*- Remove page with given number from document */
 void removePage(int position);
 /*- Get position of given page in document */
 int getPagePosition(QSPage *page);
 /*- Get number of pages in document */
 unsigned int getPageCount();
 /*- Insert given page in document, at given position. Return inserted page. */
 QSPage* insertPage(QSPage* page, int position);
 /*- Get Page, given its page number. */
 QSPage* getPage(int position);
 /*- Get first page in document. */
 QSPage* getFirstPage();
 /*- Get last page in document. */
 QSPage* getLastPage();
 /*- Get next page in document, relative to given page. */
 QSPage* getNextPage(QSPage* page);
 /*- Get previous page in document, relative to given page. */
 QSPage* getPrevPage(QSPage* page);
 /*- Return true, if there is next page in document for given page. */
 bool hasNextPage(QSPage* page);
 /*- Return true, if there is previous page in document for given page. */
 bool hasPrevPage(QSPage* page);
 /*- Return number of available revisions */
 size_t getRevisionsCount();
private:
 /** Object held in class*/
 CPdf *obj;
};

} // namespace gui

#endif
