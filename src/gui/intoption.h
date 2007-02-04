/*                                                                              
 * PDFedit - free program for PDF document manipulation.                        
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko, 
 *                                              Miroslav Jahoda,       
 *                                              Jozef Misutka, 
 *                                              Martin Petricek                                             
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit                                                                      
 */ 
#ifndef __INTOPTION_H__
#define __INTOPTION_H__

#include "stringoption.h"
class QString;

namespace gui {

/**
 class for widget containing one editable setting of type integer<br>
 Used as one item type in option window<br>
 \brief Widget to edit integer option
*/
class IntOption : public StringOption {
 Q_OBJECT
public:
 IntOption(const QString &_key=0,QWidget *parent=0,int defValue=0);
 virtual ~IntOption();
};

} // namespace gui

#endif
