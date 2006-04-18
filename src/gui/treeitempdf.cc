/** @file
 TreeItemPdf - class holding CPDF in tree, descendant of QListViewItem
*/

#include "treeitempdf.h"
#include "treeitem.h"
#include "treedata.h"
#include "treewindow.h"
#include "treeitempage.h"
#include <cobject.h>
#include <cpdf.h>
#include <cpage.h>
#include <qobject.h>

namespace gui {

using namespace std;
using namespace pdfobjects;

/** constructor of TreeItemPdf - create root item from given object
 @param _data TreeData containing necessary information about tree in which this item will be inserted
 @param parent QListView in which to put item
 @param _pdf CPdf Object contained in this item
 @param name Name of this item - will be shown in treeview
 @param after Item after which this one will be inserted
 */
TreeItemPdf::TreeItemPdf(TreeData *_data,CPdf *_pdf,QListView *parent,const QString name/*=QString::null*/,QListViewItem *after/*=NULL*/):QListViewItem(parent,after) {
 data=_data;
 init(_pdf,name);
}

/** constructor of TreeItemPdf - create child item from given object
 @param _data TreeData containing necessary information about tree in which this item will be inserted
 @param parent QListViewItem which is parent of this object
 @param _pdf CPdf Object contained in this item
 @param name Name of file - will be shown in treeview
 @param after Item after which this one will be inserted
 */
TreeItemPdf::TreeItemPdf(TreeData *_data,CPdf *_pdf,QListViewItem *parent,const QString name/*=QString::null*/,QListViewItem *after/*=NULL*/):QListViewItem(parent,after) {
 data=_data;
 init(_pdf,name);
}

/** constructor of TreeItemPdf - create special child item of TreeItemPdf
 @param _data TreeData containing necessary information about tree in which this item will be inserted
 @param parent QListViewItem which is parent of this object
 @param name Name (type) of this item - will be shown in treeview
 @param after Item after which this one will be inserted
 */
TreeItemPdf::TreeItemPdf(TreeData *_data,TreeItemPdf *parent,const QString name,QListViewItem *after/*=NULL*/):QListViewItem(parent,after) {
 data=_data;
 initSpec(parent->getObject(),name);
}

/** Initialize item from given PDF object
 @param pdfObj Object used to initialize this item
 @param name Name of this item - will be shown in treeview
 */
void TreeItemPdf::init(CPdf *pdf,const QString &name) {
 obj=pdf;
 // object name
 if (name.isNull()) {
  setText(0,QObject::tr("<no name>"));
 } else {
  setText(0,name);
 }
 // object type
 setText(1,QObject::tr("PDF"));
 // Page count
 setText(2,QString::number(pdf->getPageCount())+QObject::tr(" page(s)"));
 TreeItemPdf *pages=new TreeItemPdf(data,this,QT_TRANSLATE_NOOP("gui::TreeItemPdf","Pages"),NULL); 
 TreeItemPdf *outlines=new TreeItemPdf(data,this,QT_TRANSLATE_NOOP("gui::TreeItemPdf","Outlines"),pages); 
}

/** Initialize item from given PDF object and expected type
 @param pdfObj Object used to initialize this item
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
 if (name=="Pages") { //Pages list
  unsigned int count=pdf->getPageCount();
  TreeItemPage *last=NULL;
  //TODO: add on demand somehow ... kernel is parsing the page if I call getPage() !
  for(unsigned int i=1;i<=count;i++) { //Add all pages
   CPage *page=pdf->getPage(i).get();
   //TODO: dict add on demand
   last=new TreeItemPage(data,page,this,QString::number(i),last);
   TreeItem *dict=new TreeItem(data,last,page->getDictionary().get(),QObject::tr("Dictionary"));
   data->parent()->addChilds(dict,false);
  }
 } else if (name=="Oulines") { //Outline list
  //TODO : implement
 }
}

/** return CPdf stored inside this item
 @return stored object (CPdf) */
CPdf* TreeItemPdf::getObject() {
 return obj;
}

//TODO: item reloading?

/** default destructor */
TreeItemPdf::~TreeItemPdf() {
}

} // namespace gui
