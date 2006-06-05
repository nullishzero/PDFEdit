#ifndef __QSANNOTATION_H__
#define __QSANNOTATION_H__

#include "qscobject.h"
#include <boost/shared_ptr.hpp>
class QString;
namespace pdfobjects {
 class CPage;
 class CAnnotation;
}

namespace gui {

class QSDict;

using namespace pdfobjects;

/** QObject wrapper around CAnnotation object */
class QSAnnotation : public QSCObject {
 Q_OBJECT
public:
 QSAnnotation(boost::shared_ptr<CAnnotation> pdfObj,boost::shared_ptr<CPage> _page,Base *_base);
 virtual ~QSAnnotation();
 boost::shared_ptr<CAnnotation> get() const;
public slots:
 QSDict* getDictionary();
private:
 /** Object held in class*/
 boost::shared_ptr<CAnnotation> obj;
 /** Page holding this annotation */
 boost::shared_ptr<CPage> page;
};

} // namespace gui

#endif
