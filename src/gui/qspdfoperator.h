#ifndef __QSPDFOPERATOR_H__
#define __QSPDFOPERATOR_H__

#include <qobject.h>
#include <pdfoperators.h>
#include "qscobject.h"
class QString;

namespace gui {

class Base;
class QSContentStream;
class QSPdfOperatorIterator;
class QSPdfOperatorStack;
class QSIPropertyArray;

using namespace pdfobjects;

/*= This type of object represents pdf operator in content stream */
/** QObject wrapper around PdfOperator */
class QSPdfOperator : public QSCObject {
 Q_OBJECT
public:
 QSPdfOperator(Base *_base);
 QSPdfOperator(boost::shared_ptr<PdfOperator> op,Base *_base);
 QSPdfOperator(boost::shared_ptr<PdfOperator> op,boost::shared_ptr<CContentStream> cs,Base *_base);
 virtual ~QSPdfOperator();
 boost::shared_ptr<PdfOperator> get();
public slots:
 /*- Return last operator if a this operator is a composite */
 QSPdfOperator* getLastOperator();
 /**
  Return true, if operator inside this wrapper is empty (NULL) operator.
  In that case, most operations of this operator will fail, throwing
  an exception.

  The only methods that are guaranteed to succeed in that case are
  isEmpty(), textIterator() and iterator() and also you can pass this empty operator
  as parameter to some functions.
 */
 bool isEmpty();
 /*-
  Tries to find first non stroking operator.
  (some operators are modified by stroking operators, some by nonestroking)
  Return true if non stroking operator found, false otherwise.
 */
 bool containsNonStrokingOperator();
 /*-
  Tries to find first stroking operator.
  (some operators are modified by stroking operators, some by nonestroking)
  Return true if stroking operator found, false otherwise.
 */
 bool containsStrokingOperator();
 /*- Return PDF Operator iterator, initially pointing at this operator */
 QSPdfOperatorIterator* iterator();
 /*-
  Return PDF Text Operator iterator, initialialized from this operator
  Text iterator iterate only through text operators in content stream
 */
 QSPdfOperatorIterator* textIterator(bool forwardDir=true);
 /*-
  Return PDF Font Operator iterator, initialialized from this operator
  Font iterator iterate only through font operators in content stream
 */
 QSPdfOperatorIterator* fontIterator(bool forwardDir=true);
 /*- Returns stack with all child operators */
 QSPdfOperatorStack* childs();
 /*- Returns number of child operators under this pdf operator */
 int childCount();
 /*- Return text representation of this pdf operator */
 QString getText();
 /*- Return name of this pdf operator */
 QString getName();
 /*- Returns parameters of this operator in array */
 QSIPropertyArray* params();
 /*- Returns number of parameters in this pdf operator */
 int paramCount();
 /*-
  Add an operator oper to the end of composite operator prev
  The operator will be added after operator prev.
  Second parameter is optional and will default to null operator if not specified
 */
 void pushBack(QSPdfOperator *op,QSPdfOperator *prev=NULL);
 void pushBack(QObject *op,QObject *prev=NULL);
 /*-
  Remove this PDF operator from its ContentStream.
  After calling this function, this object became invalid and must not be used further,
  doing so may result in an exception
 */
 void remove();
 /*-
  Set next operator
 TODO: what is this function exactly doing?
 */
 void setNext(QSPdfOperator *op);
 void setNext(QObject *op);
 /*-
  Set previous operator
 TODO: what is this function exactly doing?
 */
 void setPrev(QSPdfOperator *op);
 void setPrev(QObject *op);
 /*-
  Return content stream in which this operator is contained
  May return NULL if the stream is not known or if this operator is not contained in any content stream
 */
 QSContentStream* stream();
private:
 void csCheck();
private:
 /** Object held in class*/
 boost::shared_ptr<PdfOperator> obj;
 /** Reference to content stream that is holding this operator. It may be NULL (empty shared_ptr) if unknown */
 boost::shared_ptr<CContentStream> csRef;
};

} // namespace gui

#endif
