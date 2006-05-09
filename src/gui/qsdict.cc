/** @file
 QObject wrapper around CDict<br>
 Export some functions to scripting.<br>
 See doc/user/scripting.xml or kernel documentation for more informations about these functions
 @author Martin Petricek
*/

#include "qsdict.h"
#include "qsimporter.h"
#include <cobject.h>

namespace gui {

using namespace pdfobjects;

/** Construct wrapper with given CDict */
QSDict::QSDict(boost::shared_ptr<CDict> _dict,Base *_base) : QSIProperty (_dict,"Dict",_base) {
}

/** Copy constructor */
QSDict::QSDict(QSDict &source) : QSIProperty (source.obj,"Dict",source.base) {
}

/** destructor */
QSDict::~QSDict() {
}

/** get CDict held inside this class. Not exposed to scripting */
boost::shared_ptr<CDict> QSDict::get() {
 return boost::dynamic_pointer_cast<CDict>(obj);
}

/** call CDict::getPropertyCount() */
size_t QSDict::count() {
 CDict *dict=dynamic_cast<CDict*>(obj.get());
 return dict->getPropertyCount();
}

/** call CDict::getProperty(name) */
QSCObject* QSDict::property(const QString &name) {
 CDict *dict=dynamic_cast<CDict*>(obj.get());
 boost::shared_ptr<IProperty> property=dict->getProperty(name);
 return QSImporter::createQSObject(property,base);
}

/** call CDict::delProperty(name) */
void QSDict::delProperty(const QString &name) {
 CDict *dict=dynamic_cast<CDict*>(obj.get());
 std::string pName=name;
 dict->delProperty(pName);
}

/** call CDict::addProperty(name,ip) */
void QSDict::add(const QString &name,QSIProperty *ip) {
 CDict *dict=dynamic_cast<CDict*>(obj.get());
 std::string pName=name;
 dict->addProperty(pName,*(ip->get().get()));
}

/** call CDict::addProperty(name,ip) */
void QSDict::add(const QString &name,QObject *ip) {
 //QSA-bugfix variant of this method
 CDict *dict=dynamic_cast<CDict*>(obj.get());
 QSIProperty *ipr=dynamic_cast<QSIProperty*>(ip);
 std::string pName=name;
 if (ipr) dict->addProperty(pName,*(ipr->get().get()));
}

/** call CDict::addProperty(name,ip) */
void QSDict::add(const QString &name,const QString &ip) {
 CDict *dict=dynamic_cast<CDict*>(obj.get());
 std::string pName=name;
 dict->addProperty(pName,CString(ip));
}

/** call CDict::getStringRepresentation(ret); return ret */
QString QSDict::getText() {
 CDict *dict=dynamic_cast<CDict*>(obj.get());
 std::string text;
 dict->getStringRepresentation(text);
 return text;
}

} // namespace gui

//todo: incomplete
