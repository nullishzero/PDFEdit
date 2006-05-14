/** @file
 TreeItemPage - class holding CPage in tree, descendant of TreeItemAbstract
 @author Martin Petricek
*/

#include <cobject.h>
#include <cpage.h>
#include <ccontentstream.h>
#include "treeitem.h"
#include "treeitempage.h"
#include "treeitemcontentstream.h"
#include "treedata.h"
#include <qobject.h>
#include "qspage.h"
#include "util.h"

namespace gui {

using namespace std;
using namespace pdfobjects;

/** constructor of TreeItemPage - create root item from given object
 @param _data TreeData containing necessary information about tree in which this item will be inserted
 @param parent QListView in which to put item
 @param _page CPage Object contained in this item
 @param name Name of this item - will be shown in treeview
 @param after Item after which this one will be inserted
 */
TreeItemPage::TreeItemPage(TreeData *_data,boost::shared_ptr<CPage> _page,QListView *parent,const QString name/*=QString::null*/,QListViewItem *after/*=NULL*/):TreeItemAbstract(name,_data,parent,after) {
 init(_page,name);
}

/** constructor of TreeItemPage - create child item from given object
 @param _data TreeData containing necessary information about tree in which this item will be inserted
 @param parent QListViewItem which is parent of this object
 @param _page CPage Object contained in this item
 @param name Name of this item - will be shown in treeview
 @param after Item after which this one will be inserted
 */
TreeItemPage::TreeItemPage(TreeData *_data,boost::shared_ptr<CPage> _page,QListViewItem *parent,const QString name/*=QString::null*/,QListViewItem *after/*=NULL*/):TreeItemAbstract(name,_data,parent,after) {
 init(_page,name);
}

/** Initialize item from given CPage
 @param page Object used to initialize this item
 @param name Name of this item - will be shown in treeview
 */
void TreeItemPage::init(boost::shared_ptr<CPage> page,const QString &name) {
 obj=page;
 // object name
 if (name.isNull()) {
  setText(0,QObject::tr("<no name>"));
 } else {
  setText(0,name);
 }
 // object type
 setText(1,QObject::tr("Page"));
 reload(false);//get childs
}

/** return CPage stored inside this item
 @return stored object (CPage) */
boost::shared_ptr<CPage> TreeItemPage::getObject() {
 return obj;
}

/** default destructor */
TreeItemPage::~TreeItemPage() {
}

//See TreeItemAbstract for description of this virtual method
TreeItemAbstract* TreeItemPage::createChild(__attribute__((unused)) const QString &name,ChildType typ,QListViewItem *after/*=NULL*/) {
 if (typ==0) { //Return page dictionary
  return TreeItem::create(data,this,obj->getDictionary(),QObject::tr("Dictionary"),after);
 }
 if (typ==1) {
  size_t streamNumber=name.toUInt();
  if (streamNumber>=streams.size()) {
   //Invalid or old data -> try to re-get list of streams from CPage
   obj->getContentStreams(streams);
  }
  assert(streamNumber<streams.size());  //Still Invalid data ?
  return new TreeItemContentStream(data,this,streams[streamNumber],QObject::tr("Stream")+" "+QString::number(streamNumber),after,QString("Stream")+QString::number(streamNumber));
 }
 assert(0);
 return NULL;
}

//See TreeItemAbstract for description of this virtual method
ChildType TreeItemPage::getChildType(const QString &name) {
 if (name=="Dict") { //Return page dictionary
  return 0;
 } else { //It's a stream
  return 1;
 }
}

//See TreeItemAbstract for description of this virtual method
QStringList TreeItemPage::getChildNames() {
 QStringList childs;
 if (data->showODict()) childs+="Dict";
 if (data->showStream()) {
  obj->getContentStreams(streams);
  for (size_t i=0;i<streams.size();i++) {
   childs+=QString::number(i);
  }
 }
 return childs;
}

//See TreeItemAbstract for description of this virtual method
void TreeItemPage::reloadSelf() {
 //Basically, nothing to reload (any useful content is in children)
}

//See TreeItemAbstract for description of this virtual method
QSCObject* TreeItemPage::getQSObject() {
 return new QSPage(obj,data->base());
}

//See TreeItemAbstract for description of this virtual method
void TreeItemPage::remove() {
 //TODO: implement
 return;
}

} // namespace gui
