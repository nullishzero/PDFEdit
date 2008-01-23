/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko,
 *                                              Miroslav Jahoda,
 *                                              Jozef Misutka,
 *                                              Martin Petricek
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit
 */
#ifndef __QSTREEITEMCONTENTSTREAM_H__
#define __QSTREEITEMCONTENTSTREAM_H__

#include <qobject.h>
#include "qstreeitem.h"

namespace gui {

class TreeItemContentStream;

/*= This type of object represents one item in treeview representing content stream.*/
/** \brief QObject wrapper around TreeItemContentStream */
class QSTreeItemContentStream : public QSTreeItem {
 Q_OBJECT
public slots:
 /*-
  Set mode of this tree item, i.e. what operators to show as children.
  Can be:
  <informaltable frame="none">
   <tgroup cols="2"><tbody>
    <row><entry>all	</entry><entry>Show all operators</entry></row>
    <row><entry>text	</entry><entry>Show only text operators</entry></row>
    <row><entry>font	</entry><entry>Show only font operators</entry></row>
    <row><entry>graphic	</entry><entry>Show only graphic operators</entry></row>
   </tbody></tgroup>
  </informaltable>
 */
 void setMode(const QString &newMode);
 /*-
  Get mode of this tree item, i.e. what operators to show as children.
  See setMode for list of modes it can return.
 */
 QString getMode();
public:
 QSTreeItemContentStream(TreeItemContentStream *item,BaseCore *_base);
 virtual ~QSTreeItemContentStream();
};

} // namespace gui

#endif
