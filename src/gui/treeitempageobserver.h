#ifndef __TREEITEMPAGEOBSERVER_H__
#define __TREEITEMPAGEOBSERVER_H__

#include "treeitemgenericobserver.h"
#include <cpage.h>
#include <observer.h>

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
