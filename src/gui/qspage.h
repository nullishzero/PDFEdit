#ifndef __QSPAGE_H__
#define __QSPAGE_H__

#include <qstring.h>
#include <qobject.h>
#include <cpage.h>
#include "qsdict.h"
#include "qscobject.h"

namespace gui {

using namespace pdfobjects;

class QSPage : public QSCObject {
 Q_OBJECT
public:
 virtual ~QSPage();
 QSPage(boost::shared_ptr<CPage> _page);
 boost::shared_ptr<CPage> get();
public slots:
 QSDict *getDictionary();
 QString getText();
private:
 /** Object held in class*/
 boost::shared_ptr<CPage> obj;
};

} // namespace gui

#endif
