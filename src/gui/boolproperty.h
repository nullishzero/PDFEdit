/*                                                                              
 * PDFedit - free program for PDF document manipulation.                        
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko, 
 *                                              Miroslav Jahoda,       
 *                                              Jozef Misutka, 
 *                                              Martin Petricek                                             
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit                                                                      
 */ 
#ifndef __BOOLPROPERTY_H__
#define __BOOLPROPERTY_H__

#include "property.h"
class QCheckBox;
namespace pdfobjects {
 class IProperty;
}

namespace gui {

using namespace pdfobjects;

/**
 BoolProperty - class for widget containing one editable property of type "Bool"
 (represented by checkbox)<br>
 Used as one item type in property editor
 \brief Property editor widget for editing CBool
*/
class BoolProperty : public Property {
Q_OBJECT
public:
 virtual QSize sizeHint() const;
 virtual void resizeEvent (QResizeEvent *e);
 BoolProperty(const QString &_name, QWidget *parent=0, PropertyFlags _flags=defaultPropertyMode);
 virtual ~BoolProperty();
 virtual void setDisabled(bool disabled);
 virtual void applyReadOnly(bool _readonly);
 virtual void setValue(IProperty *pdfObject);
 virtual void readValue(IProperty *pdfObject);
 virtual bool isValid();
protected slots:
 void emitChange();
protected:
 /** checkbox used for editing the value */
 QCheckBox *ed;
};

} // namespace gui

#endif
