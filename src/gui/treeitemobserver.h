/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko,
 *                                              Miroslav Jahoda,
 *                                              Jozef Misutka,
 *                                              Martin Petricek
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit
 */
#ifndef __TREEITEMOBSERVER_H__
#define __TREEITEMOBSERVER_H__

#include "treeitemgenericobserver.h"
#include <kernel/iproperty.h>
#include <utils/observer.h>

namespace gui {

/**
 This class provides observer monitoring IProperty item.<br>
 The observer will reload associated tree item when the observed item changes.
 \brief Observer for TreeItem
*/
class TreeItemObserver : public TreeItemGenericObserver<pdfobjects::IProperty>  {
public:
 /**
  Constructor
  @param _parent Object to be reloaded on any change to monitored item
 */
 TreeItemObserver(TreeItemAbstract* _parent) : TreeItemGenericObserver<pdfobjects::IProperty> (_parent) {
  //No extra initialization
 };
 /** Destructor */
 virtual ~TreeItemObserver() throw() {
  //Empty for now
 }
};

} // namespace gui

#endif
