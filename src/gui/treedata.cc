/** @file
 TreeItem - class holding Tree Data that need to be shared between tree window and its items
 @author Martin Petricek
*/

#include "treedata.h"
#include "treewindow.h"
#include "treeitem.h"
#include "treeitemref.h"
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
 show_simple=false;
 update();
 dirty=false;
 needreload=false;
}

/** Check if setting have changed, updating if necessary.
 If setting is changed, set dirty to true.
 @param key Setting to check
 @param target Pointer to setting to check
*/
void TreeData::checkSetting(bool &target,const QString &key) {
 bool tmp=globalSettings->readBool(key);
 if (target==tmp) return;
 printDbg(debug::DBG_DBG,"Tree settings check failed: " << key);
 target=tmp;
 dirty=true;
 needreload=true;
}

/** update internal data from settings */
void TreeData::update() {
 checkSetting(show_simple,"tree/show_simple");
}

/** Return value of show_simple setting
 @return True if show_simple is set, false if not
 */
bool TreeData::showSimple() {
 return show_simple;
}

/** Return true, if the tree should be reloaded because the settings changed
 since last time the tree was reloaded, false if the tree does not need reloading
 @return True if tree is to be reloaded
 */
bool TreeData::needReload() {
 return needreload;
}

/** Resets the "tree need to be reloaded" flag */
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
void TreeData::add(TreeItemRef *it) {
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
void TreeData::remove(TreeItemRef *it) {
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
TreeItemRef* TreeData::find(const QString &ref) {
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
