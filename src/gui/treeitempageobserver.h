#ifndef __TREEITEMPAGEOBSERVER_H__
#define __TREEITEMPAGEOBSERVER_H__

#include <cpage.h>
#include <observer.h>

namespace gui {

class TreeItemAbstract;

using namespace pdfobjects;

/**
 This class provides observer monitoring CPage item.<br>
 The observer will reload associated tree item when the observed item changes.
*/
class TreeItemPageObserver : public CPage::Observer {
public:
 TreeItemPageObserver(TreeItemAbstract* _parent);
 void deactivate();
 virtual void notify (boost::shared_ptr<CPage> newValue, boost::shared_ptr<const CPage::ObserverContext> context) const throw();
 virtual CPage::Observer::priority_t getPriority() const throw();
 ~TreeItemPageObserver() throw();
protected:
 /** Parent object holding observed property*/
 TreeItemAbstract *parent;
};

} // namespace gui

#endif
