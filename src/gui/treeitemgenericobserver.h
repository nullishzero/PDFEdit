#ifndef __TREEITEMGENERICOBSERVER_H__
#define __TREEITEMGENERICOBSERVER_H__

#include "treeitemabstract.h"
#include "util.h"
#include <boost/shared_ptr.hpp>
#include <assert.h>
#include <observer.h>
#include <utils/debug.h>

namespace gui {

//template class

/*
template <typename T> struct ObserverItemTrait {
// public: typedef observer::ObserverHandler<T>::Observer		ObserverType;
 public: typedef boost::shared_ptr<T>				NewValueType;
 public: typedef boost::shared_ptr<const observer::ObserverHandler<T>::ObserverContext>	ObserverContextType;
};
*/

/**
 This template class provides observer monitoring some item.<br>
 That item must have ObserverContext and Observer types defined within it
 The observer will reload associated tree item when the observed item changes.
*/
template <typename ObservedItem>
class TreeItemGenericObserver : public observer::IObserver<ObservedItem> {
public:

 /**
  Constructor
  @param _parent Object to be reloaded on any change to monitored item
 */
 TreeItemGenericObserver(TreeItemAbstract* _parent){
  parent=_parent;
 };

 /** Deactivate observer */
 void deactivate() {
  parent=NULL;
 }

 /**
  Notification function called by changing property
  @param newValue New value of property
  @param context Context of change
 */
 virtual void notify (__attribute__((unused)) boost::shared_ptr<ObservedItem> newValue,
                      __attribute__((unused)) boost::shared_ptr<const observer::IChangeContext<ObservedItem> > context) const throw() {
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
 virtual observer::IObserver<int>::priority_t getPriority() const throw(){
  return 0;//TODO: what priority?
 }

 /** Destructor */
 virtual ~TreeItemGenericObserver() throw() {
 //Empty for now
 }

protected:
 /** Parent object holding observed property*/
 TreeItemAbstract *parent;
};

} // namespace gui

#endif
