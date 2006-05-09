/** @file
 QObject wrapper around CArray<br>
 Export some functions to scripting.<br>
 See doc/user/scripting.xml or kernel documentation for more informations about these functions
 @author Martin Petricek
*/

#include "qsarray.h"
#include "qsimporter.h"
#include <cobject.h>

namespace gui {

using namespace pdfobjects;

/** Construct wrapper with given CArray */
QSArray::QSArray(boost::shared_ptr<CArray> _array,Base *_base) : QSIProperty (_array,"Array",_base) {
}

/** Copy constructor */
QSArray::QSArray(QSArray &source) : QSIProperty (source.obj,"Array",source.base) {
}

/** destructor */
QSArray::~QSArray() {
}

/** get CArray held inside this class. Not exposed to scripting */
boost::shared_ptr<CArray> QSArray::get() {
 return boost::dynamic_pointer_cast<CArray>(obj);
}

/** call CArray::getPropertyCount() */
size_t QSArray::count() {
 CArray *array=dynamic_cast<CArray*>(obj.get());
 return array->getPropertyCount();
}

/** call CArray::getProperty(index) */
QSCObject* QSArray::property(int index) {
 CArray *array=dynamic_cast<CArray*>(obj.get());
 boost::shared_ptr<IProperty> property=array->getProperty(index);
 return QSImporter::createQSObject(property,base);
}

/** call CArray::delProperty(name) */
void QSArray::delProperty(int index) {
 CArray *array=dynamic_cast<CArray*>(obj.get());
 array->delProperty(index);
}

/** call CArray::getStringRepresentation(ret); return ret */
QString QSArray::getText() {
 CArray *array=dynamic_cast<CArray*>(obj.get());
 std::string text;
 array->getStringRepresentation(text);
 return text;
}

/** call CArray::addProperty(index,ip) */
void QSArray::add(int index,QSIProperty *ip) {
 CArray *array=dynamic_cast<CArray*>(obj.get());
 array->addProperty(index,*(ip->get().get()));
}

/** call CArray::addProperty(index,ip) */
void QSArray::add(int index,QObject *ip) {
 //QSA-bugfix variant of this method
 CArray *array=dynamic_cast<CArray*>(obj.get());
 QSIProperty *ipx=dynamic_cast<QSIProperty*>(ip);
 if (ipx) array->addProperty(index,*(ipx->get().get()));
}

/** call CArray::addProperty(index,ip) */
void QSArray::add(int index,const QString &ip) {
 CArray *array=dynamic_cast<CArray*>(obj.get());
 array->addProperty(index,CString(ip));
}

/** call CArray::addProperty(ip) */
void QSArray::add(QSIProperty *ip) {
 CArray *array=dynamic_cast<CArray*>(obj.get());
 array->addProperty(*(ip->get().get()));
}

/** call CArray::addProperty(ip) */
void QSArray::add(QObject *ip) {
 //QSA-bugfix variant of this method
 CArray *array=dynamic_cast<CArray*>(obj.get());
 QSIProperty *ipx=dynamic_cast<QSIProperty*>(ip);
 if (ipx) array->addProperty(*(ipx->get().get()));
}

/** call CArray::addProperty(ip) */
void QSArray::add(const QString &ip) {
 CArray *array=dynamic_cast<CArray*>(obj.get());
 array->addProperty(CString(ip));
}

} // namespace gui

//todo: incomplete
