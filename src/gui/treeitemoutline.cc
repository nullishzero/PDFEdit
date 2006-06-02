/** @file
 TreeItemOutline - class holding one outline in tree
 @author Martin Petricek
*/

#include <cobject.h>
#include <coutline.h>
#include "treeitemoutline.h"
#include "util.h"
#include "pdfutil.h"

namespace gui {

class TreeData;

using namespace std;

/**
 @copydoc TreeItem(const QString&,TreeData *,QListView *,boost::shared_ptr<IProperty>,const QString&,QListViewItem *)
 */
TreeItemOutline::TreeItemOutline(TreeData *_data,QListView *parent,OutlineItem pdfObj,const QString name/*=QString::null*/,QListViewItem *after/*=NULL*/,const QString &nameId/*=NULL*/)
 : TreeItemDict(_data,parent,pdfObj,name,after,nameId) {
 assert(data);
 reload(false);
 showTitle();
}

/**
@copydoc TreeItem(const QString&,TreeData *,QListViewItem *,boost::shared_ptr<IProperty>,const QString&,QListViewItem *)
 */
TreeItemOutline::TreeItemOutline(TreeData *_data,QListViewItem *parent,OutlineItem pdfObj,const QString name/*=QString::null*/,QListViewItem *after/*=NULL*/,const QString &nameId/*=NULL*/)
 : TreeItemDict(_data,parent,pdfObj,name,after,nameId) {
 assert(data);
 reload(false);
 showTitle();
}

/** Update title of the item */
void TreeItemOutline::showTitle() {
 try {
  //Reload the title
  title=getOutlineText(obj);
 } catch (...) {
  guiPrintDbg(debug::DBG_WARN,"Someone deleted title property in outline");
  title="<?>";
 }
 setText(0,title);
}

//See TreeItemAbstract for description of this virtual method
void TreeItemOutline::reloadSelf() {
 guiPrintDbg(debug::DBG_DBG,"Reload Outline ->" << title);
 TreeItemDict::reloadSelf();
 showTitle();
}

/** default destructor */
TreeItemOutline::~TreeItemOutline() {
}

} // namespace gui
