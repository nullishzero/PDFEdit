#ifndef __OPTION_H__
#define __OPTION_H__

#include <qstring.h>
#include <qlineedit.h>
#include "settings.h"

class Option : public QWidget{
 Q_OBJECT
public:
 Option(const QString &_key=0,QWidget *parent=0);
 QString getName();
 virtual ~Option();
 QSize sizeHint() const;
 void resizeEvent (QResizeEvent *e);
 virtual void writeValue();
 virtual void readValue();
protected:
 /** Name of this option */
 QString key;
 /** one line edit control used for editing the value */
 QLineEdit *ed;
 /** Settings used to read/write value*/
 Settings *set;
};

#endif
