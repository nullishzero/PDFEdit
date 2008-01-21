/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko,
 *                                              Miroslav Jahoda,
 *                                              Jozef Misutka,
 *                                              Martin Petricek
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit
 */
#ifndef __TREEITEMPAGEOBSERVER_H__
#define __TREEITEMPAGEOBSERVER_H__

#include "treeitemgenericobserver.h"
#include <kernel/cpage.h>
#include <utils/observer.h>

namespace gui {

/**
 This class provides observer monitoring CPage item.<br>
 The observer will reload associated tree item when the observed item changes.
 \brief Observer for TreeItemPage
*/
class TreeItemPageObserver : public TreeItemGenericObserver<pdfobjects::CPage> {
public:
 /**
  Constructor
  @param _parent Object to be reloaded on any change to monitored item
 */
 TreeItemPageObserver(TreeItemAbstract* _parent) : TreeItemGenericObserver<pdfobjects::CPage> (_parent) {
  //No extra initialization
 };
 /** Destructor */
 virtual ~TreeItemPageObserver() throw() {
  //Empty for now
 }
};

} // namespace gui

#endif
