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
/* ==== Callback functions ==== */

/** Function to enable/disable certain items when currently selected object changes */
function checkMenus() {
 //TODO: multiselect
 var first=firstSelectedItem();
 try {
   //Check current tree item
  theType=first.itemtype();
 } catch (e) {
  //This is mainly because tree item is not defined on start
  theType="";
 }
 try {
  //Check parent tree item
  parentType=first.parent().itemtype();
 } catch (e) {
  //This is mainly because tree item is not defined on start
  parentType="";
 }
 try {
  //Check for document being opened
  if (document.isValid()) have_document=true;  else have_document=false;
 } catch (e) {
  have_document=false;
 }
 try {
  //Check for page being opened
  if (page()) have_page=true;  else have_page=false;
 } catch (e) {
  have_page=false;
 }
 try {
  //Check tree root
  rootType=treeRoot().itemtype();
 } catch (e) {
  rootType="";
 }
 cstream_tab=(rootType=="ContentStream");
// enableItem("/mode_button",have_page);
 enableItem("/need_operator",		(theType=="PdfOperator"));
// enableItem("/need_dict_or_array",	(theType=="Dict" || theType=="Array"));
 enableItem("/need_dict_or_array_p",	(theType=="Dict" || theType=="Array" || parentType=="Dict" || parentType=="Array"));
 enableItem("/need_removable",		(theType!="Pdf" && theType!=""));
 enableItem("/need_page",		(have_page));
 enableItem("/need_document",		(have_document));
 enableItem("/need_rw",			(have_document && !document.isReadOnly()));
 enableItem("/need_page_rw",		(have_page && have_document && !document.isReadOnly()));
 showItem("/need_contentstream_root",	cstream_tab);
 enableItem("_zoom_tool",have_page);
 enableItem("_page_tool",have_document);
 if (cstream_tab) {
  mod=treeRoot().getMode();
  activateMode(mod);
  //
 }
}

/** Callback called after document is loaded */
function onLoad() {
 //show first page if present
 if (document.getPageCount()>0) PageSpace.refresh(document.getFirstPage(),document);
 checkMenus();
}

/** Callback called after new window is opened without opening file or after closing a file */
function onEmptyFile() {
 //just check menus and disable items as appropriate
 checkMenus();
}

/** Callback called after error occured while trying to load document */
function onLoadError() {
 warn(error());
}

/** Callback called when pressing "delete" key while in preview window */
function onPageDelete() {
// print("onPageDelete")
}

/** Force reloading of parent tree item. */
function parentReload() {
 //TODO: solve better
 firstSelectedItem().parent().reload();
}

/** Force reloading of root tree item. */
function rootReload() {
 //TODO: solve better
 treeRootMain().reload();
}

/** Callback for click with right mouse button in tree window */
function onTreeRightClick() {
 menu=popupMenu("popup_generic");
 //TODO: check for multiselect
 tree_item_type=firstSelectedItem().itemtype();
 if (holdContainer(firstSelectedItem())) { // Dict, Array
  menu.addSeparator();
  if (tree_item_type=="Dict") {
   str_tr=tr("Add item to dictionary");
  } else {
   str_tr=tr("Add element to array");
  }
  menu.addItemDef("item "+str_tr+",addObjectDialog(),,item_add.png");
 }
 if (tree_item_type=="PdfOperator") {
  menu.addSeparator();
  menu.addItemDef("item "+tr("Set color")+",setOpColor(),,operator_setcolor.png");
  menu.addItemDef("item "+tr("Set font properties")+",editFontProps(),,operator_editfont.png");
  menu.addItemDef("item "+tr("Set relative position of a single operator")+",moveOperPos(),,operator_setrelposop.png");
  menu.addItemDef("item "+tr("Set relative position")+",moveTextPos(),,operator_setrelpos.png");
  menu.addSeparator();
  menu.addItemDef("item "+tr("Set line dash style")+",setDashPattern(),,operator_dashpattern.png");
  menu.addItemDef("item "+tr("Set line width")+",setLineWidth(),,operator_linewidth.png");
 }
 if (tree_item_type=="Page") {
  tree_item_id=firstSelectedItem().id();
  menu.addSeparator();
  menu.addItemDef("item "+tr("View page")+" "+tree_item_id+",go("+tree_item_id+")");
  menu.addItemDef("item "+tr("Add system font")+",addSystemFont(),,page_add_font.png");
  menu.addItemDef("item "+tr("Extract text from page")+",viewPageText(),,page_text.png");
  menu.addItemDef("item "+tr("Set page tranformation matrix")+",setPageTm(),,page_settm.png");
//  menu.addItemDef("item "+tr("Draw line")+",initconnection(0),,draw_line.png");
//  menu.addItemDef("item "+tr("Draw rect")+",initconnection(10),,draw_rect.png");
  menu.addItemDef("item "+tr("Add text")+",addText(),,add_text.png");
  menu.addItemDef("item "+tr("Move page up",'pageMoveUp')+",movePage(firstSelected()\\,-1);parentReload(),,page_move_up.png");
  menu.addItemDef("item "+tr("Move page down",'pageMoveDown')+",movePage(firstSelected()\\,1);parentReload(),,page_move_down.png");
  menu.addItemDef("item "+tr("Remove page")+",removePageWithConditionalRefresh(),,page_delete.png");
 }
 if (tree_item_type=="ContentStream" && treeRoot().itemtype()=="ContentStream") {
  menu.addSeparator();
  menu.addItemDef("item "+tr("Show all operators")+",firstSelectedItem().setMode('all'),,stream_mode_all.png");
  menu.addItemDef("item "+tr("Show only text operators")+",firstSelectedItem().setMode('text'),,stream_mode_text.png");
  menu.addItemDef("item "+tr("Show only font operators")+",firstSelectedItem().setMode('font'),,stream_mode_font.png");
  menu.addItemDef("item "+tr("Show only graphical operators")+",firstSelectedItem().setMode('graphic'),,stream_mode_gfx.png");
 }
 if (tree_item_type=="ContentStream" && treeRoot().itemtype()!="ContentStream") {
  menu.addItemDef("item "+tr("Move one level up")+",currMoveAbove()");
  menu.addItemDef("item "+tr("Move one level down")+",currMoveBelow()");
 }
 if (tests) {
  if (tree_item_type=="Stream") test_stream_items(menu);
 }
 print_eval(menu.popup());
}

/** move current item above if it is contentstream*/
function currMoveAbove() {
 curr=firstSelectedItem();
 curr.parent().item().moveAbove(curr.item());
}

/** move current item below if it is contentstream*/
function currMoveBelow() {
 curr=firstSelectedItem();
 curr.parent().item().moveBelow(curr.item());
}

/** Remove page and redraw current page only if  */
function removePageWithConditionalRefresh() {
 ppos=document.getPagePosition(firstSelected());
 ppos_p=document.getPagePosition(page());
 removePage(ppos);
 if (ppos==ppos_p) {
  //We removed current page, so we need refreshing
  go();
 }
 parentReload();
}

/** Callback for click with left mouse button in tree window */
function onTreeLeftClick() {
// print('Left click, type of item = '+firstSelectedItem().itemtype());
}

/** Callback for changing value in any of special tools in toolbar */
function onValueChange(x) {
// print(x+' have changed');

 if (x == "text")
 	if (changeSelectedText ())
		go();
}

/** Callback for click with middle mouse button in tree window */
function onTreeMiddleClick() {
 //TODO: check for multiselect
 firstSelectedItem().reload();
}

/** Callback for doubleclick with left mouse button in tree window */
function onTreeDoubleClick() {
 // print("Doubleclick, type of item = "+firstSelectedItem().itemtype());
 //If page, goto page
 if (firstSelectedItem().itemtype()=="Page") go(firstSelectedItem().id());
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
 print(tr("Changed revision to:")+document.getActualRevision());
}

/** Callback when selection in tree (which item is selected) changes */
function onTreeSelectionChange() {
 checkMenus();

 if (fillTextFromSelectedOperators()) {
	showItem("_edit_tool text",true);
 } else {
	showItem("_edit_tool text",false);
 }
}

/** Set certain mode button to pressed while the others are unpressed */
function setModeButton(newmode) {
 checkItem("/mode_button",false);
 checkItem(newmode,true);
}

// When adding new mode, it is necessary for its button to have
// "mode_button" class for it to work correctly (i.e. others modes
// are deselected in GUI) and the mode must call:
// setModeButton(NAME_OF_BUTTON_OR_MENU_ITEM_USED_TO_SELECT_IT);
// when it is selected.

/** Callback when selection mode is changed to select all objects on page */
function onSelectMode_AllObjects() {
 PageSpace.setSelectionMode("operators_selection","rect");
 setModeButton('selectAllObjects');
}

/** Callback when selection mode is changed to select graphical operators on page */
function onSelectMode_GraphicalOperators() {
 PageSpace.setSelectionMode("graphical_operators","rect");
 setModeButton('selectGraphical');
}

/** Callback when selection mode is changed to select text operators only */
function onSelectMode_Text() {
 PageSpace.setSelectionMode("text_selection","rect2");
 setModeButton('selectText');
}

/** Callback when selection mode is changed to select text operators only for highlighting */
function onSelectMode_highlightingText() {
 PageSpace.setSelectionMode("text_marking","rect2","highlightingSelectedText(%1,%2,%3,%4,%5,%6)");
 setModeButton('highlightingtext');
}

/** Callback when selection mode is changed to select text operators only for strike trough */
function onSelectMode_strikeTrough( ) {
 PageSpace.setSelectionMode("text_marking","rect2","strikeTroughSelection(%1,%2,%3,%4,%5,%6)");
 setModeButton('striketrough');
}

/** Callback when selection mode is changed to select annotations on page */
function onSelectMode_Annot() {
 PageSpace.setSelectionMode("annotations","rect3","go_to_target_from_selected_annotation()");
 setModeButton('selectAnnot');
}

/** Callback when selection mode is changed to draw new line */
function onSelectMode_drawLine() {
 PageSpace.setSelectionMode("new_object","line","drawLine(%1,%2,%3,%4,false,%5,%6)");
 setModeButton('drawline');
}

/** Callback when selection mode is changed to draw new line */
function onSelectMode_drawLinee() {
 PageSpace.setSelectionMode("new_object","line","drawLine(%1,%2,%3,%4,true,%5,%6)");
 setModeButton('drawlinee');
}

/** Callback when selection mode is changed to draw new line */
function onSelectMode_drawArrow() {
 PageSpace.setSelectionMode("new_object","line","drawArrow(%1,%2,%3,%4,false,%5,%6)");
 setModeButton('drawarrow');
}

/** Callback when selection mode is changed to draw new line */
function onSelectMode_drawArrowe() {
 PageSpace.setSelectionMode("new_object","line","drawArrow(%1,%2,%3,%4,true,%5,%6)");
 setModeButton('drawarrowe');
}

/** Callback when selection mode is changed to draw new rectangle */
function onSelectMode_drawRect() {
 PageSpace.setSelectionMode("new_object","rect","drawRect(%1,%2,%3,%4,false,%5,%6)");
 setModeButton('drawrect');
}

/** Callback when selection mode is changed to draw new rectangle */
function onSelectMode_drawRecte() {
 PageSpace.setSelectionMode("new_object","rect","drawRect(%1,%2,%3,%4,true,%5,%6)");
 setModeButton('drawrecte');
}

/** Callback when selection mode is changed to add text to page */
function onSelectMode_addText() {
 PageSpace.setSelectionMode("new_object","rect","addText(%1,%2,%3,%4,%5,%6)");
 setModeButton('addtext');
}

/** Called when page in preview window is changed */
function onPageChange() {
 //print('page change');
 //System fonts
 stdfonts=getSystemFonts();
 //Fonts in page
 the_page=page();
 if (undefined == the_page)
   exfonts=[];
 else
   exfonts=the_page.getFontIdsAndNames(true);
 //Both can be used
 setPredefs('fontface',stdfonts.concat(exfonts));
}
