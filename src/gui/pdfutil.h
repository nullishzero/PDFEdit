#ifndef __PDFUTIL_H__
#define __PDFUTIL_H__

#include <iproperty.h>
class QString;
namespace pdfobjects {
class CPdf;
}

namespace util {

using namespace pdfobjects;

QString getTypeId(PropertyType typ);
QString getTypeId(IProperty *obj);
QString getTypeId(boost::shared_ptr<IProperty> obj);
QString getTypeName(PropertyType typ);
QString getTypeName(IProperty *obj);
QString getTypeName(boost::shared_ptr<IProperty> obj);
IndiRef getRef(IProperty *ref);
IndiRef getRef(boost::shared_ptr<IProperty> ref);
bool isRefValid(CPdf *pdf,IndiRef ref);
bool isSimple(IProperty* prop);
bool isSimple(boost::shared_ptr<IProperty> prop);
boost::shared_ptr<IProperty> dereference(boost::shared_ptr<IProperty> obj);

} // namespace util

#endif
