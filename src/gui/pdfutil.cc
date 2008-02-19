/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko,
 *                                              Miroslav Jahoda,
 *                                              Jozef Misutka,
 *                                              Martin Petricek
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit
 */
/** @file
 Various utility functions for examining and manipulating pdf objects,
 related to kernel
 \brief PDF manipulation utility functions
 @author Martin Petricek
*/

#include "pdfutil.h"
#include "qtcompat.h"
#include <qstring.h>
#include <qobject.h>
#include <kernel/cobject.h>
#include <kernel/cpdf.h>
#include "util.h"
#include <utils/debug.h>

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

/** Return string identifier of given type
 @param typ Type of PDF Object
 @return Identifier of type name
 */
QString getTypeId(PropertyType typ) {
 switch (typ) {
  case pNull:   return "Null";
  case pBool:   return "Bool";
  case pInt:    return "Int";
  case pReal:   return "Real";
  case pString: return "String";
  case pName:   return "Name";
  case pRef:    return "Ref";
  case pArray:  return "Array";
  case pDict:   return "Dict";
  case pStream: return "Stream";
// Debug types
  case pOther:    return "DebugCmd";
  case pOther1:  return "DebugError";
  case pOther2:    return "DebugEOF";
  case pOther3:   return "DebugNone";
 }
 //Unknown type. Should not happen
 assert(0);
 return "?";
}

/** Return human-readable (and possibly localized) name of given object's type
 @param obj Object to get type from (IProperty)
 @return Human readable type name
 */
QString getTypeName(IProperty *obj) {
 return getTypeName(obj->getType());
}

/** \copydoc getTypeName(IProperty *) */
QString getTypeName(boost::shared_ptr<IProperty> obj) {
 return getTypeName(obj.get());
}

/**
 Return short textual preview of what's in the property
 @param obj property for preview
 @return short string (cca. 20 chararters max)
*/
QString propertyPreview(boost::shared_ptr<IProperty> obj) {
 PropertyType typ=obj->getType();
 switch (typ) {
  case pBool: {
   bool value;
   IProperty::getSmartCObjectPtr<CBool>(obj)->getValue(value);
   return value?"true":"false";
  }
  case pInt: {
   int value;
   IProperty::getSmartCObjectPtr<CInt>(obj)->getValue(value);
   return QString::number(value);
  }
  case pReal: {
   double value;
   IProperty::getSmartCObjectPtr<CReal>(obj)->getValue(value);
   return QString::number(value);
  }
  case pName: {
   std::string value;
   IProperty::getSmartCObjectPtr<CName>(obj)->getValue(value);
   QString ret=util::convertToUnicode(value,util::PDF);
   if (ret.length()>22) {
    ret.truncate(20);
    ret+="...";
   }
   return ret;
  }
  case pString: {
   std::string value;
   IProperty::getSmartCObjectPtr<CString>(obj)->getValue(value);
   QString ret=util::convertToUnicode(value,util::PDF);
   if (ret.length()>22) {
    ret.truncate(20);
    ret+="...";
   }
   return ret;
  }
  case pArray: {
   size_t count = IProperty::getSmartCObjectPtr<CArray>(obj)->getPropertyCount();
   return countString(count,"element","elements");
  }
  case pDict: {
   size_t count = IProperty::getSmartCObjectPtr<CDict>(obj)->getPropertyCount();
   return countString(count,"item","items");
  }
  case pStream: {
   try {
    boost::shared_ptr<IProperty> len=IProperty::getSmartCObjectPtr<CStream>(obj)->getProperty("Length");
    // FIXME: Why this check? It can't be NULL AFAIK
    // dynamic_cast shouldn't be used and IProperty::getSmartCObjectPtr
    // should be used instead
    CInt* lenInt=dynamic_cast<CInt*>(len.get());
    if (!lenInt) return "";
    int iLen;
    lenInt->getValue(iLen);
    return QString::number(iLen)+" "+QObject::tr("bytes");
   } catch (...) {
    //Preview failed
    return "";
   }
  }
//  case pRef: -> Ref can't be target of ref ...
  default:;
 }
 //Not suitable for preview
 return "";
}

/** Return string identifier of given type
 @param obj Object to get type from (IProperty)
 @return Identifier of type name
 */
QString getTypeId(IProperty *obj) {
 return getTypeId(obj->getType());
}

/** \copydoc getTypeId(IProperty *) */
QString getTypeId(boost::shared_ptr<IProperty> obj) {
 return getTypeId(obj.get());
}

/**
 Get value (reference target) of CRef
 @param ref CRef reference object
*/
IndiRef getRef(IProperty *ref) {
 assert(ref->getType()==pRef);
 IndiRef iref;
 CRef* refx=dynamic_cast<CRef*>(ref);
 assert(refx);
 refx->getValue(iref);
 return iref;
}

/** \copydoc getRef(IProperty *) */
IndiRef getRef(boost::shared_ptr<IProperty> ref) {
 return getRef(ref.get());
}

/** Check for validity of reference - if ref is valid reference (have target) in given CPdf
 @param pdf pdf to check for reference
 @param ref Indirect reference to check
 @return true if given reference target exists in given pdf, false otherwise
*/
bool isRefValid(CPdf *pdf,IndiRef ref) {
 CXref *cxref=pdf->getCXref();
 Ref _val;//TODO: why there is no knowsRef(IndiRef) ?
 _val.num=ref.num;
 _val.gen=ref.gen;
 if (!cxref->knowsRef(_val)) { //ref not valid
  guiPrintDbg(debug::DBG_DBG,"Unknown reference!");
  return false;
 }
 return true;
}

/** Return true, if this is simple property (editable as item in property editor and have no children), false otherwise
 @param prop IProperty to check
 @return true if simple property, false otherwise
 */
bool isSimple(IProperty* prop) {
 PropertyType pt=prop->getType();
 switch(pt) {
  case pNull:
  case pBool:
  case pInt:
  case pReal:
  case pName:
  case pString:
   return true;
  default:
   return false;
 }
}

/**
 Return referenced IProperty for given reference, or the property itself in case property is not a reference
 Return NULL (empty shared_ptr) on error
 @param obj Reference to dereference
 @return Referenced IProperty
*/
boost::shared_ptr<IProperty> dereference(boost::shared_ptr<IProperty> obj) {
 if (!obj.get()) return obj;  //Empty pointer
 if (obj->getType()!=pRef) return obj;  //Not a reference
 CPdf* pdf=obj->getPdf();
 // TODO Is this really ok? Shouldn't we create rather CNull here?
 if (!pdf) return boost::shared_ptr<IProperty>(); //Property does not belong to document -> cannot dereference
 IndiRef ref;
 IProperty::getSmartCObjectPtr<CRef>(obj)->getValue(ref);
 boost::shared_ptr<IProperty> rp=pdf->getIndirectProperty(ref);
 return rp;
}

/** Return true, if this is simple property (editable as item in property editor and have no children), false otherwise
 @param prop IProperty to check
 @return true if simple property, false otherwise
 */
bool isSimple(boost::shared_ptr<IProperty> prop) {
 return isSimple(prop.get());
}

/**
 Save currently active revision in document under different filename
 @param obj Pdf document
 @param name New name
 @param error String that will receive error message if saving failed. If null, no message will b e received
 @return true if success, false if failure
*/
bool saveCopy(CPdf *obj, const QString &name, QString *error) {
 FILE *f=fopen(name,"wb");
 if (!f) {
  char *err=strerror(errno);
  if (err) {
   (*error)=QObject::tr("Unable to open file: %1").arg(err);
  } else {
   (*error)=QObject::tr("Unable to open file: %1").arg(errno);
  }
  return false; ///failed to open file
 }
 try {
  obj->clone(f);
  fclose(f);
  return true;
 } catch (NotImplementedException &e) {
  fclose(f);
  (*error)=QObject::tr("Not implemented: %1").arg(e.what());
  return false;
 } catch (...) {
  fclose(f);
  (*error)=QObject::tr("Unknown error");
  return false;
 }
}

/**
 recursive CDict/CArray getProperty(...)
 Will take the name as slash-separated list of childs to traverse to get to target property.
 References on the way are automatically dereferenced
 Can throw exception if some property is not found on the way
 @param obj CDict/CArray to start the search
 @param name Path to property
 @return specified property
*/
template <typename T>
boost::shared_ptr<IProperty> _recursiveProperty(boost::shared_ptr<T> obj,const QString &name) {
 QString nameFirst=name.section('/',0,0);
 QString nameEnd=name.section('/',1);
 guiPrintDbg(debug::DBG_DBG,"Recurse '" << Q_OUT(name) << "' -> '" << Q_OUT(nameFirst) << "' / '" << (nameEnd.isNull()?"(NULL)":Q_OUT(nameEnd)) << "'");
 boost::shared_ptr<IProperty> r=getObjProperty(obj,nameFirst);
 r=dereference(r);
 if (nameEnd.isNull()) return r;//This is the property we want
 if (nameEnd=="") return r;//This is the property we want

 //Next is dict, rearch for rest there
 boost::shared_ptr<CDict> dict=boost::dynamic_pointer_cast<CDict>(r);
 if (dict.get()) return recursiveProperty(dict,nameEnd);

 //Next is array, rearch for rest there
 boost::shared_ptr<CArray> array=boost::dynamic_pointer_cast<CArray>(r);
 if (array.get()) return recursiveProperty(array,nameEnd);

 //Next is some simple property, we can't search there.
 throw ElementNotFoundException("","");
}

/** \copydoc _recursiveProperty */
boost::shared_ptr<IProperty> recursiveProperty(boost::shared_ptr<CDict> obj,const QString &name) {
 guiPrintDbg(debug::DBG_DBG,"Recurse Dict " << Q_OUT(name));
 return _recursiveProperty(obj,name);
}

/** \copydoc _recursiveProperty */
boost::shared_ptr<IProperty> recursiveProperty(boost::shared_ptr<CArray> obj,const QString &name) {
 guiPrintDbg(debug::DBG_DBG,"Recurse Array " << Q_OUT(name));
 return _recursiveProperty(obj,name);
}

/**
 Get property from Dict based in QString key
 @param obj Dict to get property from
 @param name name of property
 @return specified property
*/
boost::shared_ptr<IProperty> getObjProperty(boost::shared_ptr<CDict> obj,const QString &name) {
 std::string theName=convertFromUnicode(name,util::PDF);
 guiPrintDbg(debug::DBG_DBG,"getObjDict " << Q_OUT(name));
 return obj->getProperty(theName);
}

/**
 Get property from Array based in QString key
 @param obj Array to get property from
 @param name name of property (must be convertable to number)
 @return specified property
*/
boost::shared_ptr<IProperty> getObjProperty(boost::shared_ptr<CArray> obj,const QString &name) {
 bool ok=false;
 int index=name.toInt(&ok);
 guiPrintDbg(debug::DBG_DBG,"getObjArray " << Q_OUT(name) << " -> " << index);
 if (!ok) {
  //Not a number;
  throw ElementNotFoundException("","");
 }
 return obj->getProperty(index);
}

/**
 Return type identifier of annotation type
 @param at Annotation type
*/
QString annotType(CAnnotation::AnnotType at) {
 switch(at) {
  case CAnnotation::Text:            return QT_TRANSLATE_NOOP("annotation_type","Text"           );
  case CAnnotation::Link:            return QT_TRANSLATE_NOOP("annotation_type","Link"           );
  case CAnnotation::FreeText:        return QT_TRANSLATE_NOOP("annotation_type","FreeText"       );
  case CAnnotation::Line:            return QT_TRANSLATE_NOOP("annotation_type","Line"           );
  case CAnnotation::Square:          return QT_TRANSLATE_NOOP("annotation_type","Square"         );
  case CAnnotation::Circle:          return QT_TRANSLATE_NOOP("annotation_type","Circle"         );
  case CAnnotation::Polygon:         return QT_TRANSLATE_NOOP("annotation_type","Polygon"        );
  case CAnnotation::PolyLine:        return QT_TRANSLATE_NOOP("annotation_type","PolyLine"       );
  case CAnnotation::Highlight:       return QT_TRANSLATE_NOOP("annotation_type","Highlight"      );
  case CAnnotation::Underline:       return QT_TRANSLATE_NOOP("annotation_type","Underline"      );
  case CAnnotation::Squiggly:        return QT_TRANSLATE_NOOP("annotation_type","Squiggly"       );
  case CAnnotation::StrikeOut:       return QT_TRANSLATE_NOOP("annotation_type","StrikeOut"      );
  case CAnnotation::Stamp:           return QT_TRANSLATE_NOOP("annotation_type","Stamp"          );
  case CAnnotation::Caret:           return QT_TRANSLATE_NOOP("annotation_type","Caret"          );
  case CAnnotation::Ink:             return QT_TRANSLATE_NOOP("annotation_type","Ink"            );
  case CAnnotation::Popup:           return QT_TRANSLATE_NOOP("annotation_type","Popup"          );
  case CAnnotation::Fileattachement: return QT_TRANSLATE_NOOP("annotation_type","Fileattachement");
  case CAnnotation::Sound:           return QT_TRANSLATE_NOOP("annotation_type","Sound"          );
  case CAnnotation::Movie:           return QT_TRANSLATE_NOOP("annotation_type","Movie"          );
  case CAnnotation::Widget:          return QT_TRANSLATE_NOOP("annotation_type","Widget"         );
  case CAnnotation::Screen:          return QT_TRANSLATE_NOOP("annotation_type","Screen"         );
  case CAnnotation::Printermark:     return QT_TRANSLATE_NOOP("annotation_type","Printermark"    );
  case CAnnotation::Trapnet:         return QT_TRANSLATE_NOOP("annotation_type","Trapnet"        );
  case CAnnotation::Watermark:       return QT_TRANSLATE_NOOP("annotation_type","Watermark"      );
  case CAnnotation::_3D:             return QT_TRANSLATE_NOOP("annotation_type","3D"             );
  case CAnnotation::Unknown:         return QT_TRANSLATE_NOOP("annotation_type","Unknown"        );
 }
 return "?";
}

/**
 Return type identifier of annotation
 @param anot Annotation
*/
QString annotType(boost::shared_ptr<CAnnotation> anot) {
 return annotType(anot->getType());
}

/**
 Return human readable, localized type identifier of annotation
 @param anot Annotation
*/
QString annotTypeName(boost::shared_ptr<CAnnotation> anot) {
 return QObject::tr(annotType(anot->getType()),"annotation_type");
}

} // namespace util
