/*                                                                              
 * PDFedit - free program for PDF document manipulation.                        
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko, 
 *                                              Miroslav Jahoda,       
 *                                              Jozef Misutka, 
 *                                              Martin Petricek                                             
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit                                                                      
 */ 
/** @file
 QObject wrapper around CAnnotation<br>
 Export some functions to scripting.<br>
 See doc/user/scripting.xml or kernel documentation for more informations about these functions
 @author Martin Petricek
*/

#include "qsannotation.h"
#include "qsdict.h"
#include "qspage.h"
#include <cannotation.h>
#include <cpage.h>
#include "pdfutil.h"

namespace gui {

using namespace pdfobjects;
using namespace util;

/**
 Construct wrapper with given CAnnotation
 @param pdfObj CAnnotation object
 @param _base scripting base
 @param _page page holding the annotation
 */
QSAnnotation::QSAnnotation(boost::shared_ptr<CAnnotation> pdfObj,boost::shared_ptr<CPage> _page,BaseCore *_base) : QSCObject ("Annotation",_base) {
 obj=pdfObj;
 assert(obj);
 page=_page;
}

/** destructor */
QSAnnotation::~QSAnnotation() {
}

/** Get annotation's page */
QSPage* QSAnnotation::getPage() { 
 if (!page.get()) return NULL;
 return new QSPage(page,base);
}

/** Call CAnnotation::getDictionary */
QSDict* QSAnnotation::getDictionary() { 
 return new QSDict(obj->getDictionary(),base);
}

/** Return type identifier of annotation */
QString QSAnnotation::getType() {
 return annotType(obj);
}

/** Return human-readable, localized type identifier of annotation */
QString QSAnnotation::getTypeName() {
 return annotTypeName(obj);
}

/**
 Remove this annotation from its page, if it is in a page
 @return true if removed
*/
bool QSAnnotation::remove() {
 if (!page.get()) return false;//Not in page
 try {
  bool result=page->delAnnotation(obj);
  page.reset();//It's not in page anymore
  return result;
 } catch (ReadOnlyDocumentException &e) {
  base->errorException("Annotation","remove",QObject::tr("Document is read-only"));
  return false;
 }
}

/** get CAnnotation held inside this class. Not exposed to scripting */
boost::shared_ptr<CAnnotation> QSAnnotation::get() const {
 return obj;
}

} // namespace gui

