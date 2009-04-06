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
#ifndef __DRAGLISTVIEW_H__
#define __DRAGLISTVIEW_H__

#include <qlistview.h>
#include <qdragobject.h>

namespace gui {

class TreeItemAbstract;

/**
 Inner class for DragListView DragObject
 \brief Drag and drop object
*/
class TreeItemDragObject : public QDragObject {
 Q_OBJECT
public:
 TreeItemDragObject(TreeItemAbstract *_item,QWidget *dragSource=0,const char *name=0);
 virtual const char* format (int i=0) const;
 virtual QByteArray encodedData (const char *m) const;
 virtual ~TreeItemDragObject();
private:
 /** Item stored inside this dragobject */
 TreeItemAbstract *item;
};

/**
 Class wrapping QListView, providing drag and drop functionality
 \brief Drag and drop enhanced QListView
 */
class DragListView : public QListView {
 Q_OBJECT
public:
 DragListView(QWidget *parent,const char *name=0,WFlags f=0);
 ~DragListView();
signals:
 /**
  Invoked when dragging one item to another within the same tree window
  @param source Source (dragged) item
  @param target Target item
 */
 void dragDrop(TreeItemAbstract *source,TreeItemAbstract *target);
 /**
  Invoked when dragging one item to another in different tree window (possibly in different document)
  @param source Source (dragged) item
  @param target Target item
 */
 void dragDropOther(TreeItemAbstract *source,TreeItemAbstract *target);
protected:
 virtual void contentsDragMoveEvent(QDragMoveEvent *e);
 virtual void contentsDropEvent(QDropEvent *e);
 virtual QDragObject* dragObject();
};

} // namespace gui

#endif
