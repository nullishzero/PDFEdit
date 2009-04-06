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
 QObject wrapper around array of IProperties<br>
 See doc/user/scripting.xml for more informations about these functions
 @author Martin Petricek
*/

#include "qsipropertyarray.h"
#include "qsimporter.h"
#include <kernel/cobject.h>
#include "qsiproperty.h"

namespace gui {

using namespace pdfobjects;

/**
 Construct wrapper with empty array
 @param _base Scripting base
 */
QSIPropertyArray::QSIPropertyArray(BaseCore *_base) : QSCObject ("IPropertyArray",_base) {
}

/** destructor */
QSIPropertyArray::~QSIPropertyArray() {
}

/**
 get one IProperty item held inside this class.
 @param index Zero-based index of item to get
 @return specified IProperty
*/
boost::shared_ptr<IProperty> QSIPropertyArray::get(int index) {
 return obj[index];
}

/**
 append one IProperty to end of array.
 @param prop IProperty to append
*/
void QSIPropertyArray::append(boost::shared_ptr<IProperty> prop) {
 obj.push_back(prop);
}

/**
 append one IProperty to end of array.
 @param prop IProperty to append
*/
void QSIPropertyArray::append(QSIProperty *prop) {
 obj.push_back(prop->get());
}

/** Delete all elements in the array */
void QSIPropertyArray::clear() {
 obj.clear();
}

/**
 append one IProperty to end of array.
 QSA-bugfix version
 @param obj IProperty to append
*/
void QSIPropertyArray::append(QObject *obj) {
 QSIProperty *prop=dynamic_cast<QSIProperty*>(obj);
 if (prop) {
  append(prop);
 }
}

/** Return number of items inside this array */
int QSIPropertyArray::count() {
 return obj.size();
}

/**
 Copy all items to Operands that can be used in PDF Operators
 @param oper Operands container that will be cleared and filled with operands from this array
 */
void QSIPropertyArray::copyTo(PdfOperator::Operands &oper) {
 oper.clear();
 std::copy(obj.begin(),obj.end(),std::back_inserter(oper));
}

/**
 get one IProperty item held inside this class.
 @param index Zero-based index of item to get
 @return specified IProperty
*/
QSCObject* QSIPropertyArray::property(int index) {
 //Check if we are in bounds
 if (index<0) return NULL;
 if (index>=(int)obj.size()) return NULL;
 return QSImporter::createQSObject(obj[index],base);
}

} // namespace gui
