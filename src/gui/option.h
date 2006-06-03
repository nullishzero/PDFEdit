#ifndef __OPTION_H__
#define __OPTION_H__

#include <qwidget.h>
class QString;

namespace gui {

/**
 Class for widget containing one editable setting<br>
 Abstract class, must be subclassed for editing specific option types (integer, string ...)
*/
class Option : public QWidget{
 Q_OBJECT
public:
 Option(const QString &_key=0,QWidget *parent=0);
 virtual QString getName();
 virtual ~Option();
 //Abstract functions
 /** write edited value to settings (using key specified in contructor) */
 virtual void writeValue()=0;
 /** read value from settings for editing (using key specified in contructor) */
 virtual void readValue()=0;
protected:
 /** Name of this option */
 QString key;
};

} // namespace gui 

#endif
