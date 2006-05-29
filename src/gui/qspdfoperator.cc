/** @file
 QObject wrapper around PdfOperator (one operator found inside content stream)<br>
 Export some functions to scripting.<br>
 See doc/user/scripting.xml or kernel documentation for more informations about these functions
 @author Martin Petricek
*/

#include "qspdfoperator.h"
#include "qsimporter.h"
#include "qscontentstream.h"
#include "qsiproperty.h"
#include <ccontentstream.h>

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
 obj->getStringRepresentation(text);
 return text;
}

/** Call PdfOperator::getOperatorName(ret); return ret */
QString QSPdfOperator::getName() {
 std::string text;
 obj->getOperatorName(text);
 return text;
}

/**
 Return child operator with given number from this operator.
 Get the list of operators from this PDF operator and store for later use if necessary
 @param childNumber number of child to get
 @return Child operator with given number, or NULL if number is outside range
*/
QSPdfOperator* QSPdfOperator::child(int childNumber) {
 if (numChilds<0) loadChilds();
 if (childNumber<0) return NULL;		//child number outside range
 if (childNumber>=numChilds) return NULL;	//child number outside range
 return new QSPdfOperator(childs[childNumber],base);
}

/**
 Return number of child operators under this operator
 Get the list of operators from this PDF operator and store for later use if necessary
 @return number of child operators under this operator
*/
int QSPdfOperator::childCount() {
 if (numChilds<0) loadChilds();
 return numChilds;
}

/**
 Return parameter with given number from this operator. Parameter is IProperty
 Get the list of parameters from this PDF operator and store for later use if necessary
 @param paramNumber number of parameter to get
 @return Parameter with given number, or NULL if number is outside range
*/
QSCObject* QSPdfOperator::param(int paramNumber) {
 if (numParams<0) loadParams();
 if (paramNumber<0) return NULL;		//Param number outside range
 if (paramNumber>=numParams) return NULL;	//Param number outside range
 return QSImporter::createQSObject(params[paramNumber],base);
}

/**
 Return number of parameters for this operator
 Get the list of parameters from this PDF operator and store for later use if necessary
 @return number of parameters fro this operator
*/
int QSPdfOperator::paramCount() {
 if (numParams<0) loadParams();
 return numParams;
}

/** Call PdfOperator::getChildren(), store result */
void QSPdfOperator::loadParams() {
 PdfOperator::PdfOperators opList;
 obj->getChildren(opList);
 //We need vector. We get list. We must copy.
 childs.clear();
 std::copy(opList.begin(),opList.end(),std::back_inserter(childs));
 //Store number of childs
 numChilds=childs.size(); 
}

/** Call PdfOperator::getChildren(), store result */
void QSPdfOperator::loadChilds() {
 PdfOperator::Operands paramList;
 obj->getParameters(paramList);
 //We need vector. We get deque. We must copy.
 params.clear();
 std::copy(paramList.begin(),paramList.end(),std::back_inserter(params));
 //Store number of childs
 numParams=params.size(); 
}

/** Call PdfOperator::setNext() */
void QSPdfOperator::setNext(QSPdfOperator *op) {
 obj->setNext(op->get());
}

/** Call PdfOperator::setPrev() */
void QSPdfOperator::setPrev(QSPdfOperator *op) {
 obj->setPrev(op->get());
}

/** setNext: QSA bug workaround */
void QSPdfOperator::setNext(QObject *op) {
 QSPdfOperator *qop=dynamic_cast<QSPdfOperator*>(op);
 if (!qop) return; //Invalid parameter
 obj->setNext(qop->get());
}

/** setPrev: QSA bug workaround */
void QSPdfOperator::setPrev(QObject *op) {
 QSPdfOperator *qop=dynamic_cast<QSPdfOperator*>(op);
 if (!qop) return; //Invalid parameter
 obj->setPrev(qop->get());
}

/**
 Remove itself from the stream. After this operation, the operator must not be used
 - any attempt to use it will probably end up with an exception
 */
void QSPdfOperator::remove() {
 obj->getContentStream()->deleteOperator(obj);
 treeNeedReload();
}

/** Call PdfOperator::push_back() */
void QSPdfOperator::pushBack(QSPdfOperator *op,QSPdfOperator *prev) {
 obj->push_back(op->get(),prev->get());
}

/**
 Call PdfOperator::push_back()
 QSA bug workaround version
*/
void QSPdfOperator::pushBack(QObject *op,QObject *prev) {
 QSPdfOperator *qop=dynamic_cast<QSPdfOperator*>(op);
 QSPdfOperator *qprev=dynamic_cast<QSPdfOperator*>(prev);
 obj->push_back(qop->get(),qprev->get());
}

/** Call PdfOperator::putBehind() */
void QSPdfOperator::putBehind(QSPdfOperator *op) {
 pdfobjects::PdfOperator::putBehind(obj,op->get());
}

/** putBehind: QSA bug workaround */
void QSPdfOperator::putBehind(QObject *op) {
 QSPdfOperator *qop=dynamic_cast<QSPdfOperator*>(op);
 if (!qop) return; //Invalid parameter
 pdfobjects::PdfOperator::putBehind(obj,qop->get());
}

/** get Pdf Operator shared pointer held inside this class. Not exposed to scripting */
boost::shared_ptr<PdfOperator> QSPdfOperator::get() {
 return obj;
}

/**
 Return content stream in which this operator is contained.
 May return NULL
 (if operator is not contained in any content stream or if content stream is not known)
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
