/** @file
 QObject wrapper around CPdf (PDF document)<br>
 Export some functions to scripting.<br>
 See doc/user/scripting.xml or kernel documentation for more informations about these functions
 @author Martin Petricek
*/

#include "qspdf.h"
#include <stdio.h>

namespace gui {

/** Construct wrapper with given PDF document */
QSPdf::QSPdf(CPdf *_pdf,Base *_base) : QSCObject ("Pdf",_base) {
 obj=_pdf;
}

/** destructor */
QSPdf::~QSPdf() {
}

/** Save copy under different name. Does not check if the file exists, overwrite anything without warning
 @return true if saved successfully, false if any error occured
*/
bool QSPdf::saveAs(QString name) {
 if (name.isNull()) return false;//No empty names!
 FILE *f=fopen(name,"wb");
 if (!f) return false; ///failed to open file
 try {
  obj->clone(f);
  fclose(f);
  return true;
 } catch (...) {
  fclose(f);
  return false;
 }
}

/** Call CPdf::getDictionary */
QSDict* QSPdf::getDictionary() {
 return new QSDict(obj->getDictionary(),base);
}

/** Call CPdf::insertPage(page,position) */
QSPage* QSPdf::insertPage(QSPage* page, int position) {
 if (!page) return NULL;//Something invalid passed
 return new QSPage(obj->insertPage(page->get(),position),base);
}

/** Call CPdf::getPagePosition(page) - QSA bugfix version */
QSPage* QSPdf::insertPage(QObject* page, int position) {
 return insertPage(dynamic_cast<QSPage*>(page),position);
}


/** Call CPdf::removePage(page,position) */
void QSPdf::removePage(int position) {
 obj->removePage(position);
}

/** Call CPdf::getPagePosition(page) */
int QSPdf::getPagePosition(QSPage *page) {
 if (!page) return -1;//Something invalid passed
 return obj->getPagePosition(page->get());
}

/** Call CPdf::getPagePosition(page) - QSA bugfix version */
int QSPdf::getPagePosition(QObject *page) {
 return getPagePosition(dynamic_cast<QSPage*>(page));
}

/** Call CPdf::getPageCount() */
int QSPdf::getPageCount() {
 return obj->getPageCount();
}

/** Call CPdf::getPage(position) */
QSPage* QSPdf::getPage(int position) {
 return new QSPage(obj->getPage(position),base);
}

/** Call CPdf::getFirstPage() */
QSPage* QSPdf::getFirstPage() {
 return new QSPage(obj->getFirstPage(),base);
}

/** Call CPdf::getNextPage(page) */
QSPage* QSPdf::getNextPage(QSPage* page) {
 return new QSPage(obj->getNextPage(page->get()),base);
}

/** Call CPdf::getPrevPage(page) */
QSPage* QSPdf::getPrevPage(QSPage* page) {
 return new QSPage(obj->getPrevPage(page->get()),base);
}

/** Call CPdf::hasNextPage(page) */
bool QSPdf::hasNextPage(QSPage* page) {
 return obj->hasNextPage(page->get());
}

/** Call CPdf::hasPrevPage(page) */
bool QSPdf::hasPrevPage(QSPage* page) {
 return obj->hasPrevPage(page->get());
}

/** Call CPdf::getNextPage(page) - qsa bugfix version */
QSPage* QSPdf::getNextPage(QObject* page) {
 return getNextPage(dynamic_cast<QSPage*>(page));
}

/** Call CPdf::getPrevPage(page) - qsa bugfix version */
QSPage* QSPdf::getPrevPage(QObject* page) {
 return getPrevPage(dynamic_cast<QSPage*>(page));
}

/** Call CPdf::hasNextPage(page) - qsa bugfix version */
bool QSPdf::hasNextPage(QObject* page) {
 return hasNextPage(dynamic_cast<QSPage*>(page));
}

/** Call CPdf::hasPrevPage(page) - qsa bugfix version */
bool QSPdf::hasPrevPage(QObject* page) {
 return hasPrevPage(dynamic_cast<QSPage*>(page));
}

/** Call CPdf::getRevisionsCount() */
int QSPdf::getRevisionsCount() {
 return obj->getRevisionsCount();
}

/** Call CPdf::getActualRevision() */
int QSPdf::getActualRevision() {
 return obj->getActualRevision();
}

/** Call CPdf::getLastPage() */
QSPage* QSPdf::getLastPage() {
 return new QSPage(obj->getLastPage(),base);
}

/** get CPdf held inside this class. Not exposed to scripting */
CPdf* QSPdf::get() const {
 return obj;
}

} // namespace gui 

//TODO: implement outlines and revisions, rest of functions ...
