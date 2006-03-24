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
 */
TreeItem::TreeItem(QListView *parent,IProperty *pdfObj,const QString name/*=QString::null*/):QListViewItem(parent) {
 init(pdfObj,name);
}

/** constructor of TreeItem - create child item from given object
 @param parent QListViewItem which is parent of this object
 @param pdfObj Object contained in this item
 */
TreeItem::TreeItem(QListViewItem *parent,IProperty *pdfObj,const QString name/*=QString::null*/):QListViewItem(parent) {
 init(pdfObj,name);
}

/** Initialize item from given PDF object */
void TreeItem::init(IProperty *pdfObj,const QString name) {
 PropertyType typ=pdfObj->getType();
 IndiRef iref=pdfObj->getIndiRef();
 QString s;
 // object name
 //TODO: Maybe think of better naming convention
 if (name.isNull()) {
  setText(0,s.sprintf("%d_%d",iref.gen,iref.num));
 } else {
  setText(0,name+s.sprintf("_%d_%d",iref.gen,iref.num));
 }
 // object type
 setText(1,getTypeName(typ));
}

/** default destructor */
TreeItem::~TreeItem() {
}

