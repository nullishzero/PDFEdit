#ifndef __TREEITEMPAGEOBSERVER_H__
#define __TREEITEMPAGEOBSERVER_H__

#include "treeitemgenericobserver.h"
#include <cpage.h>
#include <observer.h>

namespace gui {

/**
 This class provides observer monitoring CPage item.<br>
 The observer will reload associated tree item when the observed item changes.
*/

class TreeItemPageObserver : public TreeItemGenericObserver<pdfobjects::CPage> {
public:
 /** Constructor */
 TreeItemPageObserver::TreeItemPageObserver(TreeItemAbstract* _parent) : TreeItemGenericObserver<pdfobjects::CPage> (_parent) {
  //No extra initialization
 };
 /** Destructor */
 virtual ~TreeItemPageObserver() throw() {
  //Empty for now
 }
};

} // namespace gui

#endif
