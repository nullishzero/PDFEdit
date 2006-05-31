#ifndef __QSTREEITEMCONTENTSTREAM_H__
#define __QSTREEITEMCONTENTSTREAM_H__

#include <qobject.h>
#include "qstreeitem.h"

namespace gui {

class Base;
class TreeItemContentStream;

/*= This type of object represents one item in treeview representing content stream.*/
class QSTreeItemContentStream : public QSTreeItem {
 Q_OBJECT
public slots:
 /*-
  Set mode of this tree item, i.e. what operators to show as children.
  Can be "all" (showing all operators) or "text" (showing only text operators)
 */
 void setMode(const QString &newMode);
public:
 QSTreeItemContentStream(TreeItemContentStream *item,Base *_base);
 virtual ~QSTreeItemContentStream();
};

} // namespace gui

#endif
