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
#ifndef __QSIPROPERTY_H__
#define __QSIPROPERTY_H__

#include <kernel/iproperty.h>
#include "qscobject.h"
#include <qvariant.h>
class QString;

namespace gui {

using namespace pdfobjects;

/*= This object represent one property in dictionary or array */
/** \brief QObject wrapper around IProperty */
class QSIProperty : public QSCObject {
 Q_OBJECT
public:
 QSIProperty(boost::shared_ptr<IProperty> _ip,BaseCore *_base);
 QSIProperty(QSIProperty &source);
 virtual ~QSIProperty();
 boost::shared_ptr<IProperty> get() const;
public slots:
 /*-
  Return true, if this object is equal to specified object (i.e. if they internally point to the same item)
  Reference is compared, not the value of objects
 */
 bool equals(QObject* otherObject);
 /*- Return value store inside the property, if it is a simple type. For complex types (Array, Dict, Stream), NULL is returned */
 QVariant value();
 /*- Return text representation of this property */
 QString getText();
 /*-
  Return reference to this property, but if the property is a reference, return the reference target.
  This way you will always get dereferenced property for correct manipulation
 */
 QSCObject* ref();
 /*- Get integer representation of this property's value, return 0 if it cannot be represented as integer */
 int getInt();
 /*-
  Get type identifier of this Property.
  Can be one of: Null, Bool, Int, Real, String, Name, Ref, Array, Dict, Stream
 */
 QString getType();
 /*- Get human readable and localized name of type of this Property */
 QString getTypeName();
 /*-
  Set value of this property.
  Work only on Bool, Int, Real, String or Name types,
  automatically converts value if type of property is different than type of parameter
  Will do nothing if called on different types (Dict, Array, etc ...)
 */
 void set(const QString &value);
 /*- Overloaded variant of set method */
 void set(int value);
 /*- Overloaded variant of set method */
 void set(double value);
 /*- Overloaded variant of set method */
 void set(bool value);
protected:
 QSIProperty(boost::shared_ptr<IProperty> _ip, QString _className,BaseCore *_base);
protected:
 /** Object held in class*/
 boost::shared_ptr<IProperty> obj;
};

} // namespace gui

#endif
