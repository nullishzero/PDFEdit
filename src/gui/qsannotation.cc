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
 QObject wrapper around CAnnotation<br>
 Export some functions to scripting.<br>
 See doc/user/scripting.xml or kernel documentation for more informations about these functions
 @author Martin Petricek
*/

#include "qsannotation.h"
#include "qsdict.h"
#include "qspage.h"
#include <kernel/cannotation.h>
#include <kernel/cpage.h>
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

