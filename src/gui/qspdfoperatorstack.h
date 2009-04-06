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
#ifndef __QSPDFOPERATORSTACK_H__
#define __QSPDFOPERATORSTACK_H__

#include "qscobject.h"
#include <stdlib.h>
#include <kernel/pdfoperators.h>

namespace gui {

class QSPdfOperator;

using namespace pdfobjects;

 /** Double ended queue with operators */
typedef std::deque<boost::shared_ptr<PdfOperator> > pdfOpDeque;

/*=
 This type represents arbitrary array of PDFOperator items
 It hold zero or more values indexed by positive integer, starting from zero.
 You can add pdf operators to beginning or end of the array, so the array can behave like double-ended stack
*/
/** \brief QObject wrapper around array (stack) of PDF Operators */
class QSPdfOperatorStack : public QSCObject {
 Q_OBJECT
public:
 QSPdfOperatorStack(BaseCore *_base);
 virtual ~QSPdfOperatorStack();
 boost::shared_ptr<PdfOperator> get(int index);
 void append(boost::shared_ptr<PdfOperator> prop);
 void prepend(boost::shared_ptr<PdfOperator> prop);
public slots:
 /*- Return size of the stack (number of operators) */
 int count();
 /*- append one PdfOperator element to end of stack. */
 void append(QSPdfOperator *prop);
 void append(QObject *obj);
 /*- prepend one PdfOperator element at beginning of stack. */
 void prepend(QSPdfOperator *prop);
 void prepend(QObject *obj);
 /*- Return first element in stack (from beginning of stack) */
 QSCObject* first();
 /*- Return last element in stack (from end of stack) */
 QSCObject* last();
 /*- Remove first element from stack (from beginning of stack) */
 void removeFirst();
 /*- Remove last element from stack (from end of stack) */
 void removeLast();
 /*- Get PdfOperator element with given index from stack */
 QSCObject* element(int index);
 const pdfOpDeque& get();
private:
 /** Double ended queue holding operators */
 pdfOpDeque obj;
};

} // namespace gui

#endif
