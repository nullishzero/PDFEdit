/** @file
 QObject wrapper around CContentStream (content stream contained in a page)<br>
 Export some functions to scripting.<br>
 See doc/user/scripting.xml or kernel documentation for more informations about these functions
 @author Martin Petricek
*/

#include "qscontentstream.h"
#include "qspdfoperator.h"
#include "qspdfoperatoriterator.h"
#include <pdfoperators.h>

namespace gui {

/**
 Construct wrapper with given CContentStream
 @param _cs Content Stream
 @param _base scripting base
 */
QSContentStream::QSContentStream(boost::shared_ptr<CContentStream> _cs,BaseCore *_base) : QSCObject ("ContentStream",_base) {
 obj=_cs;
}

/** destructor */
QSContentStream::~QSContentStream() {
}

/**
 Return text representation of Content Stream
 \see CContentStream::getStringRepresentation
 @return string representation
*/
QString QSContentStream::getText() {
 std::string text;
 obj->getStringRepresentation(text);
 return text;
}

/**
 get CContentStream held inside this class. Not exposed to scripting
 @return stored CContentStream shared pointer
*/
boost::shared_ptr<CContentStream> QSContentStream::get() {
 return obj;
}

/**
 Check validity of given PDF operator<br>
 Operator is valid if:<br>
 1. Is not NULL<br>
 2. Is in this contentstream (if checkThis is true)<br>
 @param op Operator to check
 @param checkThis Check also if operator is in this contentstream
 @return true if operator is valid, false if not.
*/
bool QSContentStream::opValid(QSPdfOperator *op,bool checkThis/*=false*/) {
 //op is NULL
 if (!op) return false;
 //shared_ptr in op is NULL (should not happen)
 assert(op->get());
 if (!op->get()) return false;
 if (checkThis) {
  //Check if operator is in correct contentstream (this contentstream)
  if (op->get()->getContentStream()!=obj.get()) return false;
 }
 //We found no errors, so we declare it valid
 return true;
}

/**
 Replace old operator oldOp with new operator newOp in this stream
 pre_replace must be called on old operator before calling replace
 \see pre_replace
 @param oldOp Old operator
 @param newOp New operator
 @param indicateChange If set to true (default), changes will be written to underlying stream
*/
void QSContentStream::replace(boost::shared_ptr<PdfOperator> oldOp,boost::shared_ptr<PdfOperator> newOp,bool indicateChange/*=true*/) {
 obj->replaceOperator(oldOp,newOp,itPrev,itNext,indicateChange);
}

/**
 Replace old operator oldOp with new operator newOp in this stream
 @param oldOp Old operator that will be replaced
 @param newOp New operator (replacement)
 @param itPrev Previous iterator of new operator in iterator list
 @param itNext Next iterator of new operator in iterator list
 @param indicateChange If set to true (default), changes will be written to underlying stream
*/
void QSContentStream::replace(QSPdfOperator* oldOp,QSPdfOperator* newOp,QSPdfOperatorIterator* itPrev,QSPdfOperatorIterator* itNext,bool indicateChange/*=true*/) {
 if (!(oldOp && newOp && itPrev && itNext)) return;
 obj->replaceOperator(oldOp->get(),newOp->get(),*(itPrev->get()),*(itNext->get()),indicateChange);
}

/**
 QSA bugfix version
 \copydoc replace(QSPdfOperator*,QSPdfOperator*,QSPdfOperatorIterator*,QSPdfOperatorIterator*,bool)
*/
void QSContentStream::replace(QObject* oldOp,QObject* newOp,QObject* itPrev,QObject* itNext,bool indicateChange/*=true*/) {
 QSPdfOperator* _oldOp=dynamic_cast<QSPdfOperator*>(oldOp);
 QSPdfOperator* _newOp=dynamic_cast<QSPdfOperator*>(newOp);
 QSPdfOperatorIterator* _itPrev=dynamic_cast<QSPdfOperatorIterator*>(itPrev);
 QSPdfOperatorIterator* _itNext=dynamic_cast<QSPdfOperatorIterator*>(itNext);
 if (!(_oldOp && _newOp && _itPrev && _itNext)) return;
 obj->replaceOperator(_oldOp->get(),_newOp->get(),*(_itPrev->get()),*(_itNext->get()),indicateChange);
}

/**
 Prepare for replacing operator with some other - save it's next and prev iterator
 @param op operator
 */
void QSContentStream::pre_replace(boost::shared_ptr<PdfOperator> op) {
 itPrev=PdfOperator::getIterator(op);itPrev.prev();
 itNext=PdfOperator::getIterator(getLastOperator(op));itNext.next();
}

/**
 Delete operator from content stream
 \see CContentStream::deleteOperator 
 @param op Operator to delete
 @param indicateChange If set to true (default), changes will be written to underlying stream
*/
void QSContentStream::deleteOperator(QSPdfOperator *op,bool indicateChange/*=true*/) {
 //First check for validity
 if (!opValid(op,true)) return;
 obj->deleteOperator(op->get(),indicateChange);
}

/**
 \copydoc deleteOperator(QSPdfOperator*,bool) 
 QSA bugfixed version
*/
void QSContentStream::deleteOperator(QObject *op,bool indicateChange/*=true*/) {
 QSPdfOperator* qop=dynamic_cast<QSPdfOperator*>(op);
 if (!qop) return;
 deleteOperator((QSPdfOperator*)qop,indicateChange);
}

/**
 Insert operator in content stream
 \see CContentStream::insertOperator
 @param op after which one to insert
 @param newOp operator to insert
 @param indicateChange write changes to underlying stream?
*/
void QSContentStream::insertOperator(QSPdfOperator *op,QSPdfOperator *newOp,bool indicateChange/*=true*/) {
 //First check for validity
 if (!opValid(op,this)) return;
 if (!opValid(newOp)) return;
 obj->insertOperator(op->get(),newOp->get(),indicateChange);
}

/**
 \copydoc insertOperator(QSPdfOperator*,QSPdfOperator*,bool) 
 QSA bugfixed version
*/
void QSContentStream::insertOperator(QObject *op,QObject *newOp,bool indicateChange/*=true*/) {
 QSPdfOperator* qop=dynamic_cast<QSPdfOperator*>(op);
 QSPdfOperator* qopNew=dynamic_cast<QSPdfOperator*>(newOp);
 if (!qop) return;
 if (!qopNew) return;
 insertOperator((QSPdfOperator*)qop,(QSPdfOperator*)qopNew,indicateChange);
}

/**
 Save changes to underlying content stream
 \see CContentStream::saveChange
*/
void QSContentStream::saveChange() {
 obj->saveChange();
}

} // namespace gui
