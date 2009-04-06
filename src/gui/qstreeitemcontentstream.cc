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
 QSTreeItemContentStream - QObject wrapper around TreeItemContentStream
 Export some functions to scripting.<br>
 See doc/user/scripting.xml or kernel documentation for more informations about these functions
 @author Martin Petricek
*/

#include "qstreeitemcontentstream.h"
#include "treeitemcontentstream.h"

namespace gui {

/**
 Construct wrapper with given TreeItemContentStream
 @param item tree item containing Content stream
 @param _base scripting base
 */
QSTreeItemContentStream::QSTreeItemContentStream(TreeItemContentStream *item,BaseCore *_base) : QSTreeItem ("TreeItemContentStream",item,_base) {
}

/** \copydoc TreeItemContentStream::setMode(TreeItemContentStreamMode) */
void QSTreeItemContentStream::setMode(const QString &newMode) {
 TreeItemContentStream *treeItem=dynamic_cast<TreeItemContentStream*>(obj);
 assert(treeItem);
 treeItem->setMode(newMode);
}

/** \copydoc TreeItemContentStream::getMode() */
QString QSTreeItemContentStream::getMode() {
 TreeItemContentStream *treeItem=dynamic_cast<TreeItemContentStream*>(obj);
 assert(treeItem);
 return treeItem->getMode();
}

/** destructor */
QSTreeItemContentStream::~QSTreeItemContentStream() {
}

} // namespace gui
