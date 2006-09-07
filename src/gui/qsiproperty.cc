/** @file
 QObject wrapper around IProperty (one property in dictionary or array)<br>
 Export some functions to scripting.<br>
 See doc/user/scripting.xml or kernel documentation for more informations about these functions
 @author Martin Petricek
*/

#include "pdfutil.h"
#include "qsiproperty.h"
#include "util.h"
#include <cobject.h>
#include <qsimporter.h>
#include <qstring.h>
#include <qvaluelist.h>

namespace gui {

using namespace pdfobjects;

/**
 Construct wrapper with given IProperty
 @param _ip IProperty wrapped in this object
 @param _base Scripting base
*/
QSIProperty::QSIProperty(boost::shared_ptr<IProperty> _ip,BaseCore *_base) : QSCObject ("IProperty",_base) {
 assert(_ip.get());
 obj=_ip;
}

/**
 Copy constructor
 @param source Source object to copy from
 */
QSIProperty::QSIProperty(QSIProperty &source) : QSCObject ("IProperty",source.base) {
 obj=source.obj;
}

/**
 Construct wrapper with given IProperty and given type name<br>
 This constructor is available only to subclasses - they can set their own class name here
 @param _ip IProperty wrapped in this object
 @param _className type name of this object
 @param _base Scripting base
*/
QSIProperty::QSIProperty(boost::shared_ptr<IProperty> _ip, QString _className,BaseCore *_base) : QSCObject (_className,_base) {
 obj=_ip;
}

/** 
 Return true, if this property is equal to given object (i.e. if objects inside the wrapper are the same)
 @param otherObject object to compare with this one
 @return True if the both objects hold the same item, false otherwise
*/
bool QSIProperty::equals(QObject* otherObject) {
 QSIProperty* other=dynamic_cast<QSIProperty*>(otherObject);
 if (!other) return false;	//It's not even IProperty ... 
 return obj==other->get();
}

/**
 Return text representation of property
 \see IProperty::getStringRepresentation
 @return string representation
*/
QString QSIProperty::getText() {
 std::string text;
 obj->getStringRepresentation(text);
 return text;
}

/**
 Return Value of property
 @return property value
*/
QVariant QSIProperty::value() {
 CString* str=dynamic_cast<CString*>(obj.get());
 if (str) {
  std::string value;
  str->getValue(value);
  return QString(util::convertToUnicode(value));
 }
 CName* name=dynamic_cast<CName*>(obj.get());
 if (name) {
  std::string value;
  name->getValue(value);
  return QString(util::convertToUnicode(value));
 }
 CInt* tInt=dynamic_cast<CInt*>(obj.get());
 if (tInt) {
  int value;
  tInt->getValue(value);
  return value;
 }
 CBool* ibool=dynamic_cast<CBool*>(obj.get());
 if (ibool) {
  bool value;
  ibool->getValue(value);
  return value;
 }
 CReal* real=dynamic_cast<CReal*>(obj.get());
 if (real) {
  double value;
  real->getValue(value);
  return value;
 }
 CRef* ref=dynamic_cast<CRef*>(obj.get());
 if (ref) {
  IndiRef value;
  ref->getValue(value);
  QValueList<QVariant> val;
  val+=value.num;
  val+=value.gen;
  return val;
 }
 // Dict/Array/Null/Stream ... 
 return QVariant();
}

/**
 Try to convert value to a number.
 Return 0 if it cannot be represented as number
 @return Integer representation of this property
 */
int QSIProperty::getInt() {
 return value().toInt();
}

/**
 Get type identifier of this Property
 Can be one of: Null, Bool, Int, Real, String, Name, Ref, Array, Dict, Stream
*/
QString QSIProperty::getType() { 
 return util::getTypeId(obj);
}

/** Get human readable and localized name of type of this Property */
QString QSIProperty::getTypeName() { 
 return util::getTypeName(obj);
}

/**
 Set value of this property to specified string.
 Work only on Bool, Int, Real, String or Name types,
 tries to "inteligently" convert value if type of property is different than type of parameter
 Will do nothing if called on different types
 @param value new value for the property
*/
void QSIProperty::set(const QString &value) {
 try {
  switch(obj->getType()) {
   case pBool: { //Convert to bool, call set()
    if (value.find("true",0,false)==0) {
     //String starts with "true" (case insensitive)
     set(true);
     return;
    }
    if (value.find("false",0,false)==0) {
     //String starts with "false" (case insensitive)
     set(false);
     return;
    }
    set(value.toInt()!=0);
    return;
   }
   case pInt: { //Convert to int, call set()
    set(value.toInt());
    return;
   }
   case pReal: { //Convert to double, call set()
    set(value.toDouble());
    return;
   }
   case pName: {
    CName *ip=dynamic_cast<CName*>(obj.get());
    std::string str=value;
    ip->setValue(util::convertFromUnicode(str));
    return;
   }
   case pString: {
    CString *ip=dynamic_cast<CString*>(obj.get());
    std::string str=value;
    ip->setValue(util::convertFromUnicode(str));
    return;
   }
   default:;//Do nothing
  }
 } catch (ReadOnlyDocumentException &e) {
  base->errorException("IProperty","set",tr("Document is read-only"));
 }
}

/** \copydoc set(const QString &) */
void QSIProperty::set(int value) {
 switch(obj->getType()) {
  case pBool: { //Convert to bool, call set()
   set(value!=0);
   return;
  }
  case pInt: {
   CInt *ip=dynamic_cast<CInt*>(obj.get());
   try {
    ip->setValue(value);
   } catch (ReadOnlyDocumentException &e) {
    base->errorException("IProperty","set",tr("Document is read-only"));
    }
   return;
  }
  case pReal: { //Convert to double, call set()
   set((double)value);
   return;
  }
  case pName: 
  case pString: { //Convert to string, call set()
   set(QString::number(value));
   return;
  }
  default:;//Do nothing
 }
}

/** \copydoc set(const QString &) */
void QSIProperty::set(double value) {
 switch(obj->getType()) {
  case pBool: { //Convert to bool, call set()
   set(value!=0.0);
   return;
  }
  case pInt: { //Convert to int, call set()
   set((int)value);
   return;
  }
  case pReal: {
   CReal *ip=dynamic_cast<CReal*>(obj.get());
   try {
    ip->setValue(value);
   } catch (ReadOnlyDocumentException &e) {
    base->errorException("IProperty","set",tr("Document is read-only"));
   }
   return;
  }
  case pName: 
  case pString: { //Convert to string, call set()
   set(QString::number(value));
   return;
  }
  default:;//Do nothing
 }
}

/** \copydoc set(const QString &) */
void QSIProperty::set(bool value) {
 switch(obj->getType()) {
  case pBool: {
   CBool *ip=dynamic_cast<CBool*>(obj.get());
   try {
    ip->setValue(value);
   } catch (ReadOnlyDocumentException &e) {
    base->errorException("IProperty","set",tr("Document is read-only"));
   }
   return;
  }
  case pInt: { //Convert to int, call set()
   if (value) set((int)1); else set((int)0);
   return;
  }
  case pReal: { //Convert to double, call set()
   if (value) set((double)1.0); else set((double)0.0);
   return;
  }
  case pName: 
  case pString: { //Convert to string, call set()
   if (value) set("true"); else set("false");
   return;
  }
  default:;//Do nothing
 }
}

/**
 Return reference to this property, but if the property is a reference, create and return the reference target.
 This way you will always get dereferenced property for correct manipulation.
 If new item was created this way, caller is responsible for freeing the item.
 @return Dereferenced property
 */
QSCObject* QSIProperty::ref() {
 if (obj->getType()==pRef) return QSImporter::createQSObject(util::dereference(obj),base);
 return this;
}

/** destructor */
QSIProperty::~QSIProperty() {
}

/** get IProperty held inside this class. Not exposed to scripting */
boost::shared_ptr<IProperty> QSIProperty::get() const {
 return obj;
}

} // namespace gui
