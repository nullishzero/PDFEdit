/** @file
 QObject wrapper around PdfOperator (one operator found inside content stream)<br>
 Export some functions to scripting.<br>
 See doc/user/scripting.xml or kernel documentation for more informations about these functions
 @author Martin Petricek
*/

#include "qspdfoperator.h"
#include "qspdfoperatorstack.h"
#include "qspdfoperatoriterator.h"
#include "qsimporter.h"
#include "qscontentstream.h"
#include "qsiproperty.h"
#include "qsipropertyarray.h"
#include <ccontentstream.h>
#include <iterator.h>
#include "nullpointerexception.h"
#include <qstring.h>

namespace gui {

using namespace pdfobjects;

/**
 Construct wrapper with given PdfOperator
 @param op PDF Operator
 @param _base scripting base
 */
QSPdfOperator::QSPdfOperator(boost::shared_ptr<PdfOperator> op,Base *_base) : QSCObject ("PdfOperator",_base) {
 obj=op;
}

/**
 Construct wrapper with empty PdfOperator (NULL)
 @param _base scripting base
 */
QSPdfOperator::QSPdfOperator(Base *_base) : QSCObject ("PdfOperator",_base) {
}

/**
 Construct wrapper with given PdfOperator and reference to its content stream
 @param op PDF Operator
 @param cs CContentstream in which this PdfOperator is contained
 @param _base scripting base
*/
QSPdfOperator::QSPdfOperator(boost::shared_ptr<PdfOperator> op,boost::shared_ptr<CContentStream> cs,Base *_base) : QSCObject ("PdfOperator",_base) {
 obj=op;
 csRef=cs;
 csCheck();
}

/**
 Check if the csRef point to the same contentstream,
 as the content stream operator thinks it is in<br>
 Reset the csRef if these two do not match
*/
void QSPdfOperator::csCheck() {
 if(obj->getContentStream()!=csRef.get()) {
  //The stream is invalid, set it to NULL rather than to invalid stream
  csRef.reset();
 }
}

/** destructor */
QSPdfOperator::~QSPdfOperator() {
}

/**
 Return text representation of pdf operator
 \see PdfOperator::getStringRepresentation
 @return string representation
*/
QString QSPdfOperator::getText() {
 std::string text;
 if (!obj) throw NullPointerException("PdfOperator","getText");
 obj->getStringRepresentation(text);
 return text;
}

/** 
 Create new operator iterator from this PDF operator.
 The iterator will be initially positioned at this item
 @return new iterator
*/
QSPdfOperatorIterator* QSPdfOperator::iterator() {
 csCheck();
 return new QSPdfOperatorIterator(obj,csRef,base);
}

/** 
 Create new text operator iterator from this PDF operator.
 The iterator will be initialized from this item 
 @return new text iterator
*/
QSPdfOperatorIterator* QSPdfOperator::textIterator() {
 TextOperatorIterator* opText=new TextOperatorIterator(PdfOperator::getIterator<TextOperatorIterator>(obj));
 csCheck();
 return new QSPdfOperatorIterator(opText,csRef,base);
}

/** 
 Create new font operator iterator from this PDF operator.
 The iterator will be initialized from this item 
 @return new font iterator
*/
QSPdfOperatorIterator* QSPdfOperator::fontIterator() {
 FontOperatorIterator* opFont=new FontOperatorIterator(PdfOperator::getIterator<FontOperatorIterator>(obj));
 csCheck();
 return new QSPdfOperatorIterator(opFont,csRef,base);
}

/**
 Get operator name
 \see PdfOperator::getOperatorName
 @return operator name
*/
QString QSPdfOperator::getName() {
 if (!obj) throw NullPointerException("PdfOperator","getName");
 std::string text;
 obj->getOperatorName(text);
 return text;
}

/**
 Return stack with all child operators
 @return Operator stack
*/
QSPdfOperatorStack* QSPdfOperator::childs() {
 if (!obj) throw NullPointerException("PdfOperator","childs");
 QSPdfOperatorStack* ret=new QSPdfOperatorStack(base);
 PdfOperator::PdfOperators opList;
 obj->getChildren(opList);
 for (PdfOperator::PdfOperators::iterator it=opList.begin();it!=opList.end();it++) {
  ret->append(*it);
 }
 return ret;
}

/**
 Return number of child operators under this operator
 @return number of child operators
*/
int QSPdfOperator::childCount() {
 if (!obj) throw NullPointerException("PdfOperator","childCount");
 return obj->getChildrenCount();
}

/**
 Return all parameters of this operator
 @return Array with parameters
*/
QSIPropertyArray* QSPdfOperator::params() {
 if (!obj) throw NullPointerException("PdfOperator","params");
 QSIPropertyArray* ret=new QSIPropertyArray(base);
 PdfOperator::Operands paramList;
 obj->getParameters(paramList);
 for (PdfOperator::Operands::iterator it=paramList.begin();it!=paramList.end();it++) {
  ret->append(*it);
 }
 return ret;
}

/**
 Return number of parameters for this operator
 @return number of parameters
*/
int QSPdfOperator::paramCount() {
 if (!obj) throw NullPointerException("PdfOperator","paramCount");
 return obj->getParametersCount();
}

/**
 Set next operator
 \see PdfOperator::setNext
 @param op operator to set as next
*/
void QSPdfOperator::setNext(QSPdfOperator *op) {
 if (!obj) throw NullPointerException("PdfOperator","setNext");
 obj->setNext(op->get());
}

/**
 Set previous operator
 \see PdfOperator::setPrev
 @param op operator to set as previous
*/
void QSPdfOperator::setPrev(QSPdfOperator *op) {
 if (!obj) throw NullPointerException("PdfOperator","setPrev");
 obj->setPrev(op->get());
}

/**
 QSA bug workaround
 \copydoc setNext(QSPdfOperator*)
*/
void QSPdfOperator::setNext(QObject *op) {
 if (!obj) throw NullPointerException("PdfOperator","setNext");
 QSPdfOperator *qop=dynamic_cast<QSPdfOperator*>(op);
 if (!qop) return; //Invalid parameter
 obj->setNext(qop->get());
}

/**
 QSA bug workaround
 \copydoc setPrev(QSPdfOperator*)
*/
void QSPdfOperator::setPrev(QObject *op) {
 if (!obj) throw NullPointerException("PdfOperator","setPrev");
 QSPdfOperator *qop=dynamic_cast<QSPdfOperator*>(op);
 if (!qop) return; //Invalid parameter
 obj->setPrev(qop->get());
}

/**
 Call containsNonStrokingOperator() on this operator
 @return result
 */
bool QSPdfOperator::containsNonStrokingOperator() {
 if (!obj) throw NullPointerException("PdfOperator","containsNonStrokingOperator");
 return pdfobjects::containsNonStrokingOperator(obj);
}

/**
 Call containsStrokingOperator() on this operator
 @return result
 */
bool QSPdfOperator::containsStrokingOperator() {
 if (!obj) throw NullPointerException("PdfOperator","containsStrokingOperator");
 return pdfobjects::containsStrokingOperator(obj);
}

/**
 Call getLastOperator() on this operator
 @return last operator
*/
QSPdfOperator* QSPdfOperator::getLastOperator() {
 if (!obj) throw NullPointerException("PdfOperator","getLastOperator");
 return new QSPdfOperator(pdfobjects::getLastOperator(obj),base);
}

/**
 Return true, if operator inside this wrapper is NULL
 @return Is operator empty?
*/
bool QSPdfOperator::isEmpty() {
 return (obj.get()==NULL);
}

/**
 Remove itself from the stream. After this operation, the operator must not be used
 - any attempt to use it will probably end up with an exception
*/
void QSPdfOperator::remove() {
 if (!obj) throw NullPointerException("PdfOperator","remove");
 CContentStream* cStream=obj->getContentStream();
 if (!cStream) {
  //This operator is not in any content stream, so technically, it is already removed from it :)
  return;
 }
 cStream->deleteOperator(obj);
 treeNeedReload();
}

/**
 Add an operator to the end of composite.
 \see PdfOperator::push_back
 @param op Operator to be added.
 @param prev Operator, after which we should place the added one
*/
void QSPdfOperator::pushBack(QSPdfOperator *op,QSPdfOperator *prev/*=NULL*/) {
 if (!obj) throw NullPointerException("PdfOperator","pushBack");
 if (!prev) {
  boost::shared_ptr<PdfOperator> emptyOperator;
  obj->push_back(op->get(),emptyOperator);
  return;
 }
 obj->push_back(op->get(),prev->get());
}

/**
 QSA bug workaround version
 \copydoc pushBack(QSPdfOperator *,QSPdfOperator *)
*/
void QSPdfOperator::pushBack(QObject *op,QObject *prev/*=NULL*/) {
 if (!obj) throw NullPointerException("PdfOperator","pushBack");
 QSPdfOperator *qop=dynamic_cast<QSPdfOperator*>(op);
 QSPdfOperator *qprev=dynamic_cast<QSPdfOperator*>(prev);
 if (!prev) {
  boost::shared_ptr<PdfOperator> emptyOperator;
  obj->push_back(qop->get(),emptyOperator);
  return;
 }
 obj->push_back(qop->get(),qprev->get());
}

/**
 Get Pdf Operator shared pointer held inside this class. Not exposed to scripting
 @return Pdf Operator shared pointer
*/
boost::shared_ptr<PdfOperator> QSPdfOperator::get() {
 return obj;
}

/**
 Return content stream in which this operator is contained.
 May return NULL
 (if operator is not contained in any content stream or if content stream is not known at time of creation)
 @return QObject wrapper around this operator's content stream
*/
QSContentStream* QSPdfOperator::stream() {
 csCheck();
 if (!csRef.get()) {
  //ContentStream is not known, so we can't return it
  return NULL;
 }
 return new QSContentStream(csRef,base);
}

} // namespace gui
