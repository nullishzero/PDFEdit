/*                                                                              
 * PDFedit - free program for PDF document manipulation.                        
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko, 
 *                                              Miroslav Jahoda,       
 *                                              Jozef Misutka, 
 *                                              Martin Petricek                                             
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit                                                                      
 */ 
#ifndef __REFPROPERTY_H__
#define __REFPROPERTY_H__

#include "property.h"
#include <iproperty.h>
class QLineEdit;
class QPushButton;
namespace pdfobjects {
 class CPdf;
}

namespace gui {

using namespace pdfobjects;

/**
 RefProperty - class for widget containing one editable property of type "Ref"<br>
 Represented by editable line and button allowing to pick reference target from the list<br>
 Used as one item in property editor
 \brief Property editor widget for editing CRef
*/
class RefProperty : public Property {
Q_OBJECT
public:
 RefProperty(const QString &_name, QWidget *parent=0, PropertyFlags _flags=defaultPropertyMode);
 virtual QSize sizeHint() const;
 void resizeEvent(QResizeEvent *e);
 virtual ~RefProperty();
 virtual void setValue(IProperty *pdfObject);
 virtual void readValue(IProperty *pdfObject);
 virtual void setDisabled(bool disabled);
 virtual void applyReadOnly(bool _readonly);
 virtual bool isValid();
 IndiRef getValue();
 void setPdf(CPdf *_pdf);
protected slots:
 void selectRef();
 void emitChange();
 void enableChange(const QString &newText);
private:
 /** one line edit control used for editing the value */
 QLineEdit *ed;
 /** Push button for GUI selection */
 QPushButton *pb;
 /** PDF stored for validating references */
 CPdf *pdf;
};

} // namespace gui

#endif
