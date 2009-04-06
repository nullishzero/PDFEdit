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
#ifndef __QSIPROPERTYARRAY_H__
#define __QSIPROPERTYARRAY_H__

#include "qscobject.h"
#include <kernel/cobject.h>
#include <stdlib.h>
#include <kernel/pdfoperators.h>

namespace gui {

class QSIProperty;

using namespace pdfobjects;

/*=
 This type represents arbitrary array of IProperty items
 It hold zero or more values indexed by positive integer, starting from zero.
 Values can be of any type, either simple types (int, bool, float, string)
 or complex types (Dict, Array)
*/
/** \brief QObject wrapper around array of arbitrary IProperty items */
class QSIPropertyArray : public QSCObject {
 Q_OBJECT
public:
 QSIPropertyArray(BaseCore *_base);
 virtual ~QSIPropertyArray();
 boost::shared_ptr<IProperty> get(int index);
 void append(boost::shared_ptr<IProperty> prop);
 void copyTo(PdfOperator::Operands &oper);
public slots:
 /*- append one IProperty element to end of array. */
 void append(QSIProperty *prop);
 void append(QObject *obj);
 /*- Delete all elements in the array */
 void clear();
 /*- Return size of this array (number of elements) */
 int count();
 /*- Get IProperty element with given index from this array */
 QSCObject* property(int index);
private:
 /** Vector holding parameters */
 std::vector<boost::shared_ptr<IProperty> > obj;
};

} // namespace gui

#endif
