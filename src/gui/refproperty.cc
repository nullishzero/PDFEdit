/** @file
 RefProperty - class for widget containing one editable property of type "Ref"<br>
 Represented by editable line and button allowing to pick reference target from the list<br>
 Used as one item in property editor
 @author Martin Petricek
*/

#include <utils/debug.h>
#include "refproperty.h"
#include "refvalidator.h"
#include <string>
#include <stdlib.h>
#include <qlineedit.h>
#include <qcolor.h>
#include <qpushbutton.h>
#include <cobject.h>
#include "pdfutil.h"
#include "util.h"

namespace gui {

using namespace std;
using namespace util;

/** Default constructor of property item
 @param parent parent Property Editor containing this control
 @param _name name of this property
 @param _flags flags of this property items (default 0)
 */
RefProperty::RefProperty(const QString &_name, QWidget *parent/*=0*/, PropertyFlags _flags/*=defaultPropertyMode*/)
 : Property(_name,parent,_flags) {
 ed=new QLineEdit(this,"RefProperty_edit");
 setFocusProxy(ed);
 pb=new QPushButton("..",this,"refproperty_pickbutton");
 ed->setReadOnly(readonly);
 ed->setValidator(new RefValidator(ed));
 //light yellow background color
 ed->setPaletteBackgroundColor(QColor(255,255,224));
 modifyColor(ed);
 connect(pb,SIGNAL(clicked())		,this,SLOT(selectRef()));
 connect(ed,SIGNAL(returnPressed())	,this,SLOT(emitChange()));
 connect(ed,SIGNAL(lostFocus())		,this,SLOT(emitChange()));
 connect(ed,SIGNAL(textChanged(const QString&)),this,SLOT(enableChange(const QString&)));
}

/** Invoked when someone pushes the ".." button */
void RefProperty::selectRef() {
 guiPrintDbg(debug::DBG_WARN,"Select REference : not implemented (TODO)")
 //TODO: implement
}

/** Called when text is accepted -> will emit signal informing about change */
void RefProperty::emitChange() {
 if (!changed) return;
 emit propertyChanged(this);
}

/** @copydoc StringProperty::enableChange */
void RefProperty::enableChange(__attribute__((unused)) const QString &newText) {
 changed=true;
}

/** @copydoc StringProperty::sizeHint */
QSize RefProperty::sizeHint() const {
 return ed->sizeHint();
}

/** @copydoc StringProperty::resizeEvent */
void RefProperty::resizeEvent (QResizeEvent *e) {
 int w=e->size().width();
 int h=e->size().height();
 pb->move(w-h,0);
 pb->setFixedSize(QSize(h,h));
 ed->move(0,0);
 ed->setFixedSize(QSize(w-h,h));
}

/** default destructor */
RefProperty::~RefProperty() {
 delete ed;
 delete pb;
}

/** @copydoc StringProperty::writeValue */
void RefProperty::writeValue(IProperty *pdfObject) {
 if (readonly) return;//Honor readonly setting
 CRef *obj=(CRef*)pdfObject;
 QStringList ref=QStringList::split(",",ed->text());
 assert(ref.count()==2); //Should never happen
 if (ref.count()!=2) return;
 IndiRef val;
 val.num=ref[0].toInt();
 val.gen=ref[1].toInt();

 //Check reference validity
 if (!isRefValid(obj->getPdf(),val)) { 
  ed->setFocus();
  return; //not valid
 }
 obj->writeValue(val);
 changed=false;
}

/** @copydoc StringProperty::readValue */
void RefProperty::readValue(IProperty *pdfObject) {
 CRef* obj=(CRef*)pdfObject;
 IndiRef val;
 obj->getPropertyValue(val);
 QString objString;
 objString.sprintf("%d,%d",val.num,val.gen);
 ed->setText(objString);
 changed=false;
}

/** @copydoc Property::isValid() */
bool RefProperty::isValid() {
 return ed->hasAcceptableInput();
}

/* @copydoc Property:setReadOnly */
void RefProperty::setReadOnly(bool _readonly) {
 //Widget is enabled if it is not read-only
 ed->setEnabled(!_readonly);
 pb->setEnabled(!_readonly);
 Property::setReadOnly(_readonly);
}

} // namespace gui
