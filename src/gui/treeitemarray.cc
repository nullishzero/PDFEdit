/** @file
 TreeItemArray - class holding one CArray object in tree, descendant of TreeItem
 @author Martin Petricek
*/

#include <cobject.h>
#include "treeitem.h"
#include "treeitemarray.h"
#include "treedata.h"
#include "treewindow.h"
#include "util.h"
#include "pdfutil.h"

namespace gui {

using namespace std;
using namespace util;

/**
 @copydoc TreeItem(TreeData *,QListView *,IProperty *,const QString,QListViewItem *)
 */
TreeItemArray::TreeItemArray(TreeData *_data,QListView *parent,IProperty *pdfObj,const QString name/*=QString::null*/,QListViewItem *after/*=NULL*/):TreeItem(_data,parent,pdfObj,name,after) {
 assert(data);
 reload(false);
 initObserver();
}

/**
@copydoc TreeItem(TreeData *,QListViewItem *,IProperty *,const QString,QListViewItem *)
 */
TreeItemArray::TreeItemArray(TreeData *_data,QListViewItem *parent,IProperty *pdfObj,const QString name/*=QString::null*/,QListViewItem *after/*=NULL*/):TreeItem(_data,parent,pdfObj,name,after) {
 assert(data);
 reload(false);
 initObserver();
}

//See TreeItemAbstract for description of this virtual method
TreeItemAbstract* TreeItemArray::createChild(const QString &name,ChildType typ,QListViewItem *after/*=NULL*/) {
 CArray *ar=dynamic_cast<CArray*>(obj);
 unsigned int i=name.toUInt();
 boost::shared_ptr<IProperty> property=ar->getProperty(i);
 QString oname;
 oname.sprintf("[%d]",i);
 return TreeItem::create(data,this, property.get(),oname,after);
}

//See TreeItemAbstract for description of this virtual method
ChildType TreeItemArray::getChildType(const QString &name) {
 size_t i=name.toUInt();
 CArray *ar=dynamic_cast<CArray*>(obj);
 boost::shared_ptr<IProperty> property=ar->getProperty(i);
 return property->getType();
}

//See TreeItemAbstract for description of this virtual method
QStringList TreeItemArray::getChildNames() {
 assert(data);
 QStringList itemList;
 CArray *ar=dynamic_cast<CArray*>(obj);
 size_t n=ar->getPropertyCount();
 QString name;
 for(size_t i=0;i<n;i++) { //for each property
  boost::shared_ptr<IProperty> property=ar->getProperty(i);
  if (!data->showSimple() && isSimple(property)) continue; //simple item -> skip it
  itemList += QString::number(i);
 }
 return itemList;
}

/** default destructor */
TreeItemArray::~TreeItemArray() {
 uninitObserver();
}

} // namespace gui
