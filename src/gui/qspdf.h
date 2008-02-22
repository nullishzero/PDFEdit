/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006, 2007, 2008  PDFedit team: Michal Hocko,
 *                                              Miroslav Jahoda,
 *                                              Jozef Misutka,
 *                                              Martin Petricek
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
 Note to page manipulation - page numbers start from number 1 (not from zero as some programmers may expect),
 number of last page is equal to count of pages.
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
  Return true if saved successfully, false if failed to save because of any reason
 */
 bool save(bool newRevision=false);
 /*- Get document dictionary (object catalog) */
 QSDict* getDictionary();
 /*- Remove page with given number from document. */
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
 /*- Return page from document, given its page number. */
 QSPage* getPage(int position);
 /*- Return first page in document. */
 QSPage* getFirstPage();
 /*- Return last page in document. */
 QSPage* getLastPage();
 /*- Return next page in document, relative to specified page. */
 QSPage* getNextPage(QSPage* page);
 QSPage* getNextPage(QObject* page);
 /*- Return previous page in document, relative to specified page. */
 QSPage* getPrevPage(QSPage* page);
 QSPage* getPrevPage(QObject* page);
 /*- Return true, if there is next page in document for given page. */
 bool hasNextPage(QSPage* page);
 bool hasNextPage(QObject* page);
 /*- Return true, if there is previous page in document for given page. */
 bool hasPrevPage(QSPage* page);
 bool hasPrevPage(QObject* page);
 /*- Return number of available revisions in document */
 int getRevisionsCount();
 /*- Return number of currently active revisions */
 int getActualRevision();
 /* Check validity of specified reference. Return true the number and generation number is a valid reference, false otherwise */
 bool referenceValid(int valueNum,int valueGen);
private:
 /** Pointer to object held in class*/
 CPdf *obj;
 /** Destructive flag - if the PDF file may be closed with the unloadPdf() function */
 bool destructive;
};

} // namespace gui

#endif
