/** @file
 Property - abstract class for widget containing one editable property<br>
 Should be subclassed for editing specific property types (integer, string, ...)
 @author Martin Petricek
*/

#include "property.h"
#include <iproperty.h>
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
 : QWidget (parent, "property"){
 name=_name;
 changed=false;
 hidden=readonly=false; 
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
  case mdReadOnly:
   //Shift to gray
   colorMod(widget,QColor(128,128,128),0.3,QColor(128,128,128),0.3);
   return;
  case mdHidden:
   //Shift to blue
   colorMod(widget,QColor(0,0,255),0.2,QColor(0,0,0),0.0);
   return;
  case mdAdvanced:
   //Shift to gray + red
   colorMod(widget,QColor(255,128,128),0.3,QColor(255,128,128),0.3);
   return;
  case mdUnknown:
   //Shift to heavy green
   colorMod(widget,QColor(128,255,128),0.3,QColor(0,0,0),0.0);
   return;
 }
}

/** 
 Initialize label belonging to this property.
 Called after setting text to the label.
 Property may change the label test, style or colors
 @param label Property's label
*/
void Property::initLabel(QLabel *widget) {
 switch (flags) {
  case mdNormal:
   //No color modification
   return;
  case mdReadOnly:
  case mdAdvanced:
   //Shift to gray
   colorMod(widget,QColor(128,128,128),0.3,QColor(128,128,128),0.3);
   return;
  case mdHidden:
   //Shift to blue
   colorMod(widget,QColor(0,0,255),0.5,QColor(0,0,255),0.5);
   return;
  case mdUnknown:
   //Shift to light green
   colorMod(widget,QColor(128,255,128),0.5,QColor(0,0,0),0.0);
   return;
 }
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
 if (flags==mdReadOnly || flags==mdAdvanced) readonly=true; else readonly=false;
}

/**
 Returns readonly flag of this property
 @return readonly flag
*/
bool Property::getReadOnly() {
 return readonly;
}

/**
 Explicitly set readonly flag of this property and thus enable/disable the control,
 bypassing all other flags
 @param _readonly New value for readonly flag
*/
void Property::setReadOnly(bool _readonly) {
 readonly=_readonly;
}

/** default destructor */
Property::~Property() {

}

/**
 Emit signal indicating change of the property and set state
 of property to 'changed'
*/
void Property::emitChanged() {
 emit propertyChanged(this);
 guiPrintDbg(debug::DBG_DBG,"Property was edited: " << name);
 changed=true;
}

} // namespace gui
