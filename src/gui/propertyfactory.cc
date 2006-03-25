/** @file
PropertyFactory - static function that will return correct property widget based on type of object
*/

#include "propertyfactory.h"
#include "stringproperty.h"
#include "intproperty.h"
#include "boolproperty.h"
#include "realproperty.h"

/** Return one of the subclasses of Property widget for editing given property.
    If property is of unknown or uneditable type, NULL is returned
 @param prop IProperty to create property from
 @param parent Parent widget
 @param name Name of this property
 @param flags Flags of this property items (default 0)
 @return subclass of Property widget (or NULL)
 */
Property* propertyFactory(IProperty *prop,const QString &_name/*=0*/,QWidget *parent/*=0*/, PropertyFlags _flags/*=0*/) {
 PropertyType typ=prop->getType();
 switch (typ) {
  case pNull:   return NULL;
  case pBool:   return new BoolProperty(_name,parent,_flags); 
  case pInt:    return new IntProperty(_name,parent,_flags); 
  case pReal:   return new RealProperty(_name,parent,_flags); 
  case pString: return new StringProperty(_name,parent,_flags);
  case pName:   return NULL;//TODO
  case pRef:    return NULL;//TODO
  case pArray:  return NULL;//TODO
  case pDict:   return NULL;//TODO
  case pStream: return NULL;//TODO
// Debug types - uneditable, not shown
  case pOther:  return NULL;
  case pOther1: return NULL;
  case pOther2: return NULL;
  case pOther3: return NULL;
 }
 //Unknown type. Should not happen
 assert(0);
 return NULL;
}

