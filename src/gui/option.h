#ifndef __OPTION_H__
#define __OPTION_H__

#include <qstring.h>
#include <qlineedit.h>
#include "settings.h"

class Option : public QWidget{
 Q_OBJECT
public:
 Option(const QString &_key=0,QWidget *parent=0);
 virtual QString getName();
 virtual ~Option();
 virtual void writeValue()=0;
 virtual void readValue()=0;
protected:
 /** Name of this option */
 QString key;
 /** Settings used to read/write value*/
 Settings *set;
};

#endif
