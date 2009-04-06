/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006-2009  PDFedit team: Michal Hocko,
 *                                        Jozef Misutka,
 *                                        Martin Petricek
 *                   Former team members: Miroslav Jahoda
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
#include <kernel/cobject.h>
#include <kernel/cpdf.h>
#include "util.h"

namespace gui {

/**
 Construct wrapper with given PDF document
 @param _pdf Pdf document
 @param _base Scripting base
 @param _destructive If true, document will be closed (without saving) when this object is deleted
*/
QSPdf::QSPdf(boost::shared_ptr<CPdf>_pdf,BaseCore *_base,bool _destructive/*=false*/) : QSCObject ("Pdf",_base) {
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
  obj.reset();
 }
}

/**
 Sets PDF password to use for decryption. Needed for opening password-protected documents
 @param pass password to set
 @return true if the password was successfully set, false in case of error (bad password ...)
*/
bool QSPdf::setPassword(const QString &pass) {
 return util::setPdfPassword(obj,pass);
}

/**
 Return true if document is protected and opening the document reqires credentials
 (usually a password), false otherwise.
*/
bool QSPdf::needsCredentials() {
 return obj->needsCredentials();
}

/**
 Check validity of specified reference
 @param valueNum Object number
 @param valueGen Generation number
 @return True if valid and is contained in this document
*/
bool QSPdf::referenceValid(int valueNum,int valueGen) {
 if (nullPtr(obj,"referenceValid")) return false;
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
 if (nullPtr(obj,"saveAs")) return false;
 if (name.isNull()) return false;//No empty names!
 QString errorStr;
 bool resultOk=util::saveCopy(obj,name,&errorStr);
 if (!resultOk) base->errorException("Pdf","saveAs",tr("Unable to save:\n%1").arg(errorStr));
 return resultOk;
}

/**
 Save document to disk under original name
 @param newRevision If true, create new revision while saving
 @return true if saved succesfully, false if failed to save because of any reason
*/
bool QSPdf::save(bool newRevision/*=false*/) {
 try {
  //Exception can occur while saving, for example if document is read-only
  obj->save(newRevision);
  return true;
 } catch (ReadOnlyDocumentException &e) {
  base->errorException("Pdf","save",tr("Document is in read-only mode"));
  return false;
 } catch (...) {
  base->errorException("Pdf","save",tr("Unknown error occured while saving document"));
  return false;
 }
}

/**
 \see CPdf::getDictionary
 @return document catalog
 */
QSDict* QSPdf::getDictionary() {
 if (nullPtr(obj,"getDictionary")) return NULL;
 return new QSDict(obj->getDictionary(),base);
}

/**
 \see CPdf::isLinearized()
 @return True if document is linearized
*/
bool QSPdf::isLinearized() {
 if (nullPtr(obj,"isLinearized")) return false;
 return obj->isLinearized();
}

/**
 Check for PDF validity - return true, if this object is valid PDF wrapper,
 false if the pdf was closed or not yet opened
 @return True if document is valid
*/
bool QSPdf::isValid() {
 if (obj==NULL) return false;
 return true;
}

/**
 Insert page in document at specified position
 \see CPdf::insertPage
 @param page page to insert
 @param position Position in which to insert
 */
QSPage* QSPdf::insertPage(QSPage* page, int position) {
 if (nullPtr(obj,"insertPage")) return NULL;
 try {
  return new QSPage(obj->insertPage(page->get(),position),base);
 } catch (ReadOnlyDocumentException &e) {
  base->errorException("Pdf","insertPage",QObject::tr("Document is read-only"));
  return NULL;
 }
}

/**
 QSA bugfix version
 \copydoc insertPage(QSPage*,int)
*/
QSPage* QSPdf::insertPage(QObject* page, int position) {
 QSPage *qpage=qobject_cast<QSPage*>(page,"insertPage",1,"Page");
 if (!qpage) return NULL;//Something invalid passed
 try {
  return insertPage(qpage,position);
 } catch (ReadOnlyDocumentException &e) {
  base->errorException("Pdf","insertPage",QObject::tr("Document is read-only"));
  return NULL;
 }
}

/**
 Remove page at given position
 (numbering starts from 1)
 \see CPdf::removePage()
 @param position Page number to remove
*/
void QSPdf::removePage(int position) {
 if (nullPtr(obj,"removePage")) return;
 try {
  obj->removePage(position);
 } catch (ReadOnlyDocumentException &e) {
  base->errorException("Pdf","removePage",QObject::tr("Document is read-only"));
 } catch (PageNotFoundException &e) {
  base->errorException("Pdf","removePage",tr("Page not found"));
 }
}

/**
 Return position of given page
 \see CPdf::getPagePosition
 @param page page to examine
 @return page position or -1 in case of error (page not in document, invalid page, etc...)
*/
int QSPdf::getPagePosition(QSPage *page) {
 if (nullPtr(obj,"getPagePosition")) return -1;
 if (nullPtr(page,"getPagePosition")) return -1;
 try {
  return obj->getPagePosition(page->get());
 } catch (...) {
  //Probably "page not found"
  return -1;
 }
}

/**
 \copydoc getPagePosition(QSPage *)
 QSA bugfix version
*/
int QSPdf::getPagePosition(QObject *page) {
 QSPage *qpage=qobject_cast<QSPage*>(page,"getPagePosition",1,"Page");
 if (!qpage) return -1;//Something invalid passed
 return getPagePosition(qpage);
}

/**
 Return number of pages in document
 \see CPdf::getPageCount
 @return count of pages
*/
int QSPdf::getPageCount() {
 if (nullPtr(obj,"getPageCount")) return -1;
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
 if (nullPtr(obj,"getPage")) return NULL;
 try {
  return new QSPage(obj->getPage(position),base);
 } catch (PageNotFoundException &e) {
  base->errorException("Pdf","getPage",tr("Page not found"));
  return NULL;
 }
}

/**
 Get first page
 \see CPdf::getFirstPage
 @return first page in document
*/
QSPage* QSPdf::getFirstPage() {
 if (nullPtr(obj,"getFirstPage")) return NULL;
 try {
  return new QSPage(obj->getFirstPage(),base);
 } catch (PageNotFoundException &e) {
  base->errorException("Pdf","getFirstPage",tr("Page not found"));
  return NULL;
 }
}

/**
 Return page next to the one specified
 \see CPdf::getNextPage
 @param page Page to examine
 @return next page
*/
QSPage* QSPdf::getNextPage(QSPage* page) {
 if (nullPtr(obj,"getNextPage")) return NULL;
 try {
  return new QSPage(obj->getNextPage(page->get()),base);
 } catch (PageNotFoundException &e) {
  base->errorException("Pdf","getNextPage",tr("Page not found"));
  return NULL;
 }
}

/**
 Return page previous to the one specified
 \see CPdf::getPrevPage
 @param page Page to examine
 @return previous page
*/
QSPage* QSPdf::getPrevPage(QSPage* page) {
 if (nullPtr(obj,"getPrevPage")) return NULL;
 try {
  return new QSPage(obj->getPrevPage(page->get()),base);
 } catch (PageNotFoundException &e) {
  base->errorException("Pdf","getPrevPage",tr("Page not found"));
  return NULL;
 }
}

/**
 Return true if next page in relation to given page exists
 \see CPdf::hasNextPage
 @param page Page to examine
 @return presence of next page
*/
bool QSPdf::hasNextPage(QSPage* page) {
 if (nullPtr(obj,"hasNextPage")) return false;
 return obj->hasNextPage(page->get());
}

/**
 Return true if previous page in relation to given page exists
 \see CPdf::hasPrevPage
 @param page Page to examine
 @return presence of previous page
*/
bool QSPdf::hasPrevPage(QSPage* page) {
 if (nullPtr(obj,"hasPrevPage")) return false;
 return obj->hasPrevPage(page->get());
}

/**
 \copydoc getNextPage(QSPage*)
 qsa bugfix version
*/
QSPage* QSPdf::getNextPage(QObject* page) {
 QSPage *qpage=qobject_cast<QSPage*>(page,"getNextPage",1,"Page");
 if (!qpage) return NULL;
 return getNextPage(qpage);
}

/**
 \copydoc getPrevPage(QSPage*)
 qsa bugfix version
*/
QSPage* QSPdf::getPrevPage(QObject* page) {
 QSPage *qpage=qobject_cast<QSPage*>(page,"getNextPage",1,"Page");
 if (!qpage) return NULL;
 return getPrevPage(qpage);
}

/**
 \copydoc hasNextPage(QSPage*)
 qsa bugfix version
*/
bool QSPdf::hasNextPage(QObject* page) {
 QSPage *qpage=qobject_cast<QSPage*>(page,"hasNextPage",1,"Page");
 if (!qpage) return false;
 return hasNextPage(qpage);
}

/**
 \copydoc hasPrevPage(QSPage*)
 qsa bugfix version
*/
bool QSPdf::hasPrevPage(QObject* page) {
 QSPage *qpage=qobject_cast<QSPage*>(page,"hasPrevPage",1,"Page");
 if (!qpage) return false;
 return hasPrevPage(qpage);
}

/**
 Get count of revisions
 \see CPdf::getRevisionsCount
 @return count of revisions
*/
int QSPdf::getRevisionsCount() {
 if (nullPtr(obj,"getRevisionsCount")) return 0;
 return obj->getRevisionsCount();
}

/**
 Get actual revision number
 \see CPdf::getActualRevision
 @return actual revison
*/
int QSPdf::getActualRevision() {
 if (nullPtr(obj,"getActualRevision")) return 0;
 return obj->getActualRevision();
}

/**
 Get last page in document
 \see CPdf::getLastPage
 @return last page
*/
QSPage* QSPdf::getLastPage() {
 if (nullPtr(obj,"getLastPage")) return 0;
 try {
  return new QSPage(obj->getLastPage(),base);
 } catch (PageNotFoundException &e) {
  base->errorException("Pdf","getLastPage",tr("Page not found"));
  return NULL;
 }
}

/**
 Get CPdf held inside this class. Not exposed to scripting
 @return CPdf object
*/
boost::shared_ptr<CPdf> QSPdf::get() const {
 return obj;
}

/**
 Set CPdf held inside this class. Not exposed to scripting
 @param pdf CPdf object
*/
void QSPdf::set(boost::shared_ptr<CPdf> pdf) {
 obj=pdf;
}

/**
 Checks whether document is encrypted.
 @return true if encrypted, false otherwise.
 */
bool QSPdf::isEncrypted() {
 if (nullPtr(obj,"isEncrypted")) return 0;
 return pdfobjects::utils::isEncrypted(obj);
}

/**
 Checks whether document is opened in read-only mode
 @return true if read-only, false if read-write.
 */
bool QSPdf::isReadOnly() {
 if (nullPtr(obj,"isReadOnly")) return 0;
 return obj->getMode()==CPdf::ReadOnly;
}

} // namespace gui
