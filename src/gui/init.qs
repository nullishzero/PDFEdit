//PDF Editor init script
//This script will be run at start, or on creation of new window
//Each window have its own scripting context

/* ==== load various needed functions ==== */
run( "pdfoperator_utilities.qs" );
run( "dialog_lib.qs" );
run( "dialogs.qs" );
run( "test.qs" );

/* ==== Callback functions ==== */

/** Callback called after document is loaded */
function onLoad() {
 //show first page
// go(1);
 PageSpace.refresh(document.getFirstPage(),document);
 checkMenus();
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
 treeitemtype=treeitem.itemtype();
// menu.addSeparator();
// menu.addItemDef("item ("+treeitem.itemtype()+"),");
 if (holdContainer(treeitem)) { // Dict, Array
  menu.addSeparator();
  if (treeitemtype=="Dict") {
   str_tr=tr("Add item to dictionary");
  } else {
   str_tr=tr("Add element to array");
  }
  menu.addItemDef("item "+str_tr+",addObjectDialog(),,item_add.png");
 }
 if (treeitem.itemtype()=="PdfOperator") {
  menu.addSeparator();
  menu.addItemDef("item "+tr("Set color")+",setColor(),,operator_setcolor.png");
  menu.addItemDef("item "+tr("Set font properties")+",editFontProps(),,operator_editfont.png");
  menu.addItemDef("item "+tr("Set relative position of a single operator")+",moveOperPos(),,operator_setrelposop.png");
  menu.addItemDef("item "+tr("Set relative position")+",moveTextPos(),,operator_setrelpos.png");
  menu.addSeparator();
  menu.addItemDef("item "+tr("Set line dash style")+",setDashPattern(),,operator_dashpattern.png");
  menu.addItemDef("item "+tr("Set line width")+",setLineWidth(),,operator_linewidth.png");
 }
 if (treeitem.itemtype()=="Page") {
  menu.addSeparator();
  menu.addItemDef("item "+tr("Go to page")+" "+treeitem.id()+",go("+treeitem.id()+")");
  menu.addItemDef("item "+tr("Add system font")+",addSystemFont(),,page_add_font.png");
  menu.addItemDef("item "+tr("Extract text from page")+",viewPageText(),,page_text.png");
  menu.addItemDef("item "+tr("Set page tranformation matrix")+",setPageTm(),,page_settm.png");
 }
 if (treeitem.itemtype()=="ContentStream") {
  menu.addSeparator();
  menu.addItemDef("item "+tr("Show all operators")+",treeitem.setMode('all'),,stream_mode_all.png");
  menu.addItemDef("item "+tr("Show only text operators")+",treeitem.setMode('text'),,stream_mode_text.png");
  menu.addItemDef("item "+tr("Show only font operators")+",treeitem.setMode('font'),,stream_mode_font.png");
  menu.addItemDef("item "+tr("Show only graphical operators")+",treeitem.setMode('graphic'),,stream_mode_gfx.png");
 }
 if (tests) {
  if (treeitem.itemtype()=="Stream") test_stream_items(menu);
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
 menu.addItemDef("item "+tr("Save page as image")+",savePageImage(false)");
 if (PageSpace.isSomeoneSelected())
   menu.addItemDef("item "+tr("Save selected area as image")+",savePageImage(true)");
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
 checkMenus();
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


/** Save a copy (action from menu/toolbar). Asks for name, then saves under new name */
function func_savecopy() {
 var name=fileSaveDialog(filename());
 if (!name) return;
 print(tr("Saving as")+" "+name);
 saveCopy(name);
}

/** Open new file (action from menu/toolbar) */
function func_load() {
 if (!closeFile(true,true)) return;
 var name=fileOpenDialog();
 try {
  if (name) {
   openFile(name);
   if (document.isLinearized()) {
    print(tr('Warning: This document is linearized PDF!'));
   }
   enc=document.encryption();
   if (enc) {
    print(tr('Warning: This document is encrypted!'));
    print(tr('Encryption filter:')+enc);
   }
  }
 } catch (e) {
  print(tr("Error occured while loading file")+" "+name);
 }
}

/** close current file and create a new one (action from menu/toolbar) */
function func_new() {
 closeFile(true);
 checkMenus();
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

/** Function to enable/disable certain items when currently selected object changes */
function checkMenus() {
 try {
  if (treeitem) {
   //Check current tree item
   theType=treeitem.itemtype();
  } else {
   theType="";
  }
 } catch (e) {
  //This is mainly because treeitem is not defined on start
  theType="";
 }
 try {
  //Check for document being opened
  if (document) have_document=true;  else have_document=false;
 } catch (e) {
  have_document=false;
 }
 try {
  //Check tree root
  rootType=treeRoot().itemtype();
 } catch (e) {
  rootType="";
 }
 enableItem("/need_operator_or_page",	(theType=="PDFOperator" || theType=="Page"));
 enableItem("/need_dict_or_array",	(theType=="Dict" || theType=="Array"));
 enableItem("/need_contentstream_root",	(rootType=="ContentStream"));
 enableItem("/need_document",	(have_document));

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
 return page().getDictionary();
}

/** === Validate functions === */

/** Is argument a number */
function isNumber(x) {
	if (undefined == parseFloat(x))
		return false;
	else
		return true;
}

/** Validate current page */
function isPageAvaliable() {
 if (page()) {
	return true;
 }else
 	return false;
}

/** Validate item selection */
function isTreeItemSelected() {
 if (undefined == treeitem) {
	return false;
 }else
 	return true;
}

/** Return property from dictionary of current page */
function pageProperty(propName) {
 if (isPageAvaliable())
	return page().getDictionary().property(propName).ref();
 else
 	warn(tr("No page selected!"));
}

/**
 Return property from dictionary of current page,
 adding it with default value if property is not found
*/
function pagePropertyDef(propName,defValue) {
 if (isPageAvaliable())
	return page().getDictionary().propertyDef(propName,defValue).ref();
 else
 	warn(tr("No page selected!"));
}

/** Set stream to certain mode */
function streamMode(newMode) {
 x=treeRoot();
 if (x.itemtype()!="ContentStream") return;
 x.setMode(newMode);
}


/* === Page changing functions === */

/** rotate current page N degrees clockwise */
function rotatePage(n) {
 if (!isPageAvaliable()) {
 	warn(tr("No page selected!"));
 	return;
 }
//Get page rotation
 rotate=pagePropertyDef("Rotate",0);
 //Add rotation
 n0=rotate.getInt()+n;
 //Normalize
 if (n0<0) n0+=360;
 if (n0>=360) n0-=360;
 //Set
 rotate.set(n0);
 print (tr("Page rotated."));
 //Reload page after rotating
 go();
}

/* ==== Code to run on start ==== */

//Print welcome message
print("PDF Editor "+version());
checkMenus();
