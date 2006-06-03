/** @file
 QObject wrapper around CPdf (PDF document)<br>
 Export some functions to scripting.<br>
 See doc/user/scripting.xml or kernel documentation for more informations about these functions
 @author Martin Petricek
*/

#include <qstring.h>
#include "qspdf.h"
#include "pdfutil.h"
#include <stdio.h>
#include "qspage.h"
#include "qsdict.h"
#include <cobject.h>
#include <cpdf.h>

namespace gui {

/**
 Construct wrapper with given PDF document
 @param _pdf Pdf document
 @param _base Scripting base
 @param _destructive If true, document will be closed (without saving) when this object is deleted
*/
QSPdf::QSPdf(CPdf *_pdf,Base *_base,bool _destructive/*=false*/) : QSCObject ("Pdf",_base) {
 destructive=_destructive;
 obj=_pdf;
}

/** destructor */
QSPdf::~QSPdf() {
 //Unload the pdf if this is 'destructive' PDF
 unloadPdf();
}

/** 
 Close the document, if it was opened with 'destructive' flag set (via loadPdf).
 Otherwise it does nothing.
 The document must not be used in any way after it is closed
*/
void QSPdf::unloadPdf() {
 if (obj && destructive) {
  obj->close();
  obj=NULL;
 }
}

/**
 Check validity of specified reference
 @param valueNum Object number
 @param valueGen Generation number
 @return True if valid and is contained in this document
*/
bool QSPdf::referenceValid(int valueNum,int valueGen) {
 IndiRef ref;
 ref.num=valueNum;
 ref.gen=valueGen;
 //Check reference validity
 return util::isRefValid(obj,ref);
}

/**
 Save copy under different name. Does not check if the file exists, overwrite anything without warning
 @param name Name of file
 @return true if saved successfully, false if any error occured
*/
bool QSPdf::saveAs(QString name) {
 if (!obj) return false;
 if (name.isNull()) return false;//No empty names!
 return util::saveCopy(obj,name);
}

/**
 \see CPdf::getDictionary
 @return document catalog
 */
QSDict* QSPdf::getDictionary() {
 if (!obj) return NULL;
 return new QSDict(obj->getDictionary(),base);
}

/**
 \see CPdf::isLinearized()
 @return True if document is linearized
*/
bool QSPdf::isLinearized() {
 return obj->isLinearized();
}

/**
 Insert page in document at specified position
 \see CPdf::insertPage
 @param page page to insert
 @param position Positin in which to insert
 */
QSPage* QSPdf::insertPage(QSPage* page, int position) {
 if (!obj) return NULL;
 if (!page) return NULL;//Something invalid passed
 return new QSPage(obj->insertPage(page->get(),position),base);
}

/**
 QSA bugfix version
 \copydoc insertPage(QSPage*,int)
*/
QSPage* QSPdf::insertPage(QObject* page, int position) {
 if (!obj) return NULL;
 return insertPage(dynamic_cast<QSPage*>(page),position);
}

/**
 Remove page at given position
 (numbering starts from 1)
 \see CPdf::removePage()
 @param position Page number to remove
*/
void QSPdf::removePage(int position) {
 if (!obj) return;
 obj->removePage(position);
}

/**
 Return positoon of given page
 \see CPdf::getPagePosition
 @param page page to examine
 @return page position
*/
int QSPdf::getPagePosition(QSPage *page) {
 if (!obj) return -1;
 if (!page) return -1;//Something invalid passed
 return obj->getPagePosition(page->get());
}

/**
 \copydoc getPagePosition(QSPage *)
 QSA bugfix version
*/
int QSPdf::getPagePosition(QObject *page) {
 if (!obj) return -1;
 return getPagePosition(dynamic_cast<QSPage*>(page));
}

/**
 Return number of pages in document
 \see CPdf::getPageCount
 @return count of pages
*/
int QSPdf::getPageCount() {
 if (!obj) return -1;
 return obj->getPageCount();
}

/**
 Return page at given position
 (numbering starts from 1)
 \see CPdf::getPage()
 @param position Page number
 @return page at positoin
*/
QSPage* QSPdf::getPage(int position) {
 return new QSPage(obj->getPage(position),base);
}

/**
 Get first page
 \see CPdf::getFirstPage 
 @return first page in document
*/
QSPage* QSPdf::getFirstPage() {
 if (!obj) return NULL;
 return new QSPage(obj->getFirstPage(),base);
}

/**
 Return page next to the one specified
 \see CPdf::getNextPage
 @param page Page to examine
 @return next page
*/
QSPage* QSPdf::getNextPage(QSPage* page) {
 if (!obj) return NULL;
 return new QSPage(obj->getNextPage(page->get()),base);
}

/**
 Return page previous to the one specified
 \see CPdf::getPrevPage
 @param page Page to examine
 @return previous page
*/
QSPage* QSPdf::getPrevPage(QSPage* page) {
 if (!obj) return NULL;
 return new QSPage(obj->getPrevPage(page->get()),base);
}

/**
 Return true if next page in relation to given page exists
 \see CPdf::hasNextPage
 @param page Page to examine
 @return presence of next page
*/
bool QSPdf::hasNextPage(QSPage* page) {
 if (!obj) return false;
 return obj->hasNextPage(page->get());
}

/**
 Return true if previous page in relation to given page exists
 \see CPdf::hasPrevPage
 @param page Page to examine
 @return presence of previous page
*/
bool QSPdf::hasPrevPage(QSPage* page) {
 if (!obj) return false;
 return obj->hasPrevPage(page->get());
}

/**
 \copydoc getNextPage(QSPage*)
 qsa bugfix version
*/
QSPage* QSPdf::getNextPage(QObject* page) {
 if (!obj) return NULL;
 return getNextPage(dynamic_cast<QSPage*>(page));
}

/**
 \copydoc getPrevPage(QSPage*)
 qsa bugfix version
*/
QSPage* QSPdf::getPrevPage(QObject* page) {
 if (!obj) return NULL;
 return getPrevPage(dynamic_cast<QSPage*>(page));
}

/**
 \copydoc hasNextPage(QSPage*)
 qsa bugfix version
*/
bool QSPdf::hasNextPage(QObject* page) {
 if (!obj) return false;
 return hasNextPage(dynamic_cast<QSPage*>(page));
}

/**
 \copydoc hasPrevPage(QSPage*)
 qsa bugfix version
*/
bool QSPdf::hasPrevPage(QObject* page) {
 if (!obj) return false;
 return hasPrevPage(dynamic_cast<QSPage*>(page));
}

/**
 Get count of revisions
 \see CPdf::getRevisionsCount
 @return count of revisions
*/
int QSPdf::getRevisionsCount() {
 if (!obj) return 0;
 return obj->getRevisionsCount();
}

/**
 Get actual revision number
 \see CPdf::getActualRevision
 @return actual revison
*/
int QSPdf::getActualRevision() {
 if (!obj) return 0;
 return obj->getActualRevision();
}

/**
 Get last page in document
 \see CPdf::getLastPage 
 @return last page
*/
QSPage* QSPdf::getLastPage() {
 if (!obj) return NULL;
 return new QSPage(obj->getLastPage(),base);
}

/**
 Get CPdf held inside this class. Not exposed to scripting 
 @return CPdf object
*/
CPdf* QSPdf::get() const {
 return obj;
}

/**
 Return name of filter used to encrypt the document, or NULL if document is not encrypted
 @return Encryption filter
 */
QString QSPdf::encryption() {
 std::string filter;
 if (!pdfobjects::utils::isEncrypted(*obj,&filter)) return QString::null;
 return filter;
}

} // namespace gui 
