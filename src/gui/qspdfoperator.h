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
#ifndef __QSPDFOPERATOR_H__
#define __QSPDFOPERATOR_H__

#include <qvariant.h>
#include <qobject.h>
#include <kernel/pdfoperators.h>
#include "qscobject.h"
class QString;

namespace gui {

class QSContentStream;
class QSPdfOperatorIterator;
class QSPdfOperatorStack;
class QSIPropertyArray;

using namespace pdfobjects;

/*= This type of object represents pdf operator in content stream */
/** \brief QObject wrapper around PdfOperator */
class QSPdfOperator : public QSCObject {
 Q_OBJECT
public:
 QSPdfOperator(BaseCore *_base);
 QSPdfOperator(boost::shared_ptr<PdfOperator> op,BaseCore *_base);
 QSPdfOperator(boost::shared_ptr<PdfOperator> op,boost::shared_ptr<CContentStream> cs,BaseCore *_base);
 virtual ~QSPdfOperator();
 boost::shared_ptr<PdfOperator> get();
public slots:
 /*-
  Return true, if this object is equal to specified object (i.e. if they internally point to the same item)
  Reference is compared, not the value of objects
 */
 bool equals(QObject* otherObject);
 /*-
  Return bounding box (rectangle) of this PDF operator
 */
 QVariant getBBox (); /*Variant=double[4]*/
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
  Return PDF Graphical Operator iterator, initialialized from this operator
  Graphical iterator iterate only through Graphical operators in content stream
 */
 QSPdfOperatorIterator* graphicalIterator(bool forwardDir=true);
 /*-
  Return PDF Inline Image Operator iterator, initialialized from this operator
  Inline Image iterator iterate only through inline image operators in content stream
 */
 QSPdfOperatorIterator* inlineImageIterator(bool forwardDir=true);
 /*-
  Return PDF Text Operator iterator, initialialized from this operator
  Text iterator iterate only through text operators in content stream
 */
 QSPdfOperatorIterator* textIterator(bool forwardDir=true);
 /*-
  Return PDF Changeable Operator iterator, initialialized from this operator
  Font iterator iterate only through changeable operators in content stream
 */
 QSPdfOperatorIterator* changeableIterator(bool forwardDir=true);
 /*-
  Return PDF Stroking Operator iterator, initialialized from this operator
  Font iterator iterate only through stroking operators in content stream
 */
 QSPdfOperatorIterator* strokingIterator(bool forwardDir=true);
 /*-
  Return PDF Non stroking Operator iterator, initialialized from this operator
  Font iterator iterate only through non-stroking operators in content stream
 */
 QSPdfOperatorIterator* nonStrokingIterator(bool forwardDir=true);
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

 // TODO place to somehow better place or leave it here?
 /*-
  Returns encoded text from text operator. Returns an empty string 
  for all other operators.
  */
 QString getEncodedText();

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
 //TODO: what is this function exactly doing?
 /*-
  Set next operator
 */
 void setNext(QSPdfOperator *op);
 void setNext(QObject *op);
 //TODO: what is this function exactly doing?
 /*-
  Set previous operator
 */
 void setPrev(QSPdfOperator *op);
 void setPrev(QObject *op);
 /*-
  Clone this object
 */
 QSPdfOperator* clone();
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
