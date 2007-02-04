/*                                                                              
 * PDFedit - free program for PDF document manipulation.                        
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko, 
 *                                              Miroslav Jahoda,       
 *                                              Jozef Misutka, 
 *                                              Martin Petricek                                             
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit                                                                      
 */ 
#ifndef __STRINGPROPERTY_H__
#define __STRINGPROPERTY_H__

#include "property.h"
class QLineEdit;
namespace pdfobjects {
 class IProperty;
}

namespace gui {

using namespace pdfobjects;

/**
 Class for widget containing one editable property of type "String"<br>
 Used as one item type in property editor
 \brief Property editor widget for editing CString
*/
class StringProperty : public Property {
Q_OBJECT
public:
 virtual QSize sizeHint() const;
 virtual void resizeEvent (QResizeEvent *e);
 StringProperty(const QString &_name, QWidget *parent=0, PropertyFlags _flags=defaultPropertyMode);
 virtual ~StringProperty();
 virtual void setValue(IProperty *pdfObject);
 virtual void readValue(IProperty *pdfObject);
 virtual void setDisabled(bool disabled);
 virtual void applyReadOnly(bool _readonly);
 virtual bool isValid();
protected slots:
 void emitChange();
 void enableChange(const QString &newText);
protected:
 /** one line edit control used for editing the value */
 QLineEdit *ed;
};

} // namespace gui

#endif
