/** @file
 TreeItemArray - class holding one CArray object in tree, descendant of TreeItem
 @author Martin Petricek
*/

#include <cobject.h>
#include "treeitemarray.h"
#include "treedata.h"
#include "pdfutil.h"
#include "qsarray.h"

namespace gui {

using namespace std;
using namespace util;

/**
 @copydoc TreeItem(const QString&,TreeData *,QListView *,boost::shared_ptr<IProperty>,const QString&,QListViewItem *)
 */
TreeItemArray::TreeItemArray(TreeData *_data,QListView *parent,boost::shared_ptr<IProperty> pdfObj,const QString name/*=QString::null*/,QListViewItem *after/*=NULL*/,const QString &nameId/*=NULL*/):TreeItem(nameId,_data,parent,pdfObj,name,after) {
 assert(data);
 reload(false);
 initObserver();
}

/**
@copydoc TreeItem(const QString&,TreeData *,QListViewItem *,boost::shared_ptr<IProperty>,const QString&,QListViewItem *)
 */
TreeItemArray::TreeItemArray(TreeData *_data,QListViewItem *parent,boost::shared_ptr<IProperty> pdfObj,const QString name/*=QString::null*/,QListViewItem *after/*=NULL*/,const QString &nameId/*=NULL*/):TreeItem(nameId,_data,parent,pdfObj,name,after) {
 assert(data);
 reload(false);
 initObserver();
}

//See TreeItemAbstract for description of this virtual method
TreeItemAbstract* TreeItemArray::createChild(const QString &name,__attribute__((unused)) ChildType typ,QListViewItem *after/*=NULL*/) {
 CArray *ar=dynamic_cast<CArray*>(obj.get());
 unsigned int i=name.toUInt();
 boost::shared_ptr<IProperty> property=ar->getProperty(i);
 QString oname;
 oname.sprintf("[%d]",i);
 return TreeItem::create(data,this,property,oname,after,name);
}

//See TreeItemAbstract for description of this virtual method
ChildType TreeItemArray::getChildType(const QString &name) {
 size_t i=name.toUInt();
 CArray *ar=dynamic_cast<CArray*>(obj.get());
 boost::shared_ptr<IProperty> property=ar->getProperty(i);
 return property->getType();
}

//See TreeItemAbstract for description of this virtual method
QStringList TreeItemArray::getChildNames() {
 assert(data);
 QStringList itemList;
 CArray *ar=dynamic_cast<CArray*>(obj.get());
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

//See TreeItemAbstract for description of this virtual method
QSCObject* TreeItemArray::getQSObject() {
 assert(boost::dynamic_pointer_cast<CArray>(obj).get());
 return new QSArray(boost::dynamic_pointer_cast<CArray>(obj),data->base());
}

/**
 Remove property with given index from array
 @param idx Index of property to remove
*/
void TreeItemArray::remove(unsigned int idx) {
 //This is tricky, as the items shift and de-facto they change key
 // and just reloading them does not work as expected
 boost::shared_ptr<CArray> oArray=boost::dynamic_pointer_cast<CArray>(obj);
 assert(oArray.get());
 guiPrintDbg(debug::DBG_DBG,"Removing from array: " << idx);
 //Trick to avoid/improve reloading
 //Now push childs with index above one index lower and set 'deleted' child as invalid
 // (it will then go away at 'not in array')
 int cnt=items.count();  //Assume cnt = number of elements if array
 TreeItemAbstract* tx=dynamic_cast<TreeItemAbstract*>(items[QString::number(idx)]);
 assert(tx);//Not a treeitem? What is that?
 items.replace("--",tx);//Copy "current" so we don't lose it;
 tx->unSelect(data->tree());
 QDictIterator<QListViewItem> it(items);
 for(int i=idx;i<cnt-1;i++) {
  //TODO: this is a bit ugly (but still working)
  TreeItemAbstract* t=dynamic_cast<TreeItemAbstract*>(items[QString::number(i+1)]);
  assert(t);//Not a treeitem? What is that?
  items.replace(QString::number(i),t);
  t->setName(QString::number(i));//Move one down
  QString oname;//And now set text too
  oname.sprintf("[%d]",i);
  t->setText(0,oname);
 }
 items.remove(QString::number(cnt-1));
 oArray->delProperty(idx);
}

} // namespace gui
