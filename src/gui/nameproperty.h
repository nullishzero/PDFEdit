/*                                                                              
 * PDFedit - free program for PDF document manipulation.                        
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko, 
 *                                              Miroslav Jahoda,       
 *                                              Jozef Misutka, 
 *                                              Martin Petricek                                             
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit                                                                      
 */ 
#ifndef __NAMEPROPERTY_H__
#define __NAMEPROPERTY_H__

#include "stringproperty.h"

namespace gui {

/**
 Class for widget containing one editable property of type "Name"<br>
 Basically, CName is CString, but we might want differentiation in case we'll handle names specially in future<br>
 Used as one item type in property editor
 \brief Property editor widget for editing CName
*/
class NameProperty : public StringProperty {
Q_OBJECT
public:
 NameProperty(const QString &_name, QWidget *parent=0, PropertyFlags _flags=defaultPropertyMode);
 virtual ~NameProperty();
 virtual void setValue(pdfobjects::IProperty *pdfObject);
 virtual void readValue(pdfobjects::IProperty *pdfObject);
};

} // namespace gui 

#endif
