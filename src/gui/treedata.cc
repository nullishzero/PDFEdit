/** @file
 TreeItem - class holding Tree Data that need to be shared between tree window and its items
*/

#include "treedata.h"
#include "treewindow.h"
#include "treeitem.h"

namespace gui {

using namespace std;

/** Constructor of TreeData
 @param _parent TreeWindow holding these data
 @param tree Tree holding list items
*/
TreeData::TreeData(TreeWindow *parent,QListView *tree) {
 _parent=parent;
 _tree=tree;
}

//TODO: list of opened items based on ref.
//TODO: reopening item already opened in other part of tree (ref)
//TODO: should only reparent it as child of opened ref ....

/** Add TreeItem that holds a CRef into list
 Will do nothing on TreeItems that do not hold a CRef.
 @param it TreeItem
 */
void TreeData::add(TreeItem *it) {
//TODO:implement;
}

/** Look in list for a treeItem with given reference and return it.
 @param ref Reference in string format
 @return TreeItem with given reference, or NULL if nothing found
*/
TreeItem* TreeData::find(const QString &ref) {
//TODO:implement;
}

/** Clear list of TreeItems with CRef*/
void TreeData::clear() {
//TODO:implement;
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
