//This script will be run at start, or on creation of new window
//Each window have its own scripting context

//Include testing/debugging items in menus?
var tests=settings.readBool("tests");

/** Turn on/off testing/debugging. */
function setTests(x) {
 settings.write("tests",x);
 tests=x;
}

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
// go(1);
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
 menu.addSeparator();
 if (treeitem.itemtype()=="Page")
   menu.addItemDef("item Go to page "+treeitem.id()+",go("+treeitem.id()+")");
 if (tests) {
  if (treeitem.itemtype()=="Stream")
    menu.addItemDef("item Stream integrity test,buftest(treeitem.item())");
 }
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
 //If page, goto page
 if (treeitem.itemtype()=="Page") go(treeitem.id());
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

/** Get dictionary from page number X */
function pageDict(x) {
 page=document.getPage(x);
 return page.getDictionary();
}

//Print welcome message
print("PDF Editor "+version());

/** TEST: buffer integrity */
function buftest(x) {
 s1=x.getBuffer();
 x.setBuffer(s1);
 s2=x.getBuffer();
 if (s1.length!=s2.length) {
  print ("Different length of strings: "+s1.length+" vs "+s2.length);
 } else if (s1!=s2) {
  print("Different strings: s1="+s1.length);
  print(s1)
  print("Different strings: s2="+s2.length);
  print(s2);
 }
}

function onLoadUser() {
 //Dummy, will be overidden by whatever user specify
}