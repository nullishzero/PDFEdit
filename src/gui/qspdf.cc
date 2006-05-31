/** @file
 QObject wrapper around CPdf (PDF document)<br>
 Export some functions to scripting.<br>
 See doc/user/scripting.xml or kernel documentation for more informations about these functions
 @author Martin Petricek
*/

#include "qspdf.h"
#include "pdfutil.h"
#include <stdio.h>

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

/** Save copy under different name. Does not check if the file exists, overwrite anything without warning
 @return true if saved successfully, false if any error occured
*/
bool QSPdf::saveAs(QString name) {
 if (!obj) return false;
 if (name.isNull()) return false;//No empty names!
 return util::saveCopy(obj,name);
}

/** Call CPdf::getDictionary */
QSDict* QSPdf::getDictionary() {
 if (!obj) return NULL;
 return new QSDict(obj->getDictionary(),base);
}

/** Call CPdf::isLinearized() */
bool QSPdf::isLinearized() {
 return obj->isLinearized();
}

/** Call CPdf::insertPage(page,position) */
QSPage* QSPdf::insertPage(QSPage* page, int position) {
 if (!obj) return NULL;
 if (!page) return NULL;//Something invalid passed
 return new QSPage(obj->insertPage(page->get(),position),base);
}

/** Call CPdf::getPagePosition(page) - QSA bugfix version */
QSPage* QSPdf::insertPage(QObject* page, int position) {
 if (!obj) return NULL;
 return insertPage(dynamic_cast<QSPage*>(page),position);
}

/** Call CPdf::removePage(page,position) */
void QSPdf::removePage(int position) {
 if (!obj) return;
 obj->removePage(position);
}

/** Call CPdf::getPagePosition(page) */
int QSPdf::getPagePosition(QSPage *page) {
 if (!obj) return -1;
 if (!page) return -1;//Something invalid passed
 return obj->getPagePosition(page->get());
}

/** Call CPdf::getPagePosition(page) - QSA bugfix version */
int QSPdf::getPagePosition(QObject *page) {
 if (!obj) return -1;
 return getPagePosition(dynamic_cast<QSPage*>(page));
}

/** Call CPdf::getPageCount() */
int QSPdf::getPageCount() {
 if (!obj) return -1;
 return obj->getPageCount();
}

/** Call CPdf::getPage(position) */
QSPage* QSPdf::getPage(int position) {
 return new QSPage(obj->getPage(position),base);
}

/** Call CPdf::getFirstPage() */
QSPage* QSPdf::getFirstPage() {
 if (!obj) return NULL;
 return new QSPage(obj->getFirstPage(),base);
}

/** Call CPdf::getNextPage(page) */
QSPage* QSPdf::getNextPage(QSPage* page) {
 if (!obj) return NULL;
 return new QSPage(obj->getNextPage(page->get()),base);
}

/** Call CPdf::getPrevPage(page) */
QSPage* QSPdf::getPrevPage(QSPage* page) {
 if (!obj) return NULL;
 return new QSPage(obj->getPrevPage(page->get()),base);
}

/** Call CPdf::hasNextPage(page) */
bool QSPdf::hasNextPage(QSPage* page) {
 if (!obj) return false;
 return obj->hasNextPage(page->get());
}

/** Call CPdf::hasPrevPage(page) */
bool QSPdf::hasPrevPage(QSPage* page) {
 if (!obj) return false;
 return obj->hasPrevPage(page->get());
}

/** Call CPdf::getNextPage(page) - qsa bugfix version */
QSPage* QSPdf::getNextPage(QObject* page) {
 if (!obj) return NULL;
 return getNextPage(dynamic_cast<QSPage*>(page));
}

/** Call CPdf::getPrevPage(page) - qsa bugfix version */
QSPage* QSPdf::getPrevPage(QObject* page) {
 if (!obj) return NULL;
 return getPrevPage(dynamic_cast<QSPage*>(page));
}

/** Call CPdf::hasNextPage(page) - qsa bugfix version */
bool QSPdf::hasNextPage(QObject* page) {
 if (!obj) return false;
 return hasNextPage(dynamic_cast<QSPage*>(page));
}

/** Call CPdf::hasPrevPage(page) - qsa bugfix version */
bool QSPdf::hasPrevPage(QObject* page) {
 if (!obj) return false;
 return hasPrevPage(dynamic_cast<QSPage*>(page));
}

/** Call CPdf::getRevisionsCount() */
int QSPdf::getRevisionsCount() {
 if (!obj) return 0;
 return obj->getRevisionsCount();
}

/** Call CPdf::getActualRevision() */
int QSPdf::getActualRevision() {
 if (!obj) return 0;
 return obj->getActualRevision();
}

/** Call CPdf::getLastPage() */
QSPage* QSPdf::getLastPage() {
 if (!obj) return NULL;
 return new QSPage(obj->getLastPage(),base);
}

/** get CPdf held inside this class. Not exposed to scripting */
CPdf* QSPdf::get() const {
 return obj;
}

} // namespace gui 
