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
 propertyFactory
 - static function that will return correct property widget based on type of object<br>
 Will work only for simple types (bool, int, real, string, name) and ref (reference)<br>
 Will return NULL for Arrays and Dictionaries, as these are not editable in property editor<br>
 @author Martin Petricek
*/

#include <qwidget.h>
#include <kernel/iproperty.h>
#include "propertyfactory.h"
#include "stringproperty.h"
#include "intproperty.h"
#include "boolproperty.h"
#include "realproperty.h"
#include "refproperty.h"
#include "nameproperty.h"

namespace gui {

/**
 Return one of the subclasses of Property widget for editing given property.
 If property is of unknown or uneditable type, NULL is returned
 @param prop IProperty to create property from
 @param parent Parent widget
 @param _name Name of this property
 @return subclass of Property widget (or NULL)
 */
Property* propertyFactory(pdfobjects::IProperty *prop,const QString &_name/*=0*/,QWidget *parent/*=0*/) {
 PropertyType typ=prop->getType();
 PropertyFlags _flags=prop->getMode();
 switch (typ) {
  case pNull:   return NULL;
  case pBool:   return new BoolProperty(_name,parent,_flags);
  case pInt:    return new IntProperty(_name,parent,_flags);
  case pReal:   return new RealProperty(_name,parent,_flags);
  case pString: return new StringProperty(_name,parent,_flags);
  case pName:   return new NameProperty(_name,parent,_flags);
  case pRef:    return new RefProperty(_name,parent,_flags);
  case pArray:  return NULL;//Not editable in property editor, editable in tree window
  case pDict:   return NULL;//Not editable in property editor, editable in tree window
  case pStream: return NULL;//Not editable in property editor, editable in tree window
  // Debug types - All of them uneditable and not shown
  case pOther:
  case pOther1:
  case pOther2:
  case pOther3: return NULL;
 }
 //Unknown type. Should not happen
 assert(0);
 return NULL;
}

} // namespace gui
