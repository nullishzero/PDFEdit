/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006-2009  PDFedit team: Michal Hocko,
 *                                        Jozef Misutka,
 *                                        Martin Petricek
 *                   Former team members: Miroslav Jahoda
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (in doc/LICENSE.GPL); if not, write to the 
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 * MA  02111-1307  USA
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit
 */
/* ==== Functions called directly from menu ==== */

/** Save (action from menu/toolbar) */
function func_save() {
 if (!save()) {
  err=error();
  warn(err);
  return false;
 }
 return true;
}

/** Save new revision (action from menu/toolbar) */
function func_saverev() {
 if (!saveRevision()) {
  err=error();
  warn(err);
  return false;
 } else {
  print(tr("New revision of document created"));
  return true;
 }
}

/** Show/hide certain window (toggle) */
function showHide(win,item) {
 vis=isVisible(win);
 setVisible(win,!vis);
 toggleVisItem(win,item);
}

/** Save a copy of the curent revision (action from menu/toolbar). Asks for name, then saves under new name */
function func_clonerevision() {
 var name=fileSaveDialog(filename());
 if (!name) return false;
 if (!saveCopy(name)) {
  err=error();
  warn(err);
  return false;
 } else {
  print(tr("Saved as")+" "+name);
  return true;
 }
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
   enc=document.isEncrypted();
   if (enc) {
    print(tr('Warning: This document is encrypted!'));
   }
  }
 } catch (e) {
  print(tr("Error occured while loading file")+" "+name);
 }
}

/** Save all text from document to single file */
function save_as_text(filename) {
 pages=document.getPageCount();
 qs="";
 for (i=1;i<=pages;i++) {
  pg=document.getPage(i);
  text=pg.getText();
  qs+=text;
  qs+="\n";
 }
 saveFile(filename,qs);
 print(tr("Text saved to %1").arg(filename));
}

/** Callback when selection mode is changed to draw new arrow */
function func_saveastext() {
 name=FileDialog.getSaveFileName("*.txt",tr("Save as text file"));
 if (name==undefined) return;
 save_as_text(name);
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
 names=firstSelectedItem().getChildNames();
 for(var i=0;i<names.length;i++) {
  print(names[i]);
 }
}

/** invoke "add object dialog" on current tree item, or if not possible, try its parent */
function add_obj_dlg() {
 if (holdContainer(firstSelectedItem())) addObjectDialog();
 else if (holdContainer(firstSelectedItem().parent())) addObjectDialog(firstSelectedItem().parent().item());
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
 if (document.getPageCount()==0) {
    print(tr("No pages in document"));
    return;
 }
 if (x == undefined) {
    tpage=page();
 } else {
    tpage=document.getPage(x);
 }
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

/** Check if tree item holds a container (Array, Dict) */
function holdContainer(ti) {
 type=ti.itemtype();
 if (type=='Dict' || type=='Array') {
  return true;
 }
 return false;
}

/** Delete all selected objects present in tree */
function deleteObjectsInTree() {
 /*
  Note:
  deleting the object MAY change the selection in tree in the process,
  so we need to store the objects first, then delete them all.
 */
 victims=[];
 treeItem=firstSelectedItem();
 i=0;
 while (treeItem) {
  if (treeItem) {
   victims[i]=treeItem;
   i++;
  }
  treeItem=nextSelectedItem();
 }
 //We have them, now delete them all
 for (i=0;i<victims.length;i++) {
  victims[i].remove();
//  print (victims[i].item().type());
 }
 //redraw the page
 go();
}
