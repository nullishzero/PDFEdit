/*                                                                              
 * PDFedit - free program for PDF document manipulation.                        
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko, 
 *                                              Miroslav Jahoda,       
 *                                              Jozef Misutka, 
 *                                              Martin Petricek                                             
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit                                                                      
 */ 
#ifndef __QSPDF_H__
#define __QSPDF_H__

#include <qobject.h>
#include "qscobject.h"
class QString;
namespace pdfobjects {
 class CPdf;
}

namespace gui {

class QSPage;
class QSDict;

using namespace pdfobjects;

/*=
 This type of object represents entire document.
 Note to page manipulation - page number start from number 1,
 number of last page is equal to count of pages
 */
/** \brief QObject wrapper around CPdf */
class QSPdf : public QSCObject {
 Q_OBJECT
public:
 QSPdf(CPdf *_pdf,BaseCore *_base,bool _destructive=false);
 virtual ~QSPdf();
 CPdf* get() const;
 void set(CPdf* pdf);
public slots:
 /*- Return name of filter used to encrypt the document, or NULL if document is not encrypted */
 QString encryption();
 /*- Return true, if document is linearized PDF, false otherwise */
 bool isLinearized();
 /*-
  Check for PDF validity - return true, if this object is valid PDF document,
  false if the PDF document was closed or not yet opened
  (so the object is invalid)
 */
 bool isValid();
 /*-
  Close the document.
  If the document was opened directly in editor window (with openFile function), it won't be closed,
  you must use closeFile() function for this.
  However, all documents loaded with loadPdf function should be closed using this function.
 */
 void unloadPdf();
 /*-
  Save this document under different name.
  Does not modify name of file in editor, original file is still edited, not this one.
  Does not check for file existence - it will overwrite the file without warning if it already exists
  Return true if document was saved, false if it failed to save for any reason
 */
 bool saveAs(QString name);
 /*-
  Save document to disk under original name
  If newRevision is true, create new revision while saving.
  Return true if saved succesfully, false if failed to save because of any reason
 */
 bool save(bool newRevision=false);
 /*- Get document dictionary */
 QSDict* getDictionary();
 /*- Remove page with given number from document */
 void removePage(int position);
 /*-
   Get position of given page in document or -1 in case of error
  (page not in document, invalid page, etc...)
 */
 int getPagePosition(QSPage *page);
 int getPagePosition(QObject *page);
 /*- Get number of pages in document */
 int getPageCount();
 /*- Insert given page in document, at given position. Return inserted page. */
 QSPage* insertPage(QSPage* page, int position);
 QSPage* insertPage(QObject* page, int position);
 /*- Get Page, given its page number. */
 QSPage* getPage(int position);
 /*- Get first page in document. */
 QSPage* getFirstPage();
 /*- Get last page in document. */
 QSPage* getLastPage();
 /*- Get next page in document, relative to given page. */
 QSPage* getNextPage(QSPage* page);
 QSPage* getNextPage(QObject* page);
 /*- Get previous page in document, relative to given page. */
 QSPage* getPrevPage(QSPage* page);
 QSPage* getPrevPage(QObject* page);
 /*- Return true, if there is next page in document for given page. */
 bool hasNextPage(QSPage* page);
 bool hasNextPage(QObject* page);
 /*- Return true, if there is previous page in document for given page. */
 bool hasPrevPage(QSPage* page);
 bool hasPrevPage(QObject* page);
 /*- Return number of available revisions */
 int getRevisionsCount();
 /*- Return number of currently active revisions */
 int getActualRevision();
 /* Check validity of specified reference. Return true if it is valid */
 bool referenceValid(int valueNum,int valueGen);
private:
 /** Object held in class*/
 CPdf *obj;
 /** Destructive flag - if the PDF file may be closed with the unloadPdf() function */
 bool destructive;
};

} // namespace gui

#endif
