/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006-2009  PDFedit team: Michal Hocko,
 *                                        Jozef Misutka,
 *                                        Martin Petricek
 *                   Former team members: Miroslav Jahoda
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (in doc/LICENSE.GPL); if not, write to the 
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 * MA  02111-1307  USA
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit
 */
/** @file
 Property - abstract class for widget containing one editable property<br>
 Should be subclassed for editing specific property types (integer, string, ...)
 @author Martin Petricek
*/

#include "property.h"
#include "qtcompat.h"
#include <kernel/iproperty.h>
#include <qstring.h>
#include <qlabel.h>
#include "util.h"
#include <utils/debug.h>

namespace gui {

using namespace util;

/**
 Default constructor of property item
 @param parent parent Property Editor containing this control
 @param _name name of this property
 @param _flags flags of this property items (default 0)
*/
Property::Property(const QString &_name/*=0*/,QWidget *parent/*=0*/, PropertyFlags _flags/*=defaultPropertyMode*/)
 : QWidget (parent){
 name=_name;
 changed=false;
 effectiveReadonly=hidden=readonly=false;
 setFlags(_flags);
}

/**
 Return name of this property
 @return Property name
*/
QString Property::getName() {
 return name;
}

/**
 Modify the widget color according to widget flags
 Best results are with themes in which the widget is "white or at least light color"
 @param widget Widget to modify its color
*/
void Property::modifyColor(QWidget* widget) {
 switch (flags) {
  case mdNormal:
   //No color modification
   return;
  case mdHidden:
   //Shift to blue fg
   colorMod(widget,QColor(0,0,255),0.4,QColor(0,0,0),0.0);
   return;
  case mdAdvanced:
   //Shift to reddish fg/bg
   colorMod(widget,QColor(255,64,64),0.3,QColor(255,64,64),0.3);
   return;
  case mdReadOnly:
   //Shift to gray fg/bg
   colorMod(widget,QColor(128,128,128),0.3,QColor(128,128,128),0.3);
   return;
  case mdUnknown:
   //No color modification
   return;
 }
}

/**
 Initialize label belonging to this property.<br>
 Called after setting text to the label.<br>
 Guaranteed to be called before override.<br>
 Property may change the label test, style or colors.
 @param widget Property's label
*/
void Property::initLabel(QLabel *widget) {
 guiPrintDbg(debug::DBG_DBG,"Property " << Q_OUT(widget->text()) << " " << Q_OUT(modeName(flags)));
 propertyLabel=widget;
 switch (flags) {
  case mdNormal:
   //No color modification
   return;
  case mdReadOnly:
   //Shift to gray foreground
   colorMod(widget,QColor(128,128,128),0.4,QColor(0,0,0),0);
   return;
  case mdAdvanced:
   //Shift to gray foreground, blue background
   colorMod(widget,QColor(128,128,128),0.4,QColor(0,0,255),0.14);
   return;
  case mdHidden:
   //Shift to blue background
   colorMod(widget,QColor(0,0,0),0,QColor(0,0,255),0.14);
   return;
  case mdUnknown:
   //Shift to light green foreground
   colorMod(widget,QColor(128,255,128),0.4,QColor(0,0,0),0.0);
   return;
 }
}

/**
 Apply overrides bypassing limitation set by modecontroller.
 @param showHidden Show hidden properties
 @param editReadOnly Edit read-only properties
*/
void Property::override(bool showHidden,bool editReadOnly) {
 bool effectiveHidden=hidden && !showHidden;
 applyHidden(effectiveHidden);
 effectiveReadonly=readonly && !editReadOnly;
 applyReadOnly(effectiveReadonly);
}

/**
 Apply "hidden" flag to property, thus showing or hiding it
 @param hideThis New hidden flag value
*/
void Property::applyHidden(bool hideThis) {
 assert(dynamic_cast<QLabel*>(propertyLabel));
 guiPrintDbg(debug::DBG_DBG,"Hide/show >> " << hideThis << " ?? " << Q_OUT(dynamic_cast<QLabel*>(propertyLabel)->text()));
 if (hideThis) {
  propertyLabel->hide();
  this->hide();
 } else {
  propertyLabel->show();
  this->show();
 }
}

/**
 Check if this property is hidden
 @return true if hidden
*/
bool Property::isHidden() {
 return hidden;
}

/**
 Return flags of this property
 @return Property flags
*/
PropertyFlags Property::getFlags() {
 return flags;
}

/**
 Set flags of this property
 @param flag New value for property flags
*/
void Property::setFlags(PropertyFlags flag) {
 flags=flag;
 readonly=(flags==mdReadOnly || flags==mdAdvanced);
 hidden=  (flags==mdHidden   || flags==mdAdvanced);
}

/**
 Event handler for mouse cursor entering widget.
 @param e Event
*/
void Property::enterEvent(QEvent *e) {
 emit infoText(modeName(flags));
 QWidget::enterEvent(e);
}

/**
 Event handler for mouse cursor leaving widget.
 @param e Event
*/
void Property::leaveEvent(QEvent *e) {
 emit infoText(QString::null);
 QWidget::leaveEvent(e);
}

/**
 For given properyy mode return its localized description
 @param flag Mode
 @return description
*/
QString Property::modeName(PropertyFlags flag) {
 if (flag==mdReadOnly) return tr("Read only");
 if (flag==mdAdvanced) return tr("Advanced");
 if (flag==mdHidden) return tr("Hidden");
 if (flag==mdUnknown) return tr("Unknown");
 if (flag==mdNormal) return tr("Normal");
 assert(0);
 return "?";
}

/** default destructor */
Property::~Property() {
 //Nothing to do here
}

/**
 Emit signal indicating change of the property and set state
 of property to 'changed'
*/
void Property::emitChanged() {
 emit propertyChanged(this);
 guiPrintDbg(debug::DBG_DBG,"Property was edited: " << Q_OUT(name));
 changed=true;
}

} // namespace gui
