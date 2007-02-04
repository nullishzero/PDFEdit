/*                                                                              
 * PDFedit - free program for PDF document manipulation.                        
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko, 
 *                                              Miroslav Jahoda,       
 *                                              Jozef Misutka, 
 *                                              Martin Petricek                                             
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit                                                                      
 */ 
#ifndef __OPTION_H__
#define __OPTION_H__

#include <qwidget.h>
class QString;

namespace gui {

/**
 Class for widget containing one editable setting<br>
 Abstract class, must be subclassed for editing specific option types (integer, string ...)
 \brief Abstract class for option editing widget
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
 /** Was the value changed? */
 bool changed;
};

} // namespace gui 

#endif
