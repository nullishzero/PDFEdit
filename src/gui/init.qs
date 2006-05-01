//this script will be run at start

function quit() {
 //todo: do another way
 exitApp();
}

function closewindow() {
 //todo: do another way
 closeWindow();
}

function newwindow() {
 createNewWindow();
}

//Toggle given boolean setting
function toggle(key) {
 val=settings.readBool(key);
 settings.write(key,!val);
}

//Save from menu/toolbar
function func_save() {
 save();
}

//Save as from menu/toolbar
function func_saveas() {
 var name=fileSaveDialog(filename());
 if (!name) return;
 print("Saving as "+name);
 saveAs(name);
}

//Load from menu/toolbar
function func_load() {
 try {
  if (!closeFile(true,true)) return;
  var name=fileOpenDialog();
  if (name) openFile(name);
 } catch (e) {
  print("Error occured while loading file");
 }
}

//Load from menu/toolbar
function func_new() {
 closeFile(true);
}

//Go to page number x
function go(x) {
 PageSpace.refresh(x,document);
}

//Called after document is loaded
function onLoad() {
 //show first page
 PageSpace.refresh(document.getFirstPage(),document);
}

//Handler for click with right mouse button
function onTreeRightClick() {
// print('Right click, type of item = '+treeitem.itemtype());
 menu=popupMenu("popup_generic");
 menu.addSeparator();
 type=treeitem.itemtype();
 if (type=='Dict' || type=='Array') {
  menu.addItemDef("item Add object to "+treeitem.itemtype()+",addObjectDialog()");
 }
 menu.addItemDef("item ("+treeitem.itemtype()+"),");
 eval(menu.popup());
}

//Handler for click with left mouse button
function onTreeLeftClick() {
// print('Left click, type of item = '+treeitem.itemtype());
}

//Handler for click with middle mouse button
function onTreeMiddleClick() {
// print('Middle click, type of item = '+treeitem.itemtype());
 treeitem.reload();
}

//Handler for doubleclick with left mouse button
function onTreeDoubleClick() {
// print("Doubleclick, type of item = "+treeitem.itemtype());
}

//Print tree childs
function printTreeChilds() {
 names=treeitem.getChildNames();
 for(var i=0;i<names.length;i++) {
  print(names[i]);
 }
}

//Set zoom level
function zoom(x) {
 PageSpace.zoomTo(x);
}

print("PDF Editor "+version());
