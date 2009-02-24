#ifndef _COMMON_H
#define _COMMON_H

#include <boost/shared_ptr.hpp>
#include <kernel/pdfedit-core-dev.h>
#include <kernel/cpdf.h>
#include <string>

typedef std::vector<pdfobjects::IndiRef> RefContainer;
typedef std::vector<int> PagePosList;
int add_ref(RefContainer &refs, const char *refStr);
int add_page_range(PagePosList &pagePos, const char *posStr);

#endif
