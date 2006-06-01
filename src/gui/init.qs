//PDF Editor init script
//This script will be run at start, or on creation of new window
//Each window have its own scripting context

/* ==== load utilities ==== */
run( "pdfoperator_utilities.qs" );

/* ==== Callback functions ==== */

/** Callback called after document is loaded */
function onLoad() {
 //show first page
// go(1);
 PageSpace.refresh(document.getFirstPage(),document);
}

/** Callback called after error occured while trying to load document */
function onLoadError() {
 warn(error());
}

/** Same-treeview dragdrop handler */
function onDragDrop() {
 print("Drag drop: "+source.type()+" -> "+target.type());
}

/** Different-treeview dragdrop handler */
function onDragDropOther() {
 print("Drag drop: "+source.type()+" -> "+target.type());
}

/** Callback for click with right mouse button in tree window */
function onTreeRightClick() {
 menu=popupMenu("popup_generic");
 menu.addSeparator();
 if (holdContainer(treeitem)) { // Dict, Array
  menu.addItemDef("item Add object to "+treeitem.itemtype()+",addObjectDialog(),,item_add.png");
 }
 menu.addItemDef("item ("+treeitem.itemtype()+"),");
 menu.addSeparator();
 if (treeitem.itemtype()=="PdfOperator") {
  menu.addItemDef("item Set color,setColor()");
 }
 if (treeitem.itemtype()=="Page") {
  menu.addItemDef("item "+tr("Go to page")+" "+treeitem.id()+",go("+treeitem.id()+")");
 }
 if (treeitem.itemtype()=="ContentStream") {
  menu.addItemDef("item "+tr("Show all operators")+",treeitem.setMode('all'),,stream_mode_all.png");
  menu.addItemDef("item "+tr("Show only text operators")+",treeitem.setMode('text'),,stream_mode_text.png");
  menu.addItemDef("item "+tr("Show only font operators")+",treeitem.setMode('font'),,stream_mode_font.png");
 }
 if (tests) {
  if (treeitem.itemtype()=="Stream") {
   menu.addItemDef("item Decoded representation,print(treeitem.item().getDecoded())");
   menu.addItemDef("item Stream integrity test\\, array,buftest(treeitem.item()\\,1\\,0)");
   menu.addItemDef("item Stream integrity test\\, string,buftest(treeitem.item()\\,0\\,1)");
   menu.addItemDef("item Stream integrity test\\, both,buftest(treeitem.item()\\,1\\,1)");
  }
 }
 print_eval(menu.popup());
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

/** Callback for click with right mouse button in page */
function onPageRightClick() {
 menu=popupMenu();
 menu.addSeparator();
 menu.addItemDef("item Save page as image,savePageImage(false)");
 if (PageSpace.isSomeoneSelected())
   menu.addItemDef("item Save selected area as image,savePageImage(true)");
 menu.addSeparator();
 print_eval(menu.popup());
}

/**
 Callback called after document is loaded and after onLoad.
 This function is empty and to be re-defined by user if necessary
*/
function onLoadUser() {
 //Dummy, will be overridden by whatever user specify
}

/** Called when active revision was changed */
function onChangeRevision() {
 print(tr("Changed revision to:")+activeRevision());
}

/** Callback when selection in tree (which item is selected) changes */
function onTreeSelectionChange() {
 //print('SelectionChange, type of item = '+treeitem.itemtype());
}

/* ==== Functions called directly from menu ==== */

/** Save (action from menu/toolbar) */
function func_save() {
 if (!save()) {
  err=error();
  warn(err);
 }
}

/** Save new revision (action from menu/toolbar) */
function func_saverev() {
 if (!saveRevision()) {
  err=error();
  warn(err);
 } else {
  print(tr("New revision of document created"));
 }
}

/** Show/hide certain window (toggle) */
function showHide(win) {
 vis=isVisible(win);
 setVisible(win,!vis);
}

/** Perform "set color" operation on currently selected operator */
function setColor() {
 op=treeitem.item();
 stream=op.stream();
 col=pickColor();
 if (!col) return;//Dialog aborted
 stream.setColor(op,col);
 //reload the page
 go();
}

/** Save a copy (action from menu/toolbar). Asks for name, then saves under new name */
function func_savecopy() {
 var name=fileSaveDialog(filename());
 if (!name) return;
 print(tr("Saving as")+" "+name);
 saveCopy(name);
}

/** Open new file (action from menu/toolbar) */
function func_load() {
 try {
  if (!closeFile(true,true)) return;
  var name=fileOpenDialog();
  if (name) {
   openFile(name);
   if (document.isLinearized()) {
    print(tr('Warning: This document is linearized PDF!'));
   }
  }
 } catch (e) {
  print(tr("Error occured while loading file")+" "+name);
 }
}

/** close current file and create a new one (action from menu/toolbar) */
function func_new() {
 closeFile(true);
}

/** Create new empty editor window */
function newwindow() {
 createNewWindow();
}

/** Print names of childs of currently selected tree item to console */
function printTreeChilds() {
 names=treeitem.getChildNames();
 for(var i=0;i<names.length;i++) {
  print(names[i]);
 }
}

/** invoke "add object dialog" on current tree item, or if not possible, try its parent */
function add_obj_dlg() {
 if (holdContainer(treeitem)) addObjectDialog();
 if (holdContainer(treeitem.parent())) addObjectDialog(treeitem.parent().item());
}

/* ==== Other helper functions ==== */

/** Turn on/off testing/debugging. */
function setTests(x) {
 settings.write("tests",x);
 tests=x;
}

/** Toggle on/off boolean setting with specified key */
function toggle(key) {
 val=settings.readBool(key);
 settings.write(key,!val);
}

/** Go to page with number x in document. If parameter is empty, current page is reloaded */
function go(x) {
 PageSpace.refresh(x,document);
}

/** Check if treeitem holds a container (Array, Dict) */
function holdContainer(ti) {
 type=ti.itemtype();
 if (type=='Dict' || type=='Array') {
  return true;
 }
 return false;
}

/** Save page/selection as image */
function savePageImage(onlySelection) {
 if (!PageSpace.saveImageWithDialog(onlySelection)) print(tr("Image was not saved!"));
}

/** Print to console and evaluate */
function print_eval(x) {
 print("> "+x);
 eval(x);
}

/** Set zoom level to x percent */
function zoom(x) {
 PageSpace.zoomTo(x);
}

/** Get dictionary from page number X */
function pageDict(x) {
 page=document.getPage(x);
 return page.getDictionary();
}

/** TEST: buffer integrity */
function buftest(x,at,st) {
 if (at) {
  a1=x.getBuffer();
  x.setBuffer(a1);
  a2=x.getBuffer();
  if (a1.length!=a2.length) {
   print ("Different length of arrays: "+a1.length+" vs "+a2.length);
  } else if (a1!=a2) {
   print("Different arrays: a1="+a1.length);
   print(a1)
   print("Different arrays: a2="+a2.length);
   print(a2);
  }
  print("Done array test");
 }
 if (st) {
  s1=x.getBufferString();
  x.setBuffer(s1);
  s2=x.getBufferString();
  if (s1.length!=s2.length) {
   print ("Different length of strings: "+s1.length+" vs "+s2.length);
  } else if (s1!=s2) {
   print("Different strings: s1="+s1.length);
   print(s1)
   print("Different strings: s2="+s2.length);
   print(s2);
  }
  print("Done string test");
 }
}

/** Return property from dictionary of current page */
function pageProperty(propName) {
 return page.getDictionary().property(propName).ref();
}

/**
 Return property from dictionary of current page,
 adding it with default value if property is not found
*/
function pagePropertyDef(propName,defValue) {
 return page.getDictionary().propertyDef(propName,defValue).ref();
}

/** rotate current page N degrees clockwise */
function rotatePage(n) {
 //Get page rotation
 rotate=pagePropertyDef("Rotate",0);
 //Add rotation
 n0=rotate.getInt()+n;
 //Normalize
 if (n0<0) n0+=360;
 if (n0>=360) n0-=360;
 //Set
 rotate.set(n0);
 //Reload page after rotating
 go();
}

/** Set stream to certain mode */
function streamMode(newMode) {
 x=treeroot();
 if (x.itemtype()!="ContentStream") return;
 treeroot().setMode(newMode);
}

/* ==== Code to run on start ==== */

//Include testing/debugging items in menus?
var tests=settings.readBool("tests");

//Print welcome message
print("PDF Editor "+version());
