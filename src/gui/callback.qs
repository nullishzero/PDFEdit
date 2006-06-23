/* ==== Callback functions ==== */

/** Function to enable/disable certain items when currently selected object changes */
function checkMenus() {
 try {
   //Check current tree item
  theType=treeitem.itemtype();
 } catch (e) {
  //This is mainly because treeitem is not defined on start
  theType="";
 }
 try {
  //Check parent tree item
  parentType=treeitem.parent().itemtype();
 } catch (e) {
  //This is mainly because treeitem is not defined on start
  parentType="";
 }
 try {
  //Check for document being opened
  if (document) have_document=true;  else have_document=false;
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
 enableItem("/need_operator_page",	(theType=="PDFOperator" || have_page));
// enableItem("/need_dict_or_array",	(theType=="Dict" || theType=="Array"));
 enableItem("/need_dict_or_array_p",	(theType=="Dict" || theType=="Array" || parentType=="Dict" || parentType=="Array"));
 enableItem("/need_removable",		(theType!="Pdf" && theType!=""));
 enableItem("/need_page",		(have_page));
 enableItem("/need_document",		(have_document));
 enableItem("/need_contentstream_root",	cstream_tab);
 if (cstream_tab) {
  mod=treeRoot().getMode();
  activateMode(mod);
  //
 }
}

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
  menu.addItemDef("item "+tr("View page")+" "+treeitem.id()+",go("+treeitem.id()+")");
  menu.addItemDef("item "+tr("Add system font")+",addSystemFont(),,page_add_font.png");
  menu.addItemDef("item "+tr("Extract text from page")+",viewPageText(),,page_text.png");
  menu.addItemDef("item "+tr("Set page tranformation matrix")+",setPageTm(),,page_settm.png");
  menu.addItemDef("item "+tr("Draw line")+",initconnection(0),,draw_line.png");
  menu.addItemDef("item "+tr("Draw rect")+",initconnection(10),,draw_rect.png");
  menu.addItemDef("item "+tr("Add text")+",addText(),,add_text.png");
 }
 if (treeitem.itemtype()=="ContentStream" && treeRoot().itemtype()=="ContentStream") {
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
 print(tr("Changed revision to:")+document.getActualRevision());
}

/** Callback when selection in tree (which item is selected) changes */
function onTreeSelectionChange() {
 checkMenus();
}
