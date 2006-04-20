/** @file
 TreeItem - class holding Tree Data that need to be shared between tree window and its items
*/

#include "treedata.h"
#include "treewindow.h"
#include "treeitem.h"
#include "settings.h"

namespace gui {

using namespace std;

/** Constructor of TreeData
 @param parent TreeWindow holding these data
 @param tree Tree holding list items
*/
TreeData::TreeData(TreeWindow *parent,QListView *tree) {
 _parent=parent;
 _tree=tree;
 //ShowData
 update();
 dirty=false;
 needreload=false;
}

 /** Check if setting have changed, updating if necessary.
  If setting is changed, set dirty to true.
  @param key Setting to check
  @param target Pointer to setting to check
 */
 void TreeData::check(bool &target,const QString &key) {
  bool tmp=globalSettings->readBool(key);
  if (target==tmp) return;
  printDbg(debug::DBG_DBG,"Tree settings check failed: " << key);
  target=tmp;
  dirty=true;
  needreload=true;
 }
 
 /** update internal data from settings */
 void TreeData::update() {
  check(show_simple,"tree/show_simple");
 }

 /** Return show_simple setting
  @return True if show_simple is set, false if not
  */
 bool TreeData::showSimple() {
  return show_simple;
 }

 bool TreeData::needReload() {
  return needreload;
 }

 void TreeData::resetReload() {
  needreload=false;
 }
 /** Return state of 'dirty' flag
  @return variable 'dirty'
  */
 bool TreeData::isDirty() {
  return dirty;
 }

 /** Reset state of 'dirty' flag */
 void TreeData::resetDirty() {
  dirty=false;
 }
/** Add TreeItem that holds a CRef into list
 Will do nothing on TreeItems that do not hold a CRef.
 @param it TreeItem holding CRef
 */
void TreeData::add(TreeItem *it) {
 QString ref=it->getRef();
 if (ref.isNull()) return;
 //Remove any old data
 while (refs.remove(ref));
 //Add new data
 refs.insert(ref,it);
}

/** Remove TreeItem that holds a CRef from list
 Will do nothing on TreeItems that do not hold a CRef.
 @param it TreeItem with reference to remove
 */
void TreeData::remove(TreeItem *it) {
 remove(it->getRef());
}

/** Remove specific reference from the list
 (Useful to call after reference in object itself was changed to remove old reference))
 @param ref Reference to remove
 */
void TreeData::remove(const QString &ref) {
 if (ref.isNull()) return;
 while (refs.remove(ref));
}


/** Look in list for a treeItem with given reference and return it.
 @param ref Reference in string format
 @return TreeItem with given reference, or NULL if nothing found
*/
TreeItem* TreeData::find(const QString &ref) {
 return refs.find(ref);
}

/** Clear list of TreeItems with CRef*/
void TreeData::clear() {
 refs.clear();
}

/** Return parent of this TreeData
 @return parent TreeWindow */
TreeWindow* TreeData::parent() {
 return _parent;
}

/** Return tree from this TreeData
 @return QlistViewparent Tree */
QListView* TreeData::tree() {
 return _tree;
}

/** default destructor */
TreeData::~TreeData() {
}

} // namespace gui
