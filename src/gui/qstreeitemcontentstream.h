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
