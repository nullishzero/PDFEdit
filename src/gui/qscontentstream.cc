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
  if (op->get()->getContentStream()!=obj) return false;
 }
 //We found no errors, so we declare it valid
 return true;
}

/**
 Replace old operator oldOp with new operator newOp in this stream
 @param oldOp Old operator that will be replaced
 @param newOp New operator (replacement)
 @param indicateChange If set to true (default), changes will be written to underlying stream
*/
void QSContentStream::replace(QSPdfOperator* oldOp,QSPdfOperator* newOp,bool indicateChange/*=true*/) {
 if (!(oldOp && newOp)) return;
 obj->replaceOperator(oldOp->get(),newOp->get(),indicateChange);
}

/**
 QSA bugfix version
 \copydoc replace(QSPdfOperator*,QSPdfOperator*,bool)
*/
void QSContentStream::replace(QObject* oldOp,QObject* newOp,bool indicateChange/*=true*/) {
 QSPdfOperator* _oldOp=qobject_cast<QSPdfOperator*>(oldOp,"replace",1,"PdfOperator");
 QSPdfOperator* _newOp=qobject_cast<QSPdfOperator*>(newOp,"replace",2,"PdfOperator");
 if (!(_oldOp && _newOp)) return;
 obj->replaceOperator(_oldOp->get(),_newOp->get(),indicateChange);
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
 QSPdfOperator* qop=qobject_cast<QSPdfOperator*>(op,"deleteOperator",1,"PdfOperator");
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
 QSPdfOperator* qop=qobject_cast<QSPdfOperator*>(op,"insertOperator",1,"PdfOperator");
 QSPdfOperator* qopNew=qobject_cast<QSPdfOperator*>(newOp,"insertOperator",1,"PdfOperator");
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
