/** @file
 SelectTool - Toolbutton with drop-down combobox with selectable items
 @author Martin Petricek
*/

#include "selecttool.h"
#include "settings.h"
#include <utils/debug.h>

namespace gui {

/**
 Default constructor of SelectTool
 @param parent Toolbar containing this control
 @param name Name of this widget (passed to QWidget constructor)
 @param cName name of text in this widget
*/
SelectTool::SelectTool(const QString &cName,QWidget *parent/*=0*/,const char *name/*=NULL*/) : QWidget (parent,name) {
 textName=cName;
 text="";
 ed=new QComboBox(false,this,"SelectTool");
 ed->setDuplicatesEnabled(false);
 QObject::connect(ed,SIGNAL(activated(const QString &)),this,SLOT(textEntered(const QString &)));
}

/** default destructor */
SelectTool::~SelectTool() {
}

/**
 return size hint of this control
 @return size hint
*/
QSize SelectTool::sizeHint() const {
 return ed->sizeHint();
}

/**
 Set list of predefined values
 @param predefs List of predefined values for quick selecting
 @param separator separator of values in list, by default comma
*/
void SelectTool::setPredefs(const QStringList &predefs) {
 //Get old text
 QString oldText=ed->currentText();
 //Update choices
 ed->clear();
 ed->insertStringList(predefs);
 text=predefs[0];
 choices=predefs;
 //Try to select back original text if possible
 setText(oldText);
}

/**
 Set list of predefined values
 @param predefs List of predefined values for quick selecting
 @param separator separator of values in list, by default comma
*/
void SelectTool::setPredefs(const QString &predefs,const QString &separator/*=","*/) {
 QStringList plist=QStringList::split(separator,predefs,true);
 setPredefs(plist);
}

/**
 Called on resizing of property editing control
 Will simply set the same fixed size to inner editbox
 @param e resize event
*/
void SelectTool::resizeEvent (QResizeEvent *e) {
 ed->setFixedSize(e->size());
}

/**
 Called whenever text in the control changes
 @param str New text
*/
void SelectTool::textEntered(const QString &str) {
 text=str;
 emit clicked(textName);
}

/**
 Return text inside this control
*/
QString SelectTool::getText() const {
 return text;
}

/**
 Return name of the text edit box
*/
QString SelectTool::getName() const {
 return textName;
}

/**
 Set text inside this control
 @param newText new text to set
*/
void SelectTool::setText(const QString &newText) {
 for (int i=0;i<(int)(choices.count());i++) {
  if (choices[i]==newText) {
   text=newText;
   ed->setCurrentItem(i);
   return;
  }
 }
}

} // namespace gui
