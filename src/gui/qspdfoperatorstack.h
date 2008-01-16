/*                                                                              
 * PDFedit - free program for PDF document manipulation.                        
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko, 
 *                                              Miroslav Jahoda,       
 *                                              Jozef Misutka, 
 *                                              Martin Petricek                                             
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
