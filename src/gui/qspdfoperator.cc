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
/** @file
 QObject wrapper around PdfOperator (one operator found inside content stream)<br>
 Export some functions to scripting.<br>
 See doc/user/scripting.xml or kernel documentation for more informations about these functions
 @author Martin Petricek
*/

#include "qspdfoperator.h"
#include "qtcompat.h"
#include "qspdfoperatorstack.h"
#include "qspdfoperatoriterator.h"
#include "qsimporter.h"
#include "qscontentstream.h"
#include "qsiproperty.h"
#include "qsipropertyarray.h"
#include <kernel/ccontentstream.h>
#include <utils/iterator.h>
#include <qstring.h>
#include "util.h"
#include QLIST

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
 Reset the csRef to the real contentstream
*/
void QSPdfOperator::csCheck() {
 csRef=obj->getContentStream();
}

/** destructor */
QSPdfOperator::~QSPdfOperator() {
}

/**
 Return bounding box of this PDF operator
 @return bounding box
*/
QVariant QSPdfOperator::getBBox () {
 libs::Rectangle br = obj->getBBox ();

 Q_List<QVariant> r;
 r.append( QVariant( br.xleft ) );
 r.append( QVariant( br.yleft ) );
 r.append( QVariant( br.xright ) );
 r.append( QVariant( br.yright ) );

 return QVariant( r );
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
 return util::convertToUnicode(text,util::PDF);
}

QString QSPdfOperator::getEncodedText() {
 std::string text;
 TextSimpleOperator * textOp = dynamic_cast<TextSimpleOperator*>(obj.get());
 if (!textOp)
	 return QString::null;
 textOp->getFontText(text);
 return util::convertToUnicode(text, util::PDF);
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
 The iterator will be initialized from this item.
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
 The iterator will be initialized from this item.
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
 The iterator will be initialized from this item.
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
 The iterator will be initialized from this item.
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
 The iterator will be initialized from this item.
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
 The iterator will be initialized from this item.
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
 return util::convertToUnicode(text,util::PDF);
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
 try {
  obj->setNext(op->get());
 } catch (ReadOnlyDocumentException &e) {
  base->errorException("PdfOperator","setNext",QObject::tr("Document is read-only"));
 }
}

/**
 Set previous operator
 \see PdfOperator::setPrev
 @param op operator to set as previous
*/
void QSPdfOperator::setPrev(QSPdfOperator *op) {
 if (nullPtr(obj,"setPrev")) return;
 try {
  obj->setPrev(op->get());
 } catch (ReadOnlyDocumentException &e) {
  base->errorException("PdfOperator","setPrev",QObject::tr("Document is read-only"));
 }
}

/**
 QSA bug workaround
 \copydoc setNext(QSPdfOperator*)
*/
void QSPdfOperator::setNext(QObject *op) {
 if (nullPtr(obj,"setNext")) return;
 QSPdfOperator *qop=qobject_cast<QSPdfOperator*>(op,"setNext",1,"PdfOperator");
 if (!qop) return; //Invalid parameter
 try {
  obj->setNext(qop->get());
 } catch (ReadOnlyDocumentException &e) {
  base->errorException("PdfOperator","setNext",QObject::tr("Document is read-only"));
 }
}

/**
 QSA bug workaround
 \copydoc setPrev(QSPdfOperator*)
*/
void QSPdfOperator::setPrev(QObject *op) {
 if (nullPtr(obj,"setPrev")) return;
 QSPdfOperator *qop=qobject_cast<QSPdfOperator*>(op,"setPrev",1,"PdfOperator");
 if (!qop) return; //Invalid parameter
 try {
  obj->setPrev(qop->get());
 } catch (ReadOnlyDocumentException &e) {
  base->errorException("PdfOperator","setPrev",QObject::tr("Document is read-only"));
 }
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
 boost::shared_ptr<CContentStream> cStream=obj->getContentStream();
 if (cStream) {
  try {
   cStream->deleteOperator(obj);
  } catch (ReadOnlyDocumentException &e) {
   base->errorException("PdfOperator","remove",QObject::tr("Document is read-only"));
  }
 }
 //This operator is not in any content stream, so technically, it is already removed from it :)
 return;
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
 try {
  if (!prev) {
   obj->push_back(op->get());
   return;
  }
  obj->push_back(op->get(),prev->get());
 } catch (ReadOnlyDocumentException &e) {
  base->errorException("PdfOperator","pushBack",QObject::tr("Document is read-only"));
 }
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
 try {
  if (!prev) {
   obj->push_back(qop->get());
   return;
  }
  obj->push_back(qop->get(),qprev->get());
 } catch (ReadOnlyDocumentException &e) {
  base->errorException("PdfOperator","pushBack",QObject::tr("Document is read-only"));
 }
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

/**
 Clone this object.
 @return Clone of this object.
*/
QSPdfOperator* QSPdfOperator::clone() {
 csCheck();
 return new QSPdfOperator (obj->clone(),base);
}


} // namespace gui
