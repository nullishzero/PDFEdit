#ifndef __TREEITEMCONTENTSTREAMOBSERVER_H__
#define __TREEITEMCONTENTSTREAMOBSERVER_H__

#include "treeitemgenericobserver.h"
#include <ccontentstream.h>
#include <observer.h>

namespace gui {

/**
 This class provides observer monitoring CContentstream item.<br>
 The observer will reload associated tree item when the observed item changes.
*/
class TreeItemContentStreamObserver : public TreeItemGenericObserver<pdfobjects::CContentStream> {
public:
 /** Constructor */
 TreeItemContentStreamObserver::TreeItemContentStreamObserver(TreeItemAbstract* _parent) : TreeItemGenericObserver<pdfobjects::CContentStream> (_parent) {
  //No extra initialization
 };
 /** Destructor */
 virtual ~TreeItemContentStreamObserver() throw() {
  //Empty for now
 }
};

} // namespace gui

#endif
