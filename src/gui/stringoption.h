#ifndef __STRINGOPTION_H__
#define __STRINGOPTION_H__

#include <qstring.h>
#include <qlineedit.h>
#include "option.h"

class StringOption : public Option {
 Q_OBJECT
public:
 StringOption(const QString &_key=0,QWidget *parent=0);
 virtual ~StringOption();
 virtual void writeValue();
 virtual void readValue();
 virtual QSize sizeHint() const;
protected:
 virtual void resizeEvent (QResizeEvent *e);
protected:
 /** one line edit control used for editing the value */
 QLineEdit *ed;
};

#endif
