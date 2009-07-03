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
 QObject wrapper around CArray<br>
 Export some functions to scripting.<br>
 See doc/user/scripting.xml or kernel documentation for more informations about these functions
 @author Martin Petricek
*/

#include "qsarray.h"
#include "qsimporter.h"
#include <kernel/cobject.h>
#include "pdfutil.h"
#include "util.h"

namespace gui {

using namespace pdfobjects;

//Theoretically, this could be done better with templates, but unfortunately, it is not possible due to some C++ limitations
/** Standard error handling - code added before the operation */
#define OP_BEGIN \
 CArray *array=dynamic_cast<CArray*>(obj.get());\
 try {
/** Standard error handling - code added after the operation */
#define OP_END(func) \
 } catch (ReadOnlyDocumentException &e) { \
  base->errorException("Array",func,QObject::tr("Document is read-only")); \
 } catch (NotImplementedException &e) { \
  base->errorException("Array",func,QObject::tr("Operation not implemented: %1").arg(e.what())); \
 }

/**
 Construct wrapper with given CArray
 @param _array Array
 @param _base scripting base
*/
QSArray::QSArray(boost::shared_ptr<CArray> _array,BaseCore *_base) : QSIProperty (_array,"Array",_base) {
}

/**
 Copy constructor
 @param source Source item
*/
QSArray::QSArray(QSArray &source) : QSIProperty (source.obj,"Array",source.base) {
}

/** destructor */
QSArray::~QSArray() {
}

/**
 get CArray held inside this class. Not exposed to scripting
 @return CArray shared pointer
*/
boost::shared_ptr<CArray> QSArray::get() {
 return boost::dynamic_pointer_cast<CArray>(obj);
}

/**
 Return number of array elements
 \see CArray::getPropertyCount
 @return number of elements
*/
int QSArray::count() {
 CArray *array=dynamic_cast<CArray*>(obj.get());
 return array->getPropertyCount();
}

/**
 Return property at specified index
 \see CArray::getProperty(int)
 @param index Index of element in array
 @return property at given index
*/
QSCObject* QSArray::property(int index) {
 CArray *array=dynamic_cast<CArray*>(obj.get());
 boost::shared_ptr<IProperty> property=array->getProperty(index);
 return QSImporter::createQSObject(property,base);
}

/**
 Delete property at specified index
 \see CArray::delProperty(int)
 @param index Index of element in array
*/
void QSArray::delProperty(int index) {
 OP_BEGIN
  array->delProperty(index);
 OP_END("delProperty")
}

/**
 Return text representation of array
 \see CArray::getStringRepresentation
 @return string representation
*/
QString QSArray::getText() {
 CArray *array=dynamic_cast<CArray*>(obj.get());
 std::string text;
 array->getStringRepresentation(text);
 return util::convertToUnicode(text,util::PDF);
}


/**
 Add given property to array at specified index
 @param index Index of element in array
 @param ip property to add
 \see CArray::addProperty
*/
void QSArray::add(int index,QSIProperty *ip) {
 OP_BEGIN
  array->addProperty(index,*(ip->get().get()));
 OP_END("add")
}

/** \copydoc add(int,QSIProperty *) */
void QSArray::add(int index,QObject *ip) {
 //QSA-bugfix variant of this method
 QSIProperty *ipx=dynamic_cast<QSIProperty*>(ip);
 OP_BEGIN
  if (ipx) array->addProperty(index,*(ipx->get().get()));
 OP_END("add")
}

/**
 Add String property to array at specified index
 @param index Index of element in array
 @param ip string to add
 \see CArray::addProperty
*/
void QSArray::add(int index,const QString &ip) {
 OP_BEGIN
  CString property(util::convertFromUnicode(ip,util::PDF));
  array->addProperty(index,property);
 OP_END("add")
}

/**
 Add Int property to array at specified index
 @param index Index of element in array
 @param ip integer to add
 \see CArray::addProperty
*/
void QSArray::add(int index,int ip) {
 OP_BEGIN
  CInt property(ip);
  array->addProperty(index,property);
 OP_END("add")
}


/**
 Append given property to array
 @param ip property to add
 \see CArray::addProperty
*/
void QSArray::add(QSIProperty *ip) {
 OP_BEGIN
  array->addProperty(*(ip->get().get()));
 OP_END("add")
}

/** \copydoc add(QSIProperty *) */
void QSArray::add(QObject *ip) {
 //QSA-bugfix variant of this method
 QSIProperty *ipx=dynamic_cast<QSIProperty*>(ip);
 OP_BEGIN
  if (ipx) array->addProperty(*(ipx->get().get()));
 OP_END("add")
}

/**
 Append given String property to array
 @param ip string to add
 \see CArray::addProperty
*/
void QSArray::add(const QString &ip) {
 OP_BEGIN
  CString prop(util::convertFromUnicode(ip,util::PDF));
  array->addProperty(prop);
 OP_END("add")
}

/**
 Append given Int property to array
 @param ip integer to add
 \see CArray::addProperty
*/
void QSArray::add(int ip) {
 OP_BEGIN
  CInt prop(ip);
  array->addProperty(prop);
 OP_END("add")
}

/**
 recursive CDict/CArray getProperty(...)
 Will take the name as slash-separated list of childs to traverse to get to target property.
 References on the way are automatically dereferenced
 @param name Path to property
 @return specified property
*/
QSCObject* QSArray::child(const QString &name) {
 try {
  boost::shared_ptr<CArray> array=boost::dynamic_pointer_cast<CArray>(obj);
  boost::shared_ptr<IProperty> property=util::recursiveProperty(array,name);
  return QSImporter::createQSObject(property,base);
 } catch (...) {
  //Some error, probably the property does not exist
  return NULL;
 }
}

} // namespace gui
