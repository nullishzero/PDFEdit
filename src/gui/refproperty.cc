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
 RefProperty - class for widget containing one editable property of type "Ref"<br>
 Represented by editable line and button allowing to pick reference target from the list<br>
 Used as one item in property editor
 @author Martin Petricek
*/

#include "refproperty.h"
#include "pdfutil.h"
#include "refpropertydialog.h"
#include "refvalidator.h"
#include "util.h"
#include <kernel/cobject.h>
#include <qcolor.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <stdlib.h>
#include <string>
#include <utils/debug.h>
#include <qevent.h>

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
 pdf=boost::shared_ptr<CPdf>();
 ed=new QLineEdit(this,"RefProperty_edit");
 setFocusProxy(ed);
 pb=new QPushButton("..",this,"refproperty_pickbutton");
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
 guiPrintDbg(debug::DBG_WARN,"Select Reference via dialog");
 RefPropertyDialog ref(pdf,getValue(),this);
 if (ref.exec()) {
  //Something valid was supposedly selected
  ed->setText(ref.getResult());
  changed=true;
  emitChange();
 }
}

/** Called when text is accepted -> will emit signal informing about change */
void RefProperty::emitChange() {
 if (!changed) return;
 emit propertyChanged(this);
}

/** \copydoc StringProperty::enableChange */
void RefProperty::enableChange(__attribute__((unused)) const QString &newText) {
 changed=true;
}

/** \copydoc StringProperty::sizeHint */
QSize RefProperty::sizeHint() const {
 return ed->sizeHint();
}

/**
 Set PDF explicitly for this property for purpose of validating references<br>
 This is usable if the property is new and does not have the pdf document from the edited CRef
 @param _pdf CPdf to set
*/
void RefProperty::setPdf(boost::weak_ptr<CPdf> _pdf) {
 pdf=_pdf;
}

/** \copydoc StringProperty::resizeEvent */
void RefProperty::resizeEvent(QResizeEvent *e) {
 int w=e->size().width();
 int h=e->size().height();
 if (w<=0 || h<=0) return;//Some invalid size anyway. No need to move the subwidgets
 pb->move(w-h,0);
 pb->setFixedSize(QSize(h,h));
 ed->move(0,0);
 ed->setFixedSize(QSize(MAX(0,w-h),h));
}

/** default destructor */
RefProperty::~RefProperty() {
 delete ed;
 delete pb;
}

/**
 Read the string in this property and return indirect reference parsed from it
 If the string is invalid, both num and gen are set to -1
 @return Indirect reference
*/
IndiRef RefProperty::getValue() {
 IndiRef val;
 val.num=0;
 val.gen=0;
 QStringList ref=QStringList::split(",",ed->text());
 assert(ref.count()<=2);	 //Should never happen
 if (ref.count()!=2) return val;//Invalid reference
 val.num=ref[0].toInt();
 val.gen=ref[1].toInt();
 return val;
}

/** \copydoc StringProperty::setValue */
void RefProperty::setValue(IProperty *pdfObject) {
 if (effectiveReadonly) return;//Honor readonly setting
 CRef *obj=dynamic_cast<CRef*>(pdfObject);
 IndiRef val=getValue();
 if (val.num==0 && val.gen==0) return;//Invalid reference
 //Check reference validity
 boost::shared_ptr<CPdf> objPdf=obj->getPdf().lock();
 if (objPdf) {
  //Get PDF from property if it have one. If not, keep the old PDF
  pdf=objPdf;
 }
 boost::shared_ptr<CPdf> p = pdf.lock();
 if (p) {
  //We can check for validity
  if (!isRefValid(p, val)) {
   emit warnText("Reference is not valid!");
   ed->setFocus();
   return; //not valid
  }
 }
 obj->setValue(val);
 changed=false;
}

/** \copydoc StringProperty::readValue */
void RefProperty::readValue(IProperty *pdfObject) {
 CRef* obj=dynamic_cast<CRef*>(pdfObject);
 IndiRef val;
 boost::shared_ptr<CPdf> objPdf=obj->getPdf().lock();
 if (objPdf) {
  //Get PDF from property if it have one. If not, keep the old PDF
  pdf=objPdf;
 }
 obj->getValue(val);
 QString objString;
 objString.sprintf("%d,%d",val.num,val.gen);
 ed->setText(objString);
 changed=false;
}

/** \copydoc Property::isValid() */
bool RefProperty::isValid() {
 return ed->hasAcceptableInput();
}

//See Property::setDisabled
void RefProperty::setDisabled(bool disabled) {
 ed->setEnabled(!disabled);
 pb->setEnabled(!disabled);
}

//See Property::applyReadOnly
void RefProperty::applyReadOnly(bool _readonly) {
 ed->setReadOnly(_readonly);
 pb->setEnabled(!_readonly);
}


} // namespace gui
