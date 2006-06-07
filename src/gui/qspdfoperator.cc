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
#include <qstring.h>

namespace gui {

using namespace pdfobjects;

/**
 Construct wrapper with given PdfOperator
 @param op PDF Operator
 @param _base scripting base
 */
QSPdfOperator::QSPdfOperator(boost::shared_ptr<PdfOperator> op,BaseCore *_base) : QSCObject ("PdfOperator",_base) {
 obj=op;
}

/**
 Construct wrapper with empty PdfOperator (NULL)
 @param _base scripting base
 */
QSPdfOperator::QSPdfOperator(BaseCore *_base) : QSCObject ("PdfOperator",_base) {
}

/**
 Construct wrapper with given PdfOperator and reference to its content stream
 @param op PDF Operator
 @param cs CContentstream in which this PdfOperator is contained
 @param _base scripting base
*/
QSPdfOperator::QSPdfOperator(boost::shared_ptr<PdfOperator> op,boost::shared_ptr<CContentStream> cs,BaseCore *_base) : QSCObject ("PdfOperator",_base) {
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
 if (nullPtr(obj,"getText")) return QString::null;
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
 @param forwardDir Direction of traversing the operators for first valid item.
 @return new text iterator
*/
QSPdfOperatorIterator* QSPdfOperator::textIterator(bool forwardDir/*=true*/) {
 TextOperatorIterator* opText=new TextOperatorIterator(PdfOperator::getIterator<TextOperatorIterator>(obj,forwardDir));
 csCheck();
 return new QSPdfOperatorIterator(opText,csRef,base);
}

/** 
 Return true, if this operator is equal to given object (i.e. if objects inside the wrapper are the same)
 @param otherObject object to compare with this one
 @return True if the both objects hold the same item, false otherwise
*/
bool QSPdfOperator::equals(QObject* otherObject) {
 QSPdfOperator* other=dynamic_cast<QSPdfOperator*>(otherObject);
 if (!other) return false;	//It's not even IProperty ... 
 return obj==other->get();
}

/** 
 Create new font operator iterator from this PDF operator.
 The iterator will be initialized from this item 
 @param forwardDir Direction of traversing the operators for first valid item.
 @return new font iterator
*/
QSPdfOperatorIterator* QSPdfOperator::fontIterator(bool forwardDir/*=true*/) {
 FontOperatorIterator* opFont=new FontOperatorIterator(PdfOperator::getIterator<FontOperatorIterator>(obj,forwardDir));
 csCheck();
 return new QSPdfOperatorIterator(opFont,csRef,base);
}

/** 
 Create new changeable operator iterator from this PDF operator.
 The iterator will be initialized from this item 
 @param forwardDir Direction of traversing the operators for first valid item.
 @return new changeable iterator
*/
QSPdfOperatorIterator* QSPdfOperator::changeableIterator(bool forwardDir/*=true*/) {
 ChangeableOperatorIterator* opChangeable=new ChangeableOperatorIterator(PdfOperator::getIterator<ChangeableOperatorIterator>(obj,forwardDir));
 csCheck();
 return new QSPdfOperatorIterator(opChangeable,csRef,base);
}

/** 
 Create new stroking operator iterator from this PDF operator.
 The iterator will be initialized from this item 
 @param forwardDir Direction of traversing the operators for first valid item.
 @return new stroking iterator
*/
QSPdfOperatorIterator* QSPdfOperator::strokingIterator(bool forwardDir/*=true*/) {
 StrokingOperatorIterator* opStroking=new StrokingOperatorIterator(PdfOperator::getIterator<StrokingOperatorIterator>(obj,forwardDir));
 csCheck();
 return new QSPdfOperatorIterator(opStroking,csRef,base);
}

/** 
 Create new non stroking operator iterator from this PDF operator.
 The iterator will be initialized from this item 
 @param forwardDir Direction of traversing the operators for first valid item.
 @return new non stroking iterator
*/
QSPdfOperatorIterator* QSPdfOperator::nonStrokingIterator(bool forwardDir/*=true*/) {
 NonStrokingOperatorIterator* opNonStroking=new NonStrokingOperatorIterator(PdfOperator::getIterator<NonStrokingOperatorIterator>(obj,forwardDir));
 csCheck();
 return new QSPdfOperatorIterator(opNonStroking,csRef,base);
}

/** 
 Create new graphical operator iterator from this PDF operator.
 The iterator will be initialized from this item 
 @param forwardDir Direction of traversing the operators for first valid item.
 @return new graphical iterator
*/
QSPdfOperatorIterator* QSPdfOperator::graphicalIterator(bool forwardDir/*=true*/) {
 GraphicalOperatorIterator* opGraphical=new GraphicalOperatorIterator(PdfOperator::getIterator<GraphicalOperatorIterator>(obj,forwardDir));
 csCheck();
 return new QSPdfOperatorIterator(opGraphical,csRef,base);
}

/** 
 Create new inline image operator iterator from this PDF operator.
 The iterator will be initialized from this item 
 @param forwardDir Direction of traversing the operators for first valid item.
 @return new inline image iterator
*/
QSPdfOperatorIterator* QSPdfOperator::inlineImageIterator(bool forwardDir/*=true*/) {
 InlineImageOperatorIterator* opInlineImage=new InlineImageOperatorIterator(PdfOperator::getIterator<InlineImageOperatorIterator>(obj,forwardDir));
 csCheck();
 return new QSPdfOperatorIterator(opInlineImage,csRef,base);
}

/**
 Get operator name
 \see PdfOperator::getOperatorName
 @return operator name
*/
QString QSPdfOperator::getName() {
 if (nullPtr(obj,"getName")) return QString::null;
 std::string text;
 obj->getOperatorName(text);
 return text;
}

/**
 Return stack with all child operators
 @return Operator stack
*/
QSPdfOperatorStack* QSPdfOperator::childs() {
 if (nullPtr(obj,"childs")) return NULL;
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
 if (nullPtr(obj,"childCount")) return 0;
 return obj->getChildrenCount();
}

/**
 Return all parameters of this operator
 @return Array with parameters
*/
QSIPropertyArray* QSPdfOperator::params() {
 if (nullPtr(obj,"params")) return NULL;
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
 if (nullPtr(obj,"paramCount")) return 0;
 return obj->getParametersCount();
}

/**
 Set next operator
 \see PdfOperator::setNext
 @param op operator to set as next
*/
void QSPdfOperator::setNext(QSPdfOperator *op) {
 if (nullPtr(obj,"setNext")) return;
 obj->setNext(op->get());
}

/**
 Set previous operator
 \see PdfOperator::setPrev
 @param op operator to set as previous
*/
void QSPdfOperator::setPrev(QSPdfOperator *op) {
 if (nullPtr(obj,"setPrev")) return;
 obj->setPrev(op->get());
}

/**
 QSA bug workaround
 \copydoc setNext(QSPdfOperator*)
*/
void QSPdfOperator::setNext(QObject *op) {
 if (nullPtr(obj,"setNext")) return;
 QSPdfOperator *qop=qobject_cast<QSPdfOperator*>(op,"setNext",1,"PdfOperator");
 if (!qop) return; //Invalid parameter
 obj->setNext(qop->get());
}

/**
 QSA bug workaround
 \copydoc setPrev(QSPdfOperator*)
*/
void QSPdfOperator::setPrev(QObject *op) {
 if (nullPtr(obj,"setPrev")) return;
 QSPdfOperator *qop=qobject_cast<QSPdfOperator*>(op,"setPrev",1,"PdfOperator");
 if (!qop) return; //Invalid parameter
 obj->setPrev(qop->get());
}

/**
 Call containsNonStrokingOperator() on this operator
 @return result
 */
bool QSPdfOperator::containsNonStrokingOperator() {
 if (nullPtr(obj,"containsNonStrokingOperator")) return false;
 return pdfobjects::containsNonStrokingOperator(obj);
}

/**
 Call containsStrokingOperator() on this operator
 @return result
 */
bool QSPdfOperator::containsStrokingOperator() {
 if (nullPtr(obj,"containsStrokingOperator")) return false;
 return pdfobjects::containsStrokingOperator(obj);
}

/**
 Call getLastOperator() on this operator
 @return last operator
*/
QSPdfOperator* QSPdfOperator::getLastOperator() {
 if (nullPtr(obj,"getLastOperator")) return NULL;
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
 if (nullPtr(obj,"remove")) return;
 CContentStream* cStream=obj->getContentStream();
 if (!cStream) {
  //This operator is not in any content stream, so technically, it is already removed from it :)
  return;
 }
 cStream->deleteOperator(obj);
//This is ugly hack anyway. Add the tree reload to gui if really necessary
// treeNeedReload();
}

/**
 Add an operator to the end of composite.
 \see PdfOperator::push_back
 @param op Operator to be added.
 @param prev Operator, after which we should place the added one
*/
void QSPdfOperator::pushBack(QSPdfOperator *op,QSPdfOperator *prev/*=NULL*/) {
 if (nullPtr(obj,"pushBack")) return;
 if (!prev) {
  obj->push_back(op->get());
  return;
 }
 obj->push_back(op->get(),prev->get());
}

/**
 QSA bug workaround version
 \copydoc pushBack(QSPdfOperator *,QSPdfOperator *)
*/
void QSPdfOperator::pushBack(QObject *op,QObject *prev/*=NULL*/) {
 if (nullPtr(obj,"pushBack")) return;
 QSPdfOperator *qop=qobject_cast<QSPdfOperator*>(op,"pushBack",1,"PdfOperator");
 if (!qop) return;
 QSPdfOperator *qprev=dynamic_cast<QSPdfOperator*>(prev);
 if (!prev) {
  obj->push_back(qop->get());
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
