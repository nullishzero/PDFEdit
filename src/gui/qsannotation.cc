/** @file
 QObject wrapper around CAnnotation<br>
 Export some functions to scripting.<br>
 See doc/user/scripting.xml or kernel documentation for more informations about these functions
 @author Martin Petricek
*/

#include "qsannotation.h"
#include "qsdict.h"
#include <cannotation.h>
#include <cpage.h>

namespace gui {

using namespace pdfobjects;

/**
 Construct wrapper with given CAnnotation
 @param pdfObj CAnnotation object
 @param _base scripting base
 @param _page page holding the annotation
 */
QSAnnotation::QSAnnotation(boost::shared_ptr<CAnnotation> pdfObj,boost::shared_ptr<CPage> _page,Base *_base) : QSCObject ("Annotation",_base) {
 obj=pdfObj;
 page=_page;
}

/** destructor */
QSAnnotation::~QSAnnotation() {
}

/** Call CAnnotation::getDictionary */
QSDict* QSAnnotation::getDictionary() {
 return new QSDict(obj->getDictionary(),base);
}

/** get CAnnotation held inside this class. Not exposed to scripting */
boost::shared_ptr<CAnnotation> QSAnnotation::get() const {
 return obj;
}

} // namespace gui

//todo: incomplete
