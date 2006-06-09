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
    warn(tr('Warning: This document is encrypted!')+"\n"+
         tr('Encryption filter:')+" "+enc+"\n"+
         tr('Encrypted content will show up as garbage and many operations will be impossible'));
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
 else if (holdContainer(treeitem.parent())) addObjectDialog(treeitem.parent().item());
}

/** Save page/selection as image */
function savePageImage(onlySelection) {
 if (!PageSpace.saveImageWithDialog(onlySelection)) print(tr("Image was not saved!"));
}

/** Print to console and evaluate */
function print_eval(x) {
 if (!x.length) return;
 print("> "+x);
 eval(x);
}

/** Set zoom level to x percent */
function zoom(x) {
 PageSpace.zoomTo(x);
}

/** Go to page with number x in document. If parameter is empty, current page is reloaded */
function go(x) {
 PageSpace.refresh(x,document);
}

/** Set icons to show that the mode 'newmode' is selected */
function activateMode(newMode) {
 checkItem("streamAll",(newMode=='all'));
 checkItem("streamText",(newMode=='text'));
 checkItem("streamFont",(newMode=='font'));
 checkItem("streamGfx",(newMode=='graphic'));
}

/** Set stream to certain mode */
function streamMode(newMode) {
 x=treeRoot();
 if (x.itemtype()!="ContentStream") return;
 x.setMode(newMode);
 activateMode(newMode);
}

/** Check if treeitem holds a container (Array, Dict) */
function holdContainer(ti) {
 type=ti.itemtype();
 if (type=='Dict' || type=='Array') {
  return true;
 }
 return false;
}
