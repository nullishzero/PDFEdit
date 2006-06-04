/** @file
 TreeItemPageObserver - class providing observer for one tree item, that will reload the specified item on any changes
 @author Martin Petricek
*/

#include "treeitempageobserver.h"
#include "treeitemabstract.h"
#include "util.h"

namespace gui {

using namespace std;

/**
 Constructor
 @param _parent Object to be reloaded on any change to monitored CPage
*/
TreeItemPageObserver::TreeItemPageObserver(TreeItemAbstract* _parent){
 parent=_parent;
};

/** Deactivate observer */
void TreeItemPageObserver::deactivate() {
 parent=NULL;
}

/**
 Notification function called by changing property
 @param newValue New value of property
 @param context Context of change
*/
void TreeItemPageObserver::notify (__attribute__((unused)) boost::shared_ptr<CPage> newValue, __attribute__((unused)) boost::shared_ptr<const CPage::ObserverContext> context) const throw() {
 if (!parent) {
  //Should never happen
  guiPrintDbg(debug::DBG_ERR,"BUG: Kernel is holding observer for item already destroyed");
  assert(parent);
  return;
 }
 //Reload contents of parent
 parent->reload();
}

/**
 Return priority of this observer
 @return priority value
*/
CPage::Observer::priority_t TreeItemPageObserver::getPriority() const throw(){
 return 0;//TODO: what priority?
}

/** Destructor */
TreeItemPageObserver::~TreeItemPageObserver() throw(){
 //Empty for now
};

} // namespace gui
