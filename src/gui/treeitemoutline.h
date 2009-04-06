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
#ifndef __TREEITEMOUTLINE_H__
#define __TREEITEMOUTLINE_H__

#include "treeitemdict.h"

class QString;

namespace gui {

using namespace pdfobjects;

/** type representing one outline item */
typedef boost::shared_ptr<IProperty> OutlineItem;

class TreeData;

/**
 Class holding one outline in tree.<br>
 As outline is essentially a dictionary, this item behaves almost like a dictionary item<br>
 Most notable difference is showing the outline title in the tree.
 \brief Tree item containing one outline
*/
class TreeItemOutline : public TreeItemDict {
public:
 TreeItemOutline(TreeData *_data,Q_ListView *parent,OutlineItem pdfObj,const QString name=QString::null,Q_ListViewItem *after=NULL,const QString &nameId=NULL);
 TreeItemOutline(TreeData *_data,Q_ListViewItem *parent,OutlineItem pdfObj,const QString name=QString::null,Q_ListViewItem *after=NULL,const QString &nameId=NULL);
 virtual ~TreeItemOutline();
 //From TreeItemAbstract interface
 virtual void reloadSelf();
private:
 void showTitle();
private:
 /** outline title */
 QString title;
};

} // namespace gui

#endif
