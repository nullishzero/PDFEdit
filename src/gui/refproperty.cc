/** @file
RefProperty - class for widget containing
 one editable property of type "Ref"
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

namespace gui {

using namespace std;

/** Default constructor of property item
 @param parent parent Property Editor containing this control
 @param _name name of this property
 @param _flags flags of this property items (default 0)
 */
RefProperty::RefProperty(const QString &_name, QWidget *parent/*=0*/, PropertyFlags _flags/*=0*/)
 : Property(_name,parent,_flags) {
 ed=new QLineEdit(this,"RefProperty_edit");
 setFocusProxy(ed);
 pb=new QPushButton("..",this,"refproperty_pickbutton");
 ed->setValidator(new RefValidator(ed));
 //light yellow
 ed->setPaletteBackgroundColor(QColor(255,255,224));
 connect(pb,SIGNAL(clicked())		,this,SLOT(selectRef()));
 connect(ed,SIGNAL(returnPressed())	,this,SLOT(emitChange()));
 connect(ed,SIGNAL(lostFocus())		,this,SLOT(emitChange()));
 connect(ed,SIGNAL(textChanged(const QString&)),this,SLOT(enableChange(const QString&)));
}

/** Invoked when someone pushes the ".." button */
void RefProperty::selectRef() {
 printDbg(debug::DBG_WARN,"Select REference : not implemented (TODO)")
 //TODO: implement
}

/** Called when text is accepted */
void RefProperty::emitChange() {
 if (!changed) return;
 emit propertyChanged(this);
}

/** Called when text changes */
void RefProperty::enableChange(const QString &newText) {
 changed=true;
}

/** return size hint of this property editing control */
QSize RefProperty::sizeHint() const {
 return ed->sizeHint();
}

/** Called on resizing of property editing control */
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

 //TODO: move to pdfutils?
/** Check for validity of reference - if ref is valid reference (have target) in given CPdf
 @param pdf pdf to check for reference
 @param ref Indirect reference to check
 @return true if given reference target exists in given pdf, false otherwise
*/
bool isRefValid(CPdf *pdf,IndiRef ref) {
 CXref *cxref=pdf->getCXref();
 Ref _val;//TODO: why there is no knowsRef(IndiRef) ?
 _val.num=ref.num;
 _val.gen=ref.gen;
 if (!cxref->knowsRef(_val)) { //ref not valid
  printDbg(debug::DBG_DBG,"Unknown reference!");
  return false;
 }
 return true;
}

/** write internal value to given PDF object
 @param pdfObject Object to write to
 */
void RefProperty::writeValue(IProperty *pdfObject) {
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
  return;
 }
 obj->writeValue(val);
 changed=false;
}
/** read internal value from given PDF object
 @param pdfObject Object to read from
 */
void RefProperty::readValue(IProperty *pdfObject) {
 CRef* obj=(CRef*)pdfObject;
 IndiRef val;
 obj->getPropertyValue(val);
 QString objString;
 objString.sprintf("%d,%d",val.num,val.gen);
 ed->setText(objString);
 changed=false;
}

} // namespace gui
