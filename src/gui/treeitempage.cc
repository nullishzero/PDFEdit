/** @file
 TreeItemPage - class holding CPage in tree, descendant of QListViewItem
*/

#include <cobject.h>
#include <cpdf.h>
#include <cpage.h>
#include "treeitempage.h"
#include "treeitem.h"
#include <qobject.h>

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
TreeItemPage::TreeItemPage(TreeData *_data,CPage *_page,QListView *parent,const QString name/*=QString::null*/,QListViewItem *after/*=NULL*/):TreeItemAbstract(parent,after) {
 data=_data;
 init(_page,name);
}

/** constructor of TreeItemPage - create child item from given object
 @param _data TreeData containing necessary information about tree in which this item will be inserted
 @param parent QListViewItem which is parent of this object
 @param _page CPage Object contained in this item
 @param name Name of this item - will be shown in treeview
 @param after Item after which this one will be inserted
 */
TreeItemPage::TreeItemPage(TreeData *_data,CPage *_page,QListViewItem *parent,const QString name/*=QString::null*/,QListViewItem *after/*=NULL*/):TreeItemAbstract(parent,after) {
 data=_data;
 init(_page,name);
}

/** Initialize item from given CPage
 @param page Object used to initialize this item
 @param name Name of this item - will be shown in treeview
 */
void TreeItemPage::init(CPage *page,const QString &name) {
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
CPage* TreeItemPage::getObject() {
 return obj;
}

/** default destructor */
TreeItemPage::~TreeItemPage() {
}

/** Return list of child names */
TreeItemAbstract* TreeItemPage::createChild(const QString &name,QListViewItem *after/*=NULL*/) {
 if (name=="Dict") { //Return page dictionary
  return new TreeItem(data,this,obj->getDictionary().get(),QObject::tr("Dictionary"),after);
 }
 assert(0);
 return NULL;
}
/** Return list of child names */
QStringList TreeItemPage::getChildNames() {
 return QStringList("Dict");
}

/** Reload itself*/
void TreeItemPage::reloadSelf() {
 //Basically, nothing to reload (all content is in children)
}

} // namespace gui
