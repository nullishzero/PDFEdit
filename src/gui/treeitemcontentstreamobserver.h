#ifndef __TREEITEMCONTENTSTREAMOBSERVER_H__
#define __TREEITEMCONTENTSTREAMOBSERVER_H__

#include <ccontentstream.h>
#include <observer.h>

namespace gui {

class TreeItemAbstract;

using namespace pdfobjects;

/** Observer for CContentstream items */
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
