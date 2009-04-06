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
 QObject wrapper around CContentStream (content stream contained in a page)<br>
 Export some functions to scripting.<br>
 See doc/user/scripting.xml or kernel documentation for more informations about these functions
 @author Martin Petricek
*/

#include "qscontentstream.h"
#include "qspdfoperator.h"
#include "qspdfoperatoriterator.h"
#include <kernel/pdfoperators.h>
#include "util.h"

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
 Return true, if this contentstream is equal to given object (i.e. if objects inside the wrapper are the same)
 @param otherObject object to compare with this one
 @return True if the both objects hold the same item, false otherwise
*/
bool QSContentStream::equals(QObject* otherObject) {
 QSContentStream * other=dynamic_cast<QSContentStream*>(otherObject);
 if (!other) return false;	//It's not even IProperty ...
 return obj==other->get();
}

/**
 Return text representation of Content Stream
 \see CContentStream::getStringRepresentation
 @return string representation
*/
QString QSContentStream::getText() {
 std::string text;
 obj->getStringRepresentation(text);
 return util::convertToUnicode(text,util::PDF);
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
 try {
  obj->replaceOperator(oldOp->get(),newOp->get(),indicateChange);
 } catch (ReadOnlyDocumentException &e) {
  base->errorException("ContentStream","replace",QObject::tr("Document is read-only"));
 }
}

/**
 QSA bugfix version
 \copydoc replace(QSPdfOperator*,QSPdfOperator*,bool)
*/
void QSContentStream::replace(QObject* oldOp,QObject* newOp,bool indicateChange/*=true*/) {
 QSPdfOperator* _oldOp=qobject_cast<QSPdfOperator*>(oldOp,"replace",1,"PdfOperator");
 QSPdfOperator* _newOp=qobject_cast<QSPdfOperator*>(newOp,"replace",2,"PdfOperator");
 if (!(_oldOp && _newOp)) return;
 try {
  obj->replaceOperator(_oldOp->get(),_newOp->get(),indicateChange);
 } catch (ReadOnlyDocumentException &e) {
  base->errorException("ContentStream","replace",QObject::tr("Document is read-only"));
 }
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
 try {
  obj->deleteOperator(op->get(),indicateChange);
 } catch (ReadOnlyDocumentException &e) {
  base->errorException("ContentStream","deleteOperator",QObject::tr("Document is read-only"));
 }
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
 try {
  obj->insertOperator(op->get(),newOp->get(),indicateChange);
 } catch (ReadOnlyDocumentException &e) {
  base->errorException("ContentStream","insertOperator",QObject::tr("Document is read-only"));
 }
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
 try {
  insertOperator((QSPdfOperator*)qop,(QSPdfOperator*)qopNew,indicateChange);
 } catch (ReadOnlyDocumentException &e) {
  base->errorException("ContentStream","insertOperator",QObject::tr("Document is read-only"));
 }
}

/**
 Save changes to underlying content stream
 \see CContentStream::saveChange
*/
void QSContentStream::saveChange() {
 try {
  obj->saveChange();
 } catch (ReadOnlyDocumentException &e) {
  base->errorException("ContentStream","saveChange",QObject::tr("Document is read-only"));
 }
}

/**
 Return first operator in this contentstream.
 @return If not contains any operator, return NULL.
*/
QSPdfOperator * QSContentStream::getFirstOperator() {
	std::vector< boost::shared_ptr< PdfOperator > >	ops;
	obj->getPdfOperators( ops );
	if (ops.size() > 0)
		return new QSPdfOperator( ops.front(), obj, base );
	// else
	return NULL;
}
/**
 Return last operator in this contentstream.
 @return If not contains any operator, return NULL.
*/
QSPdfOperator * QSContentStream::getLastOperator() {
	std::vector< boost::shared_ptr< PdfOperator > >	ops;
	obj->getPdfOperators( ops );
	if (ops.size() > 0)
		return new QSPdfOperator( pdfobjects::getLastOperator(ops.back()), obj, base );
	// else
	return NULL;
}
/**
 Check if contentstream contains some operator.
 @return Return false if conntentstream contains some operator. Otherwise return true.
*/
bool QSContentStream::isEmpty() {
	std::vector< boost::shared_ptr< PdfOperator > >	ops;
	obj->getPdfOperators( ops );
	return (ops.size() == 0);
}

} // namespace gui
