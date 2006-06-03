#ifndef __TREEITEMCONTENTSTREAMOBSERVER_H__
#define __TREEITEMCONTENTSTREAMOBSERVER_H__

#include <ccontentstream.h>
#include <observer.h>

namespace gui {

class TreeItemAbstract;

using namespace pdfobjects;

/**
 This class provides observer monitoring CContentstream item.<br>
 The observer will reload associated tree item when the observed item changes.
*/
class TreeItemContentStreamObserver : public CContentStream::Observer {
public:
 TreeItemContentStreamObserver(TreeItemAbstract* _parent);
 void deactivate();
 virtual void notify (boost::shared_ptr<CContentStream> newValue, boost::shared_ptr<const CContentStream::ObserverContext> context) const throw();
 virtual CContentStream::Observer::priority_t getPriority() const throw();
 ~TreeItemContentStreamObserver() throw();
protected:
 /** Parent object holding observed property*/
 TreeItemAbstract *parent;
};

} // namespace gui

#endif
