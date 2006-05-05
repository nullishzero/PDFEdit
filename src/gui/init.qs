//This script will be run at start, or on creation of new window
//Each window have its own scripting context

/** Create new empty editor window */
function newwindow() {
 createNewWindow();
}

/** Toggle on/off boolean setting with specified key */
function toggle(key) {
 val=settings.readBool(key);
 settings.write(key,!val);
}

/** Save (action from menu/toolbar) */
function func_save() {
 save();
}

/** Save as (action from menu/toolbar). Asks for name, then saves under new name */
function func_saveas() {
 var name=fileSaveDialog(filename());
 if (!name) return;
 print("Saving as "+name);
 saveAs(name);
}

/** Open new file (action from menu/toolbar) */
function func_load() {
 try {
  if (!closeFile(true,true)) return;
  var name=fileOpenDialog();
  if (name) openFile(name);
 } catch (e) {
  print("Error occured while loading file");
 }
}

/** close current file and create a new one (action from menu/toolbar) */
function func_new() {
 closeFile(true);
}

/** Go to page with number x in document. If parameter is empty, current page is reloaded */
function go(x) {
 PageSpace.refresh(x,document);
}

/** Callback called after document is loaded */
function onLoad() {
 //show first page
 PageSpace.refresh(document.getFirstPage(),document);
}

/** Check if treeitem holds a container (Array, Dict)
*/
function holdContainer(ti) {
 type=ti.itemtype();
 if (type=='Dict' || type=='Array') {
  return true;
 }
 return false;
}

/** Callback for click with right mouse button in tree window */
function onTreeRightClick() {
 menu=popupMenu("popup_generic");
 menu.addSeparator();
 if (holdContainer(treeitem))
   menu.addItemDef("item Add object to "+treeitem.itemtype()+",addObjectDialog()");
 menu.addItemDef("item ("+treeitem.itemtype()+"),");
 eval(menu.popup());
}

/** Callback for click with left mouse button in tree window */
function onTreeLeftClick() {
// print('Left click, type of item = '+treeitem.itemtype());
}

/** Callback for click with middle mouse button in tree window */
function onTreeMiddleClick() {
 treeitem.reload();
}

/** Callback for doubleclick with left mouse button in tree window */
function onTreeDoubleClick() {
// print("Doubleclick, type of item = "+treeitem.itemtype());
}

/** Print names of childs of currently selected tree item to console */
function printTreeChilds() {
 names=treeitem.getChildNames();
 for(var i=0;i<names.length;i++) {
  print(names[i]);
 }
}

/** Set zoom level to x percent */
function zoom(x) {
 PageSpace.zoomTo(x);
}

/** invoke "add object dialog" on current tree item, or if not possible, try its parent */
function add_obj_dlg() {
 if (holdContainer(treeitem)) addObjectDialog();
 if (holdContainer(treeitem.parent())) addObjectDialog(treeitem.parent().item());
}

//Print welcome message
print("PDF Editor "+version());
