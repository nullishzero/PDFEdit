#ifndef __PDFUTIL_H__
#define __PDFUTIL_H__

#include <iproperty.h>
class QString;

namespace util {

using namespace pdfobjects;

QString getTypeName(PropertyType typ);
IndiRef getRef(IProperty *ref);

} // namespace util

#endif
