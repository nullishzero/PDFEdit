#ifndef __QSPAGE_H__
#define __QSPAGE_H__

#include <qstring.h>
#include <qobject.h>
#include <cpage.h>
#include "qsdict.h"
#include "qsobject.h"

using namespace pdfobjects;

class QSPage : public QSObject {
 Q_OBJECT
public:
 QSPage(CPage *_page);
 virtual ~QSPage();
 CPage* get();
public slots:
 QSDict *getDictionary();
 unsigned int getPageNumber();
 void setPageNumber(unsigned int num);
 QString getText();
private:
 /** Object held in class*/
 CPage *obj;
};

#endif
