#ifndef __TREEITEMOBSERVER_H__
#define __TREEITEMOBSERVER_H__

#include "treeitemgenericobserver.h"
#include <iproperty.h>
#include <observer.h>

namespace gui {

/**
 This class provides observer monitoring IProperty item.<br>
 The observer will reload associated tree item when the observed item changes.
*/
class TreeItemObserver : public TreeItemGenericObserver<pdfobjects::IProperty>  {
public:
 /** Constructor */
 TreeItemObserver::TreeItemObserver(TreeItemAbstract* _parent) : TreeItemGenericObserver<pdfobjects::IProperty> (_parent) {
  //No extra initialization
 };
 /** Destructor */
 virtual ~TreeItemObserver() throw() {
  //Empty for now
 }
};

} // namespace gui

#endif
