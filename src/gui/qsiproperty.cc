/** @file
 QObject wrapper around IProperty (one property in dictionary or array)<br>
 Export some functions to scripting.<br>
 See doc/user/scripting.xml or kernel documentation for more informations about these functions
 @author Martin Petricek
*/

#include "qsiproperty.h"
#include "pdfutil.h"
#include <qstring.h>
#include <cobject.h>
#include <qsimporter.h>

namespace gui {

using namespace pdfobjects;

/**
 Construct wrapper with given IProperty
 @param _ip IProperty wrapped in this object
 @param _base Scripting base
*/
QSIProperty::QSIProperty(boost::shared_ptr<IProperty> _ip,Base *_base) : QSCObject ("IProperty",_base) {
 assert(_ip.get());
 obj=_ip;
}

/** Copy constructor */
QSIProperty::QSIProperty(QSIProperty &source) : QSCObject ("IProperty",source.base) {
 obj=source.obj;
}

/**
 Construct wrapper with given IProperty and given type name
 @param _ip IProperty wrapped in this object
 @param _className type name of this object
 @param _base Scripting base
*/
QSIProperty::QSIProperty(boost::shared_ptr<IProperty> _ip, QString _className,Base *_base) : QSCObject (_className,_base) {
 obj=_ip;
}

/** Call IProperty::getStringRepresentation(ret); return ret */
QString QSIProperty::getText() {
 std::string text;
 obj->getStringRepresentation(text);
 return text;
}

/**
 Try to convert textual representation to a number.
 Return 0 if it cannot be represented as number
 @return Integer representaion of this property
 */
int QSIProperty::getInt() {
 return getText().toInt();
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
   ip->writeValue(str);
   return;
  }
  case pString: {
   CString *ip=dynamic_cast<CString*>(obj.get());
   std::string str=value;
   ip->writeValue(str);
   return;
  }
  default:;//Do nothing
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
   ip->writeValue(value);
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
   ip->writeValue(value);
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
   ip->writeValue(value);
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
