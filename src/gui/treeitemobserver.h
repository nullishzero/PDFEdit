#ifndef __TREEITEMOBSERVER_H__
#define __TREEITEMOBSERVER_H__

#include <iproperty.h>
#include <ccontentstream.h>
#include <observer.h>

namespace gui {

class TreeItemAbstract;

using namespace pdfobjects;

/**
 This class provides observer monitoring IProperty item.<br>
 The observer will reload associated tree item when the observed item changes.
*/
class TreeItemObserver : public IProperty::Observer {
public:
 TreeItemObserver(TreeItemAbstract* _parent);
 void deactivate();
 virtual void notify (boost::shared_ptr<IProperty> newValue, boost::shared_ptr<const IProperty::ObserverContext> context) const throw();
 virtual IProperty::Observer::priority_t getPriority() const throw();
 ~TreeItemObserver() throw();
protected:
 /** Parent object holding observed property*/
 TreeItemAbstract *parent;
};

} // namespace gui

#endif
