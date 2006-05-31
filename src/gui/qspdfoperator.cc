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
 //Paranoid check
 assert(obj->getContentStream()==csRef.get());
}

/** destructor */
QSPdfOperator::~QSPdfOperator() {
}

/** Call PdfOperator::getStringRepresentation(ret); return ret */
QString QSPdfOperator::getText() {
 std::string text;
 if (!obj) throw NullPointerException("PdfOperator","getText");
 obj->getStringRepresentation(text);
 return text;
}

/** 
 Create new operator iterator from this PDF operator.
 The iterator will be initially positioned at this item
*/
QSPdfOperatorIterator* QSPdfOperator::iterator() {
 return new QSPdfOperatorIterator(obj,csRef,base);
}

/** 
 Create new text operator iterator from this PDF operator.
 The iterator will be initialized from this item 
*/
QSPdfOperatorIterator* QSPdfOperator::textIterator() {
 TextOperatorIterator* opText=new TextOperatorIterator(PdfOperator::getIterator<TextOperatorIterator>(obj));
 return new QSPdfOperatorIterator(opText,csRef,base);
}

/** Call PdfOperator::getOperatorName(ret); return ret */
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

/** Call PdfOperator::setNext() */
void QSPdfOperator::setNext(QSPdfOperator *op) {
 if (!obj) throw NullPointerException("PdfOperator","setNext");
 obj->setNext(op->get());
}

/** Call PdfOperator::setPrev() */
void QSPdfOperator::setPrev(QSPdfOperator *op) {
 if (!obj) throw NullPointerException("PdfOperator","setPrev");
 obj->setPrev(op->get());
}

/** setNext: QSA bug workaround */
void QSPdfOperator::setNext(QObject *op) {
 if (!obj) throw NullPointerException("PdfOperator","setNext");
 QSPdfOperator *qop=dynamic_cast<QSPdfOperator*>(op);
 if (!qop) return; //Invalid parameter
 obj->setNext(qop->get());
}

/** setPrev: QSA bug workaround */
void QSPdfOperator::setPrev(QObject *op) {
 if (!obj) throw NullPointerException("PdfOperator","setPrev");
 QSPdfOperator *qop=dynamic_cast<QSPdfOperator*>(op);
 if (!qop) return; //Invalid parameter
 obj->setPrev(qop->get());
}

/** Call containsNonStrokingOperator() on this operator */
bool QSPdfOperator::containsNonStrokingOperator() {
 return pdfobjects::containsNonStrokingOperator(obj);
}

/** Call containsStrokingOperator() on this operator */
bool QSPdfOperator::containsStrokingOperator() {
 return pdfobjects::containsStrokingOperator(obj);
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

/** Call PdfOperator::push_back() */
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
 Call PdfOperator::push_back()
 QSA bug workaround version
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

/** get Pdf Operator shared pointer held inside this class. Not exposed to scripting */
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
 if (!csRef.get()) {
  //ContentStream is not known, so we can't return it
  return NULL;
 }
 return new QSContentStream(csRef,base);
}

} // namespace gui
