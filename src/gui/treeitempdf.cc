/** @file
 TreeItemPdf - class holding CPDF in tree, descendant of TreeItemAbstract
 @author Martin Petricek
*/

#include "treeitempdf.h"
#include "treeitem.h"
#include "treedata.h"
#include "treeitempage.h"
#include <cobject.h>
#include <cpdf.h>
#include <cpage.h>
#include <qobject.h>
#include "qspdf.h"
#include "util.h"

namespace gui {

/** Child type specific for TreeItemPdf*/
enum childType {invalidItem=-1, pageList=1, outlineList, pageItem, outlineItem, dictItem};

using namespace std;
using namespace pdfobjects;

/** constructor of TreeItemPdf - create root item from given object
 @param _data TreeData containing necessary information about tree in which this item will be inserted
 @param parent QListView in which to put item
 @param _pdf CPdf Object contained in this item
 @param name Name of this item - will be shown in treeview
 @param after Item after which this one will be inserted
 */
TreeItemPdf::TreeItemPdf(TreeData *_data,CPdf *_pdf,QListView *parent,const QString &name/*=QString::null*/,QListViewItem *after/*=NULL*/):TreeItemAbstract("Document",_data,parent,after) {
 init(_pdf,name);
 reloadSelf();
}

/** constructor of TreeItemPdf - create child item from given object
 @param _data TreeData containing necessary information about tree in which this item will be inserted
 @param parent QListViewItem which is parent of this object
 @param _pdf CPdf Object contained in this item
 @param name Name of file - will be shown in treeview
 @param after Item after which this one will be inserted
 */
TreeItemPdf::TreeItemPdf(TreeData *_data,CPdf *_pdf,QListViewItem *parent,const QString &name/*=QString::null*/,QListViewItem *after/*=NULL*/):TreeItemAbstract("Document",_data,parent,after) {
 init(_pdf,name);
 reloadSelf();
}

/** constructor of TreeItemPdf - create special child item of TreeItemPdf
 @param _data TreeData containing necessary information about tree in which this item will be inserted
 @param parent QListViewItem which is parent of this object
 @param name Name (type) of this item - will be shown in treeview
 @param after Item after which this one will be inserted
 */
TreeItemPdf::TreeItemPdf(TreeData *_data,TreeItemPdf *parent,const QString &name,QListViewItem *after/*=NULL*/):TreeItemAbstract(name,_data,parent,after) {
 initSpec(parent->getObject(),name);
}

/** Initialize item from given CPdf object
 @param pdf CPdf used to initialize this item
 @param name Name of this item - will be shown in treeview (usually name of PDF file)
 */
void TreeItemPdf::init(CPdf *pdf,const QString &name) {
 obj=pdf;
 // object name
 if (name.isNull()) {
  setText(0,QObject::tr("<no name>"));
 } else {
  setText(0,name);
 }
 reload(false); //Add all subchilds, etc ...
}


/** Initialize special PDF subitem from given CPdf object and its name (which defines also type of this item)
 @param pdf CPdf used to initialize this item
 @param name Name of this item - will be shown in treeview
 */
void TreeItemPdf::initSpec(CPdf *pdf,const QString &name) {
 obj=pdf;
 // object name
 if (name.isNull()) {
  setText(0,QObject::tr("<no name>"));
 } else {
  setText(0,QObject::tr(name));
 }
 // object type
 setText(1,QObject::tr("List"));
 nType=name;//Set node type
 reload(false);//Add all childs
}

/** return CPdf stored inside this item
 @return stored object (CPdf) */
CPdf* TreeItemPdf::getObject() {
 return obj;
}

/** default destructor */
TreeItemPdf::~TreeItemPdf() {
}

//See TreeItemAbstract for description of this virtual method
void TreeItemPdf::reloadSelf() {
 if (nType.isNull()) { ///Not special type
  // object type
  setText(1,QObject::tr("PDF"));
  // Page count
  int count=obj->getPageCount();
  QString pages=QString::number(count);
  if (count==1) pages+=QObject::tr(" page","1");
  else if (count>=2 && count<=4) pages+=QObject::tr(" pages","2-4");
  else pages+=QObject::tr(" pages","5+");
  setText(2,pages);
 }
}

//See TreeItemAbstract for description of this virtual method
TreeItemAbstract* TreeItemPdf::createChild(const QString &name,ChildType typ,QListViewItem *after/*=NULL*/) {
 if (typ==dictItem) return TreeItem::create(data,this,obj->getDictionary(),QObject::tr("Catalog"));
 if (typ==pageList) return new TreeItemPdf(data,this,QT_TRANSLATE_NOOP("gui::TreeItemPdf","Pages"),after); 
 if (typ==outlineList) return new TreeItemPdf(data,this,QT_TRANSLATE_NOOP("gui::TreeItemPdf","Outlines"),after); 
 if (typ==pageItem) { //Pages - get page given its number
  //name = Page number
  unsigned int i=name.toUInt();
  guiPrintDbg(debug::DBG_DBG,"Adding page by reload() - " << i);
  return new TreeItemPage(data,obj->getPage(i),this,name,after);
 }
 if (typ==outlineItem) { //Outlines - get specific outline
  //TODO: implement
  return NULL;
 }
 assert(0);//Unknown
 return NULL;
}

//See TreeItemAbstract for description of this virtual method
ChildType TreeItemPdf::getChildType(const QString &name) {
 if (nType.isNull()) {//PDF document
  if (name=="Dict") return dictItem;
  if (name=="Pages") return pageList;
  if (name=="Outlines") return outlineList;
  assert(0); //Error
  return invalidItem;
 }
 if (nType=="Pages") return pageItem;
 if (nType=="Outlines") return outlineItem;
 assert(0); //Error
 return invalidItem;
}

//See TreeItemAbstract for description of this virtual method
QStringList TreeItemPdf::getChildNames() {
 if (nType.isNull()) {//PDF document
  QStringList items;
  if (data->showDict()) items += "Dict";
  if (data->showPage()) items += "Pages";
  if (data->showOutline()) items += "Outlines";
  return items;
 }
 if (nType=="Pages") {
  unsigned int count=obj->getPageCount();
  QStringList items;
  for(unsigned int i=1;i<=count;i++) { //Add all pages
   items += QString::number(i);
  }
  return items;
 }
 if (nType=="Outlines") {
  //TODO : implement outlines
  return QStringList();
 }
 assert(0); //Should not happen
 return QStringList();
}

//See TreeItemAbstract for description of this virtual method
void TreeItemPdf::remove() {
 //Cannot remove entire document, just return
 return;
}

//See TreeItemAbstract for description of this virtual method
QSCObject* TreeItemPdf::getQSObject() {
 return new QSPdf(obj,data->base());
}

} // namespace gui
