#ifndef __QSPAGE_H__
#define __QSPAGE_H__

#include <qstring.h>
#include <qobject.h>
#include <cpage.h>
#include "qsdict.h"
#include "qsobject.h"

using namespace pdfobjects;

class QSPage : public QSObject<CPage> {
 Q_OBJECT
public:
 QSPage(CPage *_page);
 virtual ~QSPage();
public slots:
 QSDict *getDictionary();
 unsigned int getPageNumber();
 void setPageNumber(unsigned int num);
 QString getText();
};

#endif
