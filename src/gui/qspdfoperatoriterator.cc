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
 QObject wrapper around PdfOperator Iterator<br>
 Export some functions to scripting.<br>
 See doc/user/scripting.xml or kernel documentation for more informations about these functions
 @author Martin Petricek
*/

#include "qspdfoperatoriterator.h"
#include "qspdfoperator.h"
#include "qsimporter.h"
#include "qscontentstream.h"
#include "qsiproperty.h"
#include "nullpointerexception.h"
#include <kernel/ccontentstream.h>
#include <utils/iterator.h>

namespace gui {

using namespace pdfobjects;

/**
 Construct wrapper around new Iterator with given Pdf Operator used as initial item
 @param op PDF Operator
 @param _base scripting base
 */
QSPdfOperatorIterator::QSPdfOperatorIterator(boost::shared_ptr<PdfOperator> op,BaseCore *_base) : QSCObject ("PdfOperatorIterator",_base) {
 obj=new PdfOperator::Iterator(PdfOperator::getIterator(op));
}

/**
 Construct wrapper around new Iterator with given Pdf Operator used as initial item
 and with reference to content stream in which the operator resides
 @param op PDF Operator
 @param cs CContentstream in which this PdfOperator is contained
 @param _base scripting base
*/
QSPdfOperatorIterator::QSPdfOperatorIterator(boost::shared_ptr<PdfOperator> op,boost::shared_ptr<CContentStream> cs,BaseCore *_base) : QSCObject ("PdfOperatorIterator",_base) {
 obj=new PdfOperator::Iterator(PdfOperator::getIterator(op));
 csRef=cs;
 csCheck();
}

/**
 Construct wrapper around new Iterator
 and with reference to content stream in which the iterator resides
 This class takes ownership of the iterator (and will free it once it is destroyed)
 @param op PDF Operator iterator
 @param cs CContentstream in which this PdfOperator is contained
 @param _base scripting base
*/
QSPdfOperatorIterator::QSPdfOperatorIterator(PdfOperator::Iterator *op,boost::shared_ptr<CContentStream> cs,BaseCore *_base) : QSCObject ("PdfOperatorIterator",_base) {
 assert(op);
 obj=op;
 csRef=cs;
}

/**
 Check if the csRef point to the same contentstream,
 as the content stream operator thinks it is in<br>
 Reset the csRef if these two do not match
*/
void QSPdfOperatorIterator::csCheck() {
 if (!obj->valid()) {
  //We are at invalid positon, so we do not know the contentstream anyway
  csRef.reset();
  return;
 }
 assert(obj->getCurrent());
 if(obj->getCurrent()->getContentStream()!=csRef) {
  //The stream is invalid, set it to NULL rather than to invalid stream
  csRef.reset();
 }
}

/** destructor */
QSPdfOperatorIterator::~QSPdfOperatorIterator() {
 assert(obj);
 delete obj;
}

/**
 Return current operator from the iterator
 @return Current operator
*/
QSPdfOperator* QSPdfOperatorIterator::current() {
 if (!obj->valid()) return NULL; //We are at invalid positon
 csCheck();
 //If contentstream is not known, empty reference will be passed this way
 return new QSPdfOperator(obj->getCurrent(),csRef,base);
}

/**
 Duplicate the operator iterator, while trying to preserve its type
 @param src source iterator
 @return new iterator of same type aas source iterator, initially pointing at the same element
*/
PdfOperator::Iterator* QSPdfOperatorIterator::copyIterator(PdfOperator::Iterator *src) {
 assert(src);
 //TODO: this is not the best, but should be sufficient
 //Try to duplicate as TextOperatorIterator
 TextOperatorIterator *newOpT=dynamic_cast<TextOperatorIterator*>(src);
 if (newOpT) return new TextOperatorIterator(*newOpT);
 //Try to duplicate as FontOperatorIterator
 FontOperatorIterator *newOpF=dynamic_cast<FontOperatorIterator*>(src);
 if (newOpF) return new FontOperatorIterator(*newOpF);
 //Try to duplicate as NonStrokingOperatorIterator
 NonStrokingOperatorIterator *newOpN=dynamic_cast<NonStrokingOperatorIterator*>(src);
 if (newOpN) return new NonStrokingOperatorIterator(*newOpN);
 //Try to duplicate as StrokingOperatorIterator
 StrokingOperatorIterator *newOpS=dynamic_cast<StrokingOperatorIterator*>(src);
 if (newOpS) return new StrokingOperatorIterator(*newOpS);
 //Try to duplicate as InlineImageOperatorIterator
 InlineImageOperatorIterator *newOpI=dynamic_cast<InlineImageOperatorIterator*>(src);
 if (newOpI) return new InlineImageOperatorIterator(*newOpI);
 //Try to duplicate as ChangeableOperatorIterator
 ChangeableOperatorIterator *newOpC=dynamic_cast<ChangeableOperatorIterator*>(src);
 if (newOpC) return new ChangeableOperatorIterator(*newOpC);
 //Try to duplicate as GraphicalOperatorIterator
 GraphicalOperatorIterator *newOpG=dynamic_cast<GraphicalOperatorIterator*>(src);
 if (newOpG) return new GraphicalOperatorIterator(*newOpG);
 //Fallback to standard iterator
 return new PdfOperator::Iterator(*src);
}

/**
 Copy this iterator and return new iterator pointing initially to same PDF Operator
 @return New iterator pointing to same item.
*/
QSPdfOperatorIterator* QSPdfOperatorIterator::copy() {
 PdfOperator::Iterator *newOp=copyIterator(obj);
 csCheck();
 //If contentstream is not known, empty reference will be passed this way
 return new QSPdfOperatorIterator(newOp,csRef,base);
}

/**
 Call PdfOperator::Iterator::next()
 Move iterator to next operator
 @return Operator at new position, or NULL if the new position is already invalid
*/
QSPdfOperatorIterator* QSPdfOperatorIterator::next() {
 try {
  obj->next();
  //if we are after last valid, return NULL
  if (obj->isEnd()) return NULL;
  csCheck();
  return new QSPdfOperatorIterator(copyIterator(obj),csRef,base);
 } catch (iterator::IteratorInvalidObjectException &e) {
  //Already at invalid position
  return NULL; ///a.k.a. false ...
 }
}

/**
 Call PdfOperator::Iterator::prev()
 Move iterator to previous operator
 @return Operator at new position, or NULL if the new position is already invalid
*/
QSPdfOperatorIterator* QSPdfOperatorIterator::prev() {
 try {
  obj->prev();
  //if we are before first valid, return NULL
  if (obj->isBegin()) return NULL;
  csCheck();
  return new QSPdfOperatorIterator(copyIterator(obj),csRef,base);
 } catch (iterator::IteratorInvalidObjectException &e) {
  //Already at invalid position
  return NULL; ///a.k.a. false ...
 }
}

/**
 Call PdfOperator::Iterator::isEnd()
 Are we at end of the list?
*/
bool QSPdfOperatorIterator::isEnd() {
 return obj->isEnd();
}

/**
 Call PdfOperator::Iterator::isBegin()
 Are we at beginning of the list?
*/
bool QSPdfOperatorIterator::isBegin() {
 return obj->isBegin();
}

/**
 Call PdfOperator::Iterator::isBegin()
 Is current position valid?
*/
bool QSPdfOperatorIterator::valid() {
 return obj->valid();
}

/**
 Get Pdf Operator iterator held inside this class.
 Not exposed to scripting
 @return PDF Operator Iterator
*/
PdfOperator::Iterator* QSPdfOperatorIterator::get() {
 return obj;
}

/**
 Get Pdf Operator shared pointer - the current item from iterator held inside this class.
 Not exposed to scripting
 @return PDF Operator - current item
*/
boost::shared_ptr<PdfOperator> QSPdfOperatorIterator::getCurrent() {
 if (!obj->valid()) {
  //We are at invalid position
  guiPrintDbg(debug::DBG_ERR,"obj->valid() == false! -> exception");
  throw NullPointerException("QSPdfOperatorIterator","getCurrent");
 }
 return obj->getCurrent();
}

/**
 Return content stream in which the initial operator used to construct the iterator was contained.
 May return NULL
 (if operator is not contained in any content stream or if content stream is not known at time of creation)
 @return QObject wrapper around this operator's content stream
*/
QSContentStream* QSPdfOperatorIterator::stream() {
 csCheck();
 if (!csRef.get()) {
  //ContentStream is not known, so we can't return it
  return NULL;
 }
 return new QSContentStream(csRef,base);
}

} // namespace gui
