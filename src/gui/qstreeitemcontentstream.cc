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
