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
#include <utils/iterator.h>

namespace gui {

using namespace pdfobjects;

/**
 Construct wrapper around new Iterator with given Pdf Operator used as initial item
 @param op PDF Operator
 @param _base scripting base
 */
QSPdfOperatorIterator::QSPdfOperatorIterator(boost::shared_ptr<PdfOperator> op,Base *_base) : QSCObject ("PdfOperatorIterator",_base) {
 obj=new PdfOperator::Iterator(PdfOperator::getIterator(op));
}

/**
 Construct wrapper around new Iterator with given Pdf Operator used as initial item
 and with reference to content stream in which the operator resides
 @param op PDF Operator
 @param cs CContentstream in which this PdfOperator is contained
 @param _base scripting base
*/
QSPdfOperatorIterator::QSPdfOperatorIterator(boost::shared_ptr<PdfOperator> op,boost::shared_ptr<CContentStream> cs,Base *_base) : QSCObject ("PdfOperatorIterator",_base) {
 obj=new PdfOperator::Iterator(PdfOperator::getIterator(op));
 csRef=cs;
 //Paranoid check
 assert(op->getContentStream()==csRef.get());
}

/**
 Construct wrapper around new Iterator
 and with reference to content stream in which the iterator resides
 This class takes ownership of the iterator
 @param op PDF Operator iterator
 @param cs CContentstream in which this PdfOperator is contained
 @param _base scripting base
*/
QSPdfOperatorIterator::QSPdfOperatorIterator(PdfOperator::Iterator *op,boost::shared_ptr<CContentStream> cs,Base *_base) : QSCObject ("PdfOperatorIterator",_base) {
 assert(op);
 obj=op;
 csRef=cs;
 //Paranoid check
 assert(op->getCurrent()->getContentStream()==csRef.get());
}

/** destructor */
QSPdfOperatorIterator::~QSPdfOperatorIterator() {
 delete obj;
}

/**
 Return current operator from the iterator
 @return Current operator
*/
QSPdfOperator* QSPdfOperatorIterator::current() {
 //If contentstream is not known, empty reference will be passed this way
 return new QSPdfOperator(obj->getCurrent(),csRef,base);
}

/**
 Duplicate the operator, while trying to preserve its type
 @param src source iterator
 @return new iterator of same type aas source iterator, initially pointing at the same element
*/
PdfOperator::Iterator* QSPdfOperatorIterator::copyIterator(PdfOperator::Iterator *src) {
 assert(src);
 //TODO: this is not the best, but should be sufficient
 //Try to duplicate as TextOperatorIterator
 TextOperatorIterator *newOpT=dynamic_cast<TextOperatorIterator*>(src);
 if (newOpT) return new TextOperatorIterator(*newOpT);
 //Try to duplicate as NonStrokingOperatorIterator
 NonStrokingOperatorIterator *newOpN=dynamic_cast<NonStrokingOperatorIterator*>(src);
 if (newOpN) return new NonStrokingOperatorIterator(*newOpN);
 //Try to duplicate as StrokingOperatorIterator
 StrokingOperatorIterator *newOpS=dynamic_cast<StrokingOperatorIterator*>(src);
 if (newOpS) return new StrokingOperatorIterator(*newOpS);
 //Try to duplicate as InlineImageOperatorIterator
 InlineImageOperatorIterator *newOpI=dynamic_cast<InlineImageOperatorIterator*>(src);
 if (newOpI) return new InlineImageOperatorIterator(*newOpI);
 //Try to duplicate as ChangeableOperatorIterator
 ChangeableOperatorIterator *newOpC=dynamic_cast<ChangeableOperatorIterator*>(src);
 if (newOpC) return new ChangeableOperatorIterator(*newOpC);
 //Fallback to standard iterator
 return new PdfOperator::Iterator(*src);
}

/**
 Copy this iterator and return new iterator pointing initially to same PDF Operator
 @return New iterator pointing to same item.
*/
QSPdfOperatorIterator* QSPdfOperatorIterator::copy() {
 PdfOperator::Iterator *newOp=copyIterator(obj);
 //If contentstream is not known, empty reference will be passed this way
 return new QSPdfOperatorIterator(newOp,csRef,base);
}

/**
 Call PdfOperator::Iterator::next()
 Move iterator to next operator
 @return Operator at new position, or NULL if the new position is already invalid
*/
QSPdfOperator* QSPdfOperatorIterator::next() {
 try {
  obj->next();
  return new QSPdfOperator(obj->getCurrent(),base);
 } catch (iterator::IteratorInvalidObjectException &e) {
  //Already at invalid position
  return NULL; ///a.k.a. false ... 
 }
}

/**
 Call PdfOperator::Iterator::prev()
 Move iterator to previous operator
 @return Operator at new position, or NULL if the new position is already invalid
*/
QSPdfOperator* QSPdfOperatorIterator::prev() {
 try {
  obj->prev();
  return new QSPdfOperator(obj->getCurrent(),base);
 } catch (iterator::IteratorInvalidObjectException &e) {
  //Already at invalid position
  return NULL; ///a.k.a. false ... 
 }
}

/**
 Call PdfOperator::Iterator::isEnd()
 Are we at beginning/end of the list?
*/
bool QSPdfOperatorIterator::isEnd() {
 return obj->isEnd();
}

/**
 Get Pdf Operator iterator held inside this class.
 Not exposed to scripting
 @return PDF Operator Iterator
*/
PdfOperator::Iterator* QSPdfOperatorIterator::get() {
 return obj;
}

/**
 Get Pdf Operator shared pointer - the current item from iterator held inside this class.
 Not exposed to scripting
 @return PDF Operator - current item
*/
boost::shared_ptr<PdfOperator> QSPdfOperatorIterator::getCurrent() {
 return obj->getCurrent();
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
