/** @file
 TreeItem - class holding one PDF object in tree, descendant of QListViewItem
*/

#include "treeitem.h"
#include "util.h"

using namespace std;

//todo: observers?

/** constructor of TreeItem - create root item from given object
 @param parent QListView in which to put item
 @param pdfObj Object contained in this item
 @param name Name of this item - will be shown in treeview
 */
TreeItem::TreeItem(QListView *parent,IProperty *pdfObj,const QString name/*=QString::null*/,QListViewItem *after/*=NULL*/):QListViewItem(parent,after) {
 init(pdfObj,name);
}

/** constructor of TreeItem - create child item from given object
 @param parent QListViewItem which is parent of this object
 @param pdfObj Object contained in this item
 @param name Name of this item - will be shown in treeview
 */
TreeItem::TreeItem(QListViewItem *parent,IProperty *pdfObj,const QString name/*=QString::null*/,QListViewItem *after/*=NULL*/):QListViewItem(parent,after) {
 init(pdfObj,name);
}

/** Initialize item from given PDF object
 @param pdfObj Object used to initialize this item
 @param name Name of this item - will be shown in treeview
 */
void TreeItem::init(IProperty *pdfObj,const QString name) {
 obj=pdfObj;
 PropertyType typ=pdfObj->getType();
 IndiRef iref=pdfObj->getIndiRef();
 QString s;
 // object name
 if (name.isNull()) {
  setText(0,QObject::tr("<no name>"));
 } else {
  setText(0,name);
 }
 // object type
 setText(1,getTypeName(typ));
 // object generation and number
 setText(2,s.sprintf("%d,%d",iref.gen,iref.num));
}

/** return CObject stored inside this item
 @return stored object (IProperty) */
IProperty* TreeItem::getObject() {
 return obj;
}

/** default destructor */
TreeItem::~TreeItem() {
}

