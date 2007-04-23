/*                                                                              
 * PDFedit - free program for PDF document manipulation.                        
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko, 
 *                                              Miroslav Jahoda,       
 *                                              Jozef Misutka, 
 *                                              Martin Petricek                                             
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit                                                                      
 */ 
#ifndef __BOOLOPTION_H__
#define __BOOLOPTION_H__

#include "option.h"
class QString;
class QCheckBox;
class QResizeEvent;

namespace gui {

/**
 Class for widget containing one editable setting of type bool<br>
 Used as one item type in option window<br>
 \brief Widget to edit boolean option
*/
class BoolOption : public Option {
 Q_OBJECT
public:
 BoolOption(const QString &_key,QWidget *parent=0,bool _defValue=false);
 virtual ~BoolOption();
 virtual QSize sizeHint() const;
 virtual void writeValue();
 virtual void readValue();
protected:
 virtual void resizeEvent (QResizeEvent *e);
protected slots:
 void boolChange();
protected:
 /** checkbox used for editing the value */
 QCheckBox *ed;
 /** Default value if option not found */
 bool defValue;
};

} // namespace gui

#endif
