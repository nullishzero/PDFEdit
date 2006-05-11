#ifndef __QSPAGE_H__
#define __QSPAGE_H__

#include <qobject.h>
#include <cpage.h>
#include "qsdict.h"
class QString;

namespace gui {

class QSContentStream;
class QSDict;
class Base;

using namespace pdfobjects;

/*= This type of object represents one page in document. */
class QSPage : public QSCObject {
 Q_OBJECT
public:
 virtual ~QSPage();
 QSPage(const QSPage &source);
 QSPage(boost::shared_ptr<CPage> _page,Base *_base);
 boost::shared_ptr<CPage> get() const;
public slots:
 /*- Returns content stream of this page */
 QSContentStream* getContentStream();
 /*- Returns page dictionary */
 QSDict* getDictionary();
 /*- Return text representation of this page */
 QString getText();
private:
 /** Object held in class*/
 boost::shared_ptr<CPage> obj;
};

} // namespace gui

#endif
