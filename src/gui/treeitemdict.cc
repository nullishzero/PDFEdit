/** @file
 TreeItemDict - class holding one CDict object in tree, descendant of TreeItem
 @author Martin Petricek
*/

#include <cobject.h>
#include "treeitemdict.h"
#include "treedata.h"
#include "pdfutil.h"
#include "qsdict.h"

namespace gui {

using namespace std;
using namespace util;

/**
 @copydoc TreeItem(const QString&,TreeData *,QListView *,boost::shared_ptr<IProperty>,const QString&,QListViewItem *)
 */
TreeItemDict::TreeItemDict(TreeData *_data,QListView *parent,boost::shared_ptr<IProperty> pdfObj,const QString name/*=QString::null*/,QListViewItem *after/*=NULL*/,const QString &nameId/*=NULL*/):TreeItem(nameId,_data,parent,pdfObj,name,after) {
 assert(data);
 reload(false);
 initObserver();
}

/**
@copydoc TreeItem(const QString&,TreeData *,QListViewItem *,boost::shared_ptr<IProperty>,const QString&,QListViewItem *)
 */
TreeItemDict::TreeItemDict(TreeData *_data,QListViewItem *parent,boost::shared_ptr<IProperty> pdfObj,const QString name/*=QString::null*/,QListViewItem *after/*=NULL*/,const QString &nameId/*=NULL*/):TreeItem(nameId,_data,parent,pdfObj,name,after) {
 assert(data);
 reload(false);
 initObserver();
}

//See TreeItemAbstract for description of this virtual method
TreeItemAbstract* TreeItemDict::createChild(const QString &name,ChildType typ,QListViewItem *after/*=NULL*/) {
 CDict *dict=dynamic_cast<CDict*>(obj.get());
 boost::shared_ptr<IProperty> property=dict->getProperty(name);
 return TreeItem::create(data,this,property,name,after);
}

//See TreeItemAbstract for description of this virtual method
ChildType TreeItemDict::getChildType(const QString &name) {
 CDict *dict=dynamic_cast<CDict*>(obj.get());
 boost::shared_ptr<IProperty> property=dict->getProperty(name);
 return property->getType();
}

//See TreeItemAbstract for description of this virtual method
QStringList TreeItemDict::getChildNames() {
 QStringList itemList;
 CDict *dict=dynamic_cast<CDict*>(obj.get());
 vector<string> list;
 dict->getAllPropertyNames(list);
 vector<string>::iterator it;
 for( it=list.begin();it!=list.end();++it) { // for each property
  boost::shared_ptr<IProperty> property=dict->getProperty(*it);
  if (!data->showSimple() && isSimple(property)) continue; //simple item -> skip it
  itemList += *it;
 }
 return itemList;
}

//See TreeItemAbstract for description of this virtual method
QSCObject* TreeItemDict::getQSObject() {
 boost::shared_ptr<CDict> dict=boost::dynamic_pointer_cast<CDict>(obj);
 assert(dict.get());
 return new QSDict(dict);
}

/** default destructor */
TreeItemDict::~TreeItemDict() {
 uninitObserver();
}

} // namespace gui
