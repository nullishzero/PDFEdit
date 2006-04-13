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

class QSPdf : public QSCObject {
 Q_OBJECT
public:
 QSPdf(CPdf *_pdf);
 virtual ~QSPdf();
 CPdf* get();
public slots:
 void saveAs(QString name);
 void removePage(int position);
 int getPagePosition(QSPage *page);
 unsigned int getPageCount();
 QSDict* getDictionary();
 QSPage* insertPage(QSPage* page, int position);
 QSPage* getPage(int position);
 QSPage* getFirstPage();
 QSPage* getNextPage(QSPage* page);
 QSPage* getPrevPage(QSPage* page);
 QSPage* getLastPage();
private:
 /** Object held in class*/
 CPdf *obj;
};

} // namespace gui

#endif
