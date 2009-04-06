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
 QSPdfOperatorStack
 QObject wrapper around array (stack) of PDF Operators<br>
 See doc/user/scripting.xml for more informations about these functions
 @author Martin Petricek
*/

#include "qspdfoperatorstack.h"
#include "qsimporter.h"
#include <kernel/cobject.h>
#include "qspdfoperator.h"

namespace gui {

using namespace pdfobjects;

/**
 Construct wrapper with empty operator stack
 @param _base scripting base
 */
QSPdfOperatorStack::QSPdfOperatorStack(BaseCore *_base) : QSCObject ("PdfOperatorStack",_base) {
}

/** destructor */
QSPdfOperatorStack::~QSPdfOperatorStack() {
}

/**
 get one PdfOperator item held inside this class.
 @param index Zero-based index of item to get
 @return specified PdfOperator
*/
boost::shared_ptr<PdfOperator> QSPdfOperatorStack::get(int index) {
 return obj[index];
}

/**
 append one PdfOperator to end of stack.
 @param prop PdfOperator to append
*/
void QSPdfOperatorStack::append(boost::shared_ptr<PdfOperator> prop) {
 obj.push_back(prop);
}

/**
 append one PdfOperator to end of stack.
 @param prop PdfOperator to append
*/
void QSPdfOperatorStack::append(QSPdfOperator *prop) {
 obj.push_back(prop->get());
}

/**
 append one PdfOperator to end of stack.
 QSA-bugfix version
 @param obj PdfOperator to append
*/
void QSPdfOperatorStack::append(QObject *obj) {
 QSPdfOperator *prop=dynamic_cast<QSPdfOperator*>(obj);
 if (prop) {
  append(prop);
 }
}

/**
 prepend one PdfOperator at beginning of stack.
 @param prop PdfOperator to prepend
*/
void QSPdfOperatorStack::prepend(boost::shared_ptr<PdfOperator> prop) {
 obj.push_front(prop);
}

/**
 prepend one PdfOperator at beginning of stack.
 @param prop PdfOperator to prepend
*/
void QSPdfOperatorStack::prepend(QSPdfOperator *prop) {
 obj.push_front(prop->get());
}

/**
 prepend one PdfOperator at beginning of stack.
 @param obj PdfOperator to prepend
 QSA-bugfix version
*/
void QSPdfOperatorStack::prepend(QObject *obj) {
 QSPdfOperator *prop=dynamic_cast<QSPdfOperator*>(obj);
 if (prop) {
  prepend(prop);
 }
}

/** Return number of items inside this stack */
int QSPdfOperatorStack::count() {
 return obj.size();
}

/** Return const reference to object inside this wrapper */
const pdfOpDeque& QSPdfOperatorStack::get() {
 return obj;
}

/**
 get one PdfOperator item held inside this class.
 @param index Zero-based index of item to get
 @return specified PdfOperator
*/
QSCObject* QSPdfOperatorStack::element(int index) {
 //Check if we are in bounds
 if (index<0) return NULL;
 if (index>=(int)obj.size()) return NULL;
 return new QSPdfOperator(obj[index],base);
}

/**
 get first PdfOperator item (from beginning of stack)
 @return specified PdfOperator
*/
QSCObject* QSPdfOperatorStack::first() {
 //Check if not empty
 if (!obj.size()) return NULL;
 return new QSPdfOperator(obj.front(),base);
}

/**
 get last PdfOperator item (from beginning of stack)
 @return specified PdfOperator
*/
QSCObject* QSPdfOperatorStack::last() {
 //Check if not empty
 if (!obj.size()) return NULL;
 return new QSPdfOperator(obj.back(),base);
}

/**
 Remove first PdfOperator item (from beginning of stack)
*/
void QSPdfOperatorStack::removeFirst() {
 //Check if not empty
 if (!obj.size()) return;
 obj.pop_front();
}

/**
 Remove last PdfOperator item (from beginning of stack)
*/
void QSPdfOperatorStack::removeLast() {
 //Check if not empty
 if (!obj.size()) return;
 obj.pop_back();
}

} // namespace gui
