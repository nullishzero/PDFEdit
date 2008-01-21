/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko,
 *                                              Miroslav Jahoda,
 *                                              Jozef Misutka,
 *                                              Martin Petricek
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit
 */
/** @file
 TreeItemOutline - class holding one outline in tree
 @author Martin Petricek
*/

#include "treeitemoutline.h"
#include "qtcompat.h"
#include <kernel/iproperty.h>
#include <kernel/coutline.h>
#include "util.h"

namespace gui {

class TreeData;

using namespace std;

/**
 \copydoc TreeItem(const QString&,TreeData *,Q_ListView *,boost::shared_ptr<IProperty>,const QString&,Q_ListViewItem *)
 */
TreeItemOutline::TreeItemOutline(TreeData *_data,Q_ListView *parent,OutlineItem pdfObj,const QString name/*=QString::null*/,Q_ListViewItem *after/*=NULL*/,const QString &nameId/*=NULL*/)
 : TreeItemDict(_data,parent,pdfObj,name,after,nameId) {
 assert(data);
 reload(false);
 showTitle();
}

/**
 \copydoc TreeItem(const QString&,TreeData *,Q_ListViewItem *,boost::shared_ptr<IProperty>,const QString&,Q_ListViewItem *)
 */
TreeItemOutline::TreeItemOutline(TreeData *_data,Q_ListViewItem *parent,OutlineItem pdfObj,const QString name/*=QString::null*/,Q_ListViewItem *after/*=NULL*/,const QString &nameId/*=NULL*/)
 : TreeItemDict(_data,parent,pdfObj,name,after,nameId) {
 assert(data);
 reload(false);
 showTitle();
}

/** Update title of the item */
void TreeItemOutline::showTitle() {
 try {
  //Reload the title
  title=util::convertToUnicode(getOutlineText(obj),util::PDF);
 } catch (...) {
  guiPrintDbg(debug::DBG_WARN,"Someone deleted title property in outline");
  title="<?>";
 }
 setText(0,title);
}

//See TreeItemAbstract for description of this virtual method
void TreeItemOutline::reloadSelf() {
 guiPrintDbg(debug::DBG_DBG,"Reload Outline ->" << Q_OUT(title));
 TreeItemDict::reloadSelf();
 showTitle();
}

/** default destructor */
TreeItemOutline::~TreeItemOutline() {
}

} // namespace gui
