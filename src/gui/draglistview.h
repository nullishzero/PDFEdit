#ifndef __DRAGLISTVIEW_H__
#define __DRAGLISTVIEW_H__

#include <qlistview.h>
#include <qdragobject.h>

namespace gui {

class TreeItemAbstract;

/** Inner class for DragListView DragObject*/
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

/** Class wrapping QListView, providing drag and drop functionality */
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
