/** @file
 TreeItemCStream - class holding one CStream object in tree, descendant of TreeItem
 @author Martin Petricek
*/

#include <cobject.h>
#include "treeitemcstream.h"
#include "treedata.h"
#include "pdfutil.h"
#include "qsstream.h"
#include "util.h"

namespace gui {

using namespace std;
using namespace util;

/**
 @copydoc TreeItem(const QString&,TreeData *,QListView *,boost::shared_ptr<IProperty>,const QString&,QListViewItem *)
 */
TreeItemCStream::TreeItemCStream(TreeData *_data,QListView *parent,boost::shared_ptr<IProperty> pdfObj,const QString name/*=QString::null*/,QListViewItem *after/*=NULL*/,const QString &nameId/*=NULL*/):TreeItem(nameId,_data,parent,pdfObj,name,after) {
 assert(data);
 reload(false);
 initObserver();
}

/**
@copydoc TreeItem(const QString&,TreeData *,QListViewItem *,boost::shared_ptr<IProperty>,const QString&,QListViewItem *)
 */
TreeItemCStream::TreeItemCStream(TreeData *_data,QListViewItem *parent,boost::shared_ptr<IProperty> pdfObj,const QString name/*=QString::null*/,QListViewItem *after/*=NULL*/,const QString &nameId/*=NULL*/):TreeItem(nameId,_data,parent,pdfObj,name,after) {
 assert(data);
 reload(false);
 initObserver();
}

//See TreeItemAbstract for description of this virtual method
TreeItemAbstract* TreeItemCStream::createChild(const QString &name,__attribute__((unused)) ChildType typ,QListViewItem *after/*=NULL*/) {
 CStream *dict=dynamic_cast<CStream*>(obj.get());
 boost::shared_ptr<IProperty> property=dict->getProperty(name);
 return TreeItem::create(data,this,property,name,after);
}

//See TreeItemAbstract for description of this virtual method
ChildType TreeItemCStream::getChildType(const QString &name) {
 CStream *dict=dynamic_cast<CStream*>(obj.get());
 boost::shared_ptr<IProperty> property=dict->getProperty(name);
 return property->getType();
}

//See TreeItemAbstract for description of this virtual method
QStringList TreeItemCStream::getChildNames() {
 QStringList itemList;
 CStream *dict=dynamic_cast<CStream*>(obj.get());
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
QSCObject* TreeItemCStream::getQSObject() {
 boost::shared_ptr<CStream> stream=boost::dynamic_pointer_cast<CStream>(obj);
 assert(stream.get());
 return new QSStream(stream);
}

/**
 Remove property with given name from stream
 @param name Name of property to remove
*/
void TreeItemCStream::remove(const QString &name) {
 boost::shared_ptr<CStream> oDict=boost::dynamic_pointer_cast<CStream>(obj);
 assert(oDict.get());
 guiPrintDbg(debug::DBG_DBG,"Removing from CStream: " << name);
 TreeItemAbstract* t=dynamic_cast<TreeItemAbstract*>(items[name]);
 if (t) t->unSelect(data->tree());
 oDict->delProperty(name);
}

/** default destructor */
TreeItemCStream::~TreeItemCStream() {
 uninitObserver();
}

} // namespace gui
