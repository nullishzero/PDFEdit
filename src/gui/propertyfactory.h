/*                                                                              
 * PDFedit - free program for PDF document manipulation.                        
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko, 
 *                                              Miroslav Jahoda,       
 *                                              Jozef Misutka, 
 *                                              Martin Petricek                                             
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit                                                                      
 */ 
#ifndef __PROPERTYFACTORY_H__
#define __PROPERTYFACTORY_H__

#include "property.h"
class QWidget;
namespace pdfobjects {
 class IProperty;
}

namespace gui {

Property* propertyFactory(pdfobjects::IProperty *prop,const QString &_name=0,QWidget *parent=0);

} // namespace gui

#endif
