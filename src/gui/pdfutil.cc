/** @file
 Various utility functions related to kernel
*/
#include "pdfutil.h"
#include <qstring.h>
#include <qobject.h>
#include <cobject.h>

namespace util {

using namespace std;
using namespace pdfobjects;

/** Return human-readable (and possibly localized) name of given type
 @param typ Type of PDF Object
 @return Human readable type name
 */
QString getTypeName(PropertyType typ) {
 switch (typ) {
  case pNull:   return QObject::tr("Null",	"Type name"); 
  case pBool:   return QObject::tr("Bool",	"Type name"); 
  case pInt:    return QObject::tr("Int",	"Type name"); 
  case pReal:   return QObject::tr("Real",	"Type name"); 
  case pString: return QObject::tr("String",	"Type name");
  case pName:   return QObject::tr("Name",	"Type name");
  case pRef:    return QObject::tr("Ref",	"Type name");
  case pArray:  return QObject::tr("Array",	"Type name"); 
  case pDict:   return QObject::tr("Dict",	"Type name"); 
  case pStream: return QObject::tr("Stream",	"Type name"); 
// Debug types
  case pOther:    return QObject::tr("DebugCmd",	"Type name");
  case pOther1:  return QObject::tr("DebugError",	"Type name");
  case pOther2:    return QObject::tr("DebugEOF",	"Type name");
  case pOther3:   return QObject::tr("DebugNone",	"Type name");
 }
 //Unknown type. Should not happen
 assert(0);
 return "?";
}

/** Get value (rerference target) of CRef
 @param CRef reference object
*/
IndiRef getRef(IProperty *ref) {
 assert(ref->getType()==pRef);
 IndiRef iref;
 ((CRef*)ref)->getPropertyValue(iref);
 return iref;
}

} // namespace util
