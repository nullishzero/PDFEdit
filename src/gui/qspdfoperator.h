#ifndef __QSPDFOPERATOR_H__
#define __QSPDFOPERATOR_H__

#include <qstring.h>
#include <qobject.h>
#include <pdfoperators.h>
#include "qscobject.h"

namespace gui {

class Base;
class QSContentStream;
class QSPdfOperatorIterator;

using namespace pdfobjects;

/*= This type of object represents pdf operator in content stream */
class QSPdfOperator : public QSCObject {
 Q_OBJECT
public:
 QSPdfOperator(boost::shared_ptr<PdfOperator> op,Base *_base);
 QSPdfOperator(boost::shared_ptr<PdfOperator> op,boost::shared_ptr<CContentStream> cs,Base *_base);
 virtual ~QSPdfOperator();
 boost::shared_ptr<PdfOperator> get();
public slots:
 /*-
  Returns child operator with given number from this operator
  Use data fetched by loadChilds method, if it wasn't called, it is called before returning the child
 */
 QSPdfOperator* child(int childNumber);
 /*- Return PDF Operator iterator, initially pointing at this operator */
 QSPdfOperatorIterator* iterator();
 /*-
  Return PDF Text Operator iterator, initialialized from this operator
  Text iterator iterate only through text operators in content stream
 */
 QSPdfOperatorIterator* textIterator();
 /*-
  Returns number of child operators under this pdf operator
  Use data fetched by loadChilds method, if it wasn't called, it is called before returning the count
 */
 int childCount();
 /*- Return text representation of this pdf operator */
 QString getText();
 /*- Return name of this pdf operator */
 QString getName();
 /*-
  Get all child operators under this operator and store them.
  Get the operators with child and childCount functions.
  Usually it is not necessary to call this method, as these funtions will call
  it automatically on first need, but you may call it explicitly to reload the
  list of child operators stored in this object
 */
 void loadChilds();
 /*-
  Get all parameters (operands) under this operator and store them.
  Get the operators with param and paramCount functions.
  Usually it is not necessary to call this method, as these funtions will call
  it automatically on first need, but you may call it explicitly to reload the
  list of parameters stored in this object
 */
 void loadParams();
 /*-
  Returns parameter with given number from this operator
  Use data fetched by loadParams method, if it wasn't called, it is called before returning the parameter
 */
 QSCObject* param(int paramNumber);
 /*-
  Returns number of parameters for this pdf operator
  Use data fetched by loadParams method, if it wasn't called, it is called before returning the count
 */
 int paramCount();
 /*-
  Add an operator oper to the end of composite operator prev
  The operator will be added after operator prev.
 */
 void pushBack(QSPdfOperator *op,QSPdfOperator *prev);
 void pushBack(QObject *op,QObject *prev);
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
 /** Object held in class*/
 boost::shared_ptr<PdfOperator> obj;
 /** Reference to content stream that is holding this operator. It may be NULL (empty shared_ptr) if unknown */
 boost::shared_ptr<CContentStream> csRef;
 /** Vector with child operators */
 std::vector<boost::shared_ptr<PdfOperator> > childs;
 /** Number of childs in vector. -1 mean no childs yet parsed */
 int numChilds;
 /** Vector with operator parameters */
 std::vector<boost::shared_ptr<IProperty> > params;
 /** Number of parameters in vector. -1 mean no parameters yet parsed */
 int numParams;
};

} // namespace gui

#endif
