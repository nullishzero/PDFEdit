/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006-2009  PDFedit team: Michal Hocko,
 *                                        Jozef Misutka,
 *                                        Martin Petricek
 *                   Former team members: Miroslav Jahoda
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (in doc/LICENSE.GPL); if not, write to the 
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 * MA  02111-1307  USA
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
