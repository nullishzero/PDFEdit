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
class QSPage;

using namespace pdfobjects;

/*= This type represent one annotation in page */
/** \brief QObject wrapper around CAnnotation object */
class QSAnnotation : public QSCObject {
 Q_OBJECT
public:
 QSAnnotation(boost::shared_ptr<CAnnotation> pdfObj,boost::shared_ptr<CPage> _page,BaseCore *_base);
 virtual ~QSAnnotation();
 boost::shared_ptr<CAnnotation> get() const;
public slots:
 /*- Returns anotation dictionary */
 QSDict* getDictionary();
 /*- Returns page in which this annotation is, or NULL, if it is not in any page */
 QSPage* getPage();
 /*-
  Remove this annotation from its page, if it is in a page.
  Returns true if it was removed.
 */
 bool remove();
 /*- Return type identifier of annotation */
 QString getType();
 /*- Return human-readable, localized type identifier of annotation */
 QString getTypeName();
private:
 /** Object held in class*/
 boost::shared_ptr<CAnnotation> obj;
 /** Page holding this annotation */
 boost::shared_ptr<CPage> page;
};

} // namespace gui

#endif
