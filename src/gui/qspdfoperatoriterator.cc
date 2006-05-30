/** @file
 QObject wrapper around PdfOperator Iterator<br>
 Export some functions to scripting.<br>
 See doc/user/scripting.xml or kernel documentation for more informations about these functions
 @author Martin Petricek
*/

#include "qspdfoperatoriterator.h"
#include "qspdfoperator.h"
#include "qsimporter.h"
#include "qscontentstream.h"
#include "qsiproperty.h"
#include <ccontentstream.h>

namespace gui {

using namespace pdfobjects;

/**
 Construct wrapper around new Iteratorwith given PdfOperator
 @param op PDF Operator
 @param _base scripting base
 */
QSPdfOperatorIterator::QSPdfOperatorIterator(boost::shared_ptr<PdfOperator> op,Base *_base) : QSCObject ("PdfOperator",_base) {
 obj=PdfOperator::getIterator(op);
}

/**
 Construct wrapper around new Iteratorwith given PdfOperator and reference to conten t stream in which the operator resides
 @param op PDF Operator
 @param cs CContentstream in which this PdfOperator is contained
 @param _base scripting base
*/
QSPdfOperatorIterator::QSPdfOperatorIterator(boost::shared_ptr<PdfOperator> op,boost::shared_ptr<CContentStream> cs,Base *_base) : QSCObject ("PdfOperator",_base) {
 obj=PdfOperator::getIterator(op);
 csRef=cs;
 //Paranoid check
 assert(op->getContentStream()==csRef.get());
}

/** destructor */
QSPdfOperatorIterator::~QSPdfOperatorIterator() {
}

/**
 Return current operator from the iterator
 @return Current operator
*/
QSPdfOperator* QSPdfOperatorIterator::current() {
 //If contentstream is not known, empty reference will be passed this way
 return new QSPdfOperator(obj.getCurrent(),csRef,base);
}

/**
 Call PdfOperator::Iterator::next()
 Move iterator to next operator
*/
void QSPdfOperatorIterator::next() {
 obj.next();
}

/**
 Copy this iterator and return new iterator pointing initially to same PDF Operator
 @return New iterator pointing to same item.
*/
QSPdfOperatorIterator* QSPdfOperatorIterator::copy() {
 //If contentstream is not known, empty reference will be passed this way
 return new QSPdfOperatorIterator(obj.getCurrent(),csRef,base);
}

/**
 Call PdfOperator::Iterator::prev()
 Move iterator to previous operator
*/
void QSPdfOperatorIterator::prev() {
 obj.prev();
}

/**
 Call PdfOperator::Iterator::isEnd()
 Are we at beginning/end of the list?
*/
bool QSPdfOperatorIterator::isEnd() {
 return obj.isEnd();
}

/**
 Get Pdf Operator iterator held inside this class.
 Not exposed to scripting
 @return PDF Operator Iterator
*/
PdfOperator::Iterator QSPdfOperatorIterator::get() {
 return obj;
}

/**
 Get Pdf Operator shared pointer - the current item from iterator held inside this class.
 Not exposed to scripting
 @return PDF Operator - current item
*/
boost::shared_ptr<PdfOperator> QSPdfOperatorIterator::getCurrent() {
 return obj.getCurrent();
}

/**
 Return content stream in which the initial operator used to construct the iterator was contained.
 May return NULL
 (if operator is not contained in any content stream or if content stream is not known at time of creation)
 @return QObject wrapper around this operator's content stream
*/
QSContentStream* QSPdfOperatorIterator::stream() {
 if (!csRef.get()) {
  //ContentStream is not known, so we can't return it
  return NULL;
 }
 return new QSContentStream(csRef,base);
}

} // namespace gui
