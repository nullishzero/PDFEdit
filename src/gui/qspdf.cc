/** @file
 QObject wrapper around CPdf
*/

#include "qspdf.h"

/** Construct wrapper with given PDF document */
QSPdf::QSPdf(CPdf *_pdf) : QSCObject () {
 obj=_pdf;
}

/** destructor */
QSPdf::~QSPdf() {
}

/** Call CPdf::save(NULL) */
int QSPdf::save() {
 return obj->save(NULL);
}

/** Call CPdf::save(name) */
int QSPdf::saveAs(QString name) {
 return obj->save(name);
}

/** Call CPdf::getDictionary */
QSDict* QSPdf::getDictionary() {
 return new QSDict(obj->getDictionary().get());
}

/** Call CPdf::insertPage(page,position) */
QSPage* QSPdf::insertPage(QSPage *page, int position) {
 return new QSPage(obj->insertPage(page->get(),position));
}
/** Call CPdf::removePage(page,position) */
int QSPdf::removePage(int position) {
 return obj->removePage(position);
}

/** Call CPdf::getPagePosition(page) */
int QSPdf::getPagePosition(QSPage *page) {
 return obj->getPagePosition(page->get());
}

/** Call CPdf::getPageCount() */
unsigned int QSPdf::getPageCount() {
 return obj->getPageCount();
}

/** Call CPdf::getPage(position) */
QSPage* QSPdf::getPage(int position) {
 return new QSPage(obj->getPage(position));
}

/** Call CPdf::getFirstPage() */
QSPage* QSPdf::getFirstPage() {
 return new QSPage(obj->getFirstPage());
}

/** Call CPdf::getNextPage(page) */
QSPage* QSPdf::getNextPage(QSPage *page) {
 return new QSPage(obj->getNextPage(page->get()));
}

/** Call CPdf::getPrevPage(page) */
QSPage* QSPdf::getPrevPage(QSPage *page) {
 return new QSPage(obj->getPrevPage(page->get()));
}

/** Call CPdf::getLastPage() */
QSPage* QSPdf::getLastPage() {
 return new QSPage(obj->getLastPage());
}

/** get CPdf held inside this class. Not exposed to scripting */
CPdf* QSPdf::get() {
 return obj;
}

//TODO: implement outlines and revisions, rest of functions ...
