//PDF Editor init script
//This script will be run at start, or on creation of new window
//Each window have its own scripting context

/* ==== load utilities ==== */
run( "pdfoperator_utilities.qs" );
run( "dialogs.qs" );

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
  menu.addItemDef("item "+tr("Set color")+",setColor(),,operator_setcolor.png");
  menu.addItemDef("item "+tr("Set font properties")+",editFontProps(),,operator_editfont.png");
  menu.addItemDef("item "+tr("Set relative position of a single operator")+",moveOperPos(),,operator_setrelposop.png");
  menu.addItemDef("item "+tr("Set relative position")+",moveTextPos(),,operator_setrelpos.png");
  menu.addSeparator();
  menu.addItemDef("item "+tr("Set line dash style")+",setDashPattern(),,operator_dashpattern.png");
  menu.addItemDef("item "+tr("Set line width")+",setLineWidth(),,operator_linewidth.png");
 }
 if (treeitem.itemtype()=="Page") {
  menu.addItemDef("item "+tr("Go to page")+" "+treeitem.id()+",go("+treeitem.id()+")");
  menu.addItemDef("item "+tr("Add system font")+",addSystemFont(),,page_add_font.png");
  menu.addItemDef("item "+tr("Extract text from page")+",viewPageText(),,page_text.png");
  menu.addItemDef("item "+tr("Set page tranformation matrix")+",setPageTm(),,page_settm.png");
 }
 if (treeitem.itemtype()=="ContentStream") {
  menu.addItemDef("item "+tr("Show all operators")+",treeitem.setMode('all'),,stream_mode_all.png");
  menu.addItemDef("item "+tr("Show only text operators")+",treeitem.setMode('text'),,stream_mode_text.png");
  menu.addItemDef("item "+tr("Show only font operators")+",treeitem.setMode('font'),,stream_mode_font.png");
  menu.addItemDef("item "+tr("Show only graphical operators")+",treeitem.setMode('graphic'),,stream_mode_gfx.png");
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
 return page().getDictionary();
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

/** 
 * Show page rectangle and allow changing page metrics. 
 * It displayes MediaBox entry from a page dictionary and then sets new values if desired. 
 */
function editPageMediaBox() {

 if (!isPageAvaliable()) {
 	warn(tr("No page selected!"));
 	return;
 }
	
 // Get media box
 var mediabox = page().mediabox();
 var xleft = mediabox[0];
 var yleft = mediabox[1];
 var xright = mediabox[2];
 var yright = mediabox[3];

 var dialog = createDialog (tr("Change page rectangle"), tr("Change"), tr("Cancel"), tr("Page metrics"));
 
 var gb = createGroupBoxAndDisplay (tr("Page metrics"), dialog);
 var exl = createLineEditAndDisplay(tr("Left upper corner")+", "+tr("x position")+": ", xleft, gb);
 var eyl = createLineEditAndDisplay(tr("Left upper corner")+", "+tr("y position")+": ", yleft, gb);
 var exr = createLineEditAndDisplay(tr("Right bottom corner")+", "+tr("x position")+": ", xright, gb);
 var eyr = createLineEditAndDisplay(tr("Right bottom corner")+", "+tr("y position")+": ", yright, gb);

 if (!dialog.exec()) return;

 // Save media box
 page().setMediabox (exl.text, eyl.text, exr.text, eyr.text);
 print (tr("MediaBox changed."));
 go ();
}

/** Perform "set color" operation on currently selected operator */
function setColor() {
	
	if (!isPageAvaliable() || !(isTreeItemSelected())) {
		warn(tr("No page or operator selected!"));
		return;
	}
	
	op=treeitem.item();
	if (!isChangeableOp(op)) {
		warn(tr("Selected operator")+" "+tr("is not")+" "+tr("suitable for colour setting.")+tr(" Please see the pdf specification for details."));
		return;
	}

	stream=op.stream();
	col=pickColor();
	if (!col) return;//Dialog aborted
	operatorSetColor(op,col.red, col.green, col.blue);
	print (tr("Colour changed."));
	//reload the page
	go();
}


/** Change dash style of an operator. */
function setLineWidth() {
	
	if (!isPageAvaliable() || !(isTreeItemSelected())) {
		warn(tr("No page or operator selected!"));
		return;
	}
	
	op=treeitem.item();
	if (!isChangeableOp(op)) {
		warn(tr("Selected operator")+" "+tr("is not")+" "+tr("suitable for line width change.")+tr(" Please see the pdf specification for details."));
		return;
	}
	
	var dialog = createDialog (tr("Change line width"), tr("Change"), tr("Cancel"), tr("Change line width"));
	
	var gb = createGroupBoxAndDisplay ("Line width", dialog);
	var sb = createSpinboxAndDisplay ("Line width", 0, 100,gb);
	 
	gb = createGroupBoxAndDisplay (tr("Change effect"), dialog);
	var glob = createCheckBoxAndDisplay (tr("Global change"),gb);
	glob.checked = true;

	if (!dialog.exec()) return;
	 
	operatorSetLineWidth(op,sb.value,glob.checked);
	print (tr("Line width changed."));
	// Reload page
	go ();
}


/** Change dash style of an operator. */
function setDashPattern() {
	
	if (!isPageAvaliable() || !(isTreeItemSelected())) {
		warn(tr("No page or operator selected!"));
		return;
	}
	
	op=treeitem.item();
	if (!isChangeableOp(op)) {
		warn(tr("Operator")+" "+tr("is not")+" "+tr("suitable for dash style change.")+tr(" Please see the pdf specification for details."));
		return;
	}
	
	var dialog = createDialog (tr("Change line dash pattern"), tr("Change"), tr("Cancel"), tr("Change line dash pattern"));
	 
	var gb = createGroupBoxAndDisplay (tr("Line dashing patterns"), dialog);
	var rb = [];
	rb[0] = createRadioButtonAndDisplay (tr("Solid line"),gb);
	rb[1] = createRadioButtonAndDisplay (tr("Slightly dashed"),gb);
	rb[2] = createRadioButtonAndDisplay (tr("Dot and dashed"),gb);
	
	gb = createGroupBoxAndDisplay (tr("Change effect"), dialog);
	var glob = createCheckBoxAndDisplay (tr("Global change"),gb);
	glob.checked = true;
	 
	if (!dialog.exec()) return;
	 
	var i = 0;
	for(; i<rb.length; ++i)
		if (rb[i].checked)
			break;
	
	if (!operatorSetSimpleDashPattern(i,op,glob.checked)) {
		return;
	}

	print (tr("Dash pattern changed."));
	// Reload page
	go ();
}

/**
 * Display and allow to change font atributes of a text operator.
 */
function editFontProps() {

	if (!isPageAvaliable() || !(isTreeItemSelected())) {
		warn(tr("No page or operator selected!"));
		return;
	}

	// Get selected item
 	op=treeitem.item();

	if (!isTextOp(op)) {
 		warn(tr("Not valid")+" "+tr("text operator")+". "+tr("Only text operators allowed!"));
		return;
	}
	
	var fontspecs = operatorGetFont(op);
	var fontsize = fontspecs[1];
	var fontname = fontspecs[0];
	
	var dg = createDialog (tr("Change font properties"), tr("Change font"), tr("Cancel"), tr("Change font"));
	var gb = createGroupBoxAndDisplay (tr("Avaliable fonts"),dg);
	var cb = new ComboBox;
	cb.label = tr("Select from all avaliable fonts");
	
	// Put all avaliable fonts here and select the operator font 
	var fonts = page().getFontIdsAndNames();
	var fontnames = new Array(fonts.length/2);
	
	// Fill fontnames with symbolic font names
	for(i = 1,j=0; i < fonts.length; ++i) {
        fontnames[j] = fonts[i];
		// Save symbolic name of operator font
		if (fonts[i-1] == fontname)
			fontname = fonts[i];
		// Skip id
		++i;++j;
    }
	cb.itemList = fontnames;
	cb.currentItem = fontname;
	gb.add (cb);
	
	var gb1 = createGroupBoxAndDisplay (tr("Font size according to the pdf document"),dg);
	var sb = createNumbereditAndDisplay (tr("Font size (not in px)"), 0, 100, gb1);
	print(fontsize);
	sb.value = fontsize;

	if (!dg.exec()) return;
	
	// Get font metrics and change the operator
	var newfontsize = sb.value;
	var newfontname = cb.currentItem;// get Idx according to a name

	for(i = 1; i < fonts.length; ++i) {
		// Save symbolic name of operator font
		if (fonts[i] == newfontname) {
			newfontid = fonts[i-1];
			break;
		}
		// Skip id
		++i;
     }
	// Set the font
	operatorSetFont(op,newfontid,newfontsize);
	print (tr("Font changed."));
	go();
}

/**
 * Add system font to a page.
 */
function addSystemFont() {

	if (!isPageAvaliable()) {
 		warn(tr("No page selected!"));
		return;
	}

	var dg = createDialog (tr("Add system font"), tr("Add font"), tr("Cancel"), tr("Add system fonts"));
	var gb = createGroupBoxAndDisplay (tr("Avaliable system fonts"),dg);
	var cb = new ComboBox;
	// Put values from pdf specification here
	cb.label= tr("Select system Type1 font");
	cb.itemList = getSystemFonts ();
	cb.editable = true;
	gb.add (cb);
	
	if (!dg.exec()) return;

	// add system font
	page().addSystemType1Font (cb.currentItem);
	print (tr("System font added."));
}

/**
 * Display text on a page.
 */
function viewPageText() {
	
	if (!isPageAvaliable()) {	
 		warn(tr("No page selected!"));
		return;
	}

	
	dg = createDialog (tr("Text on page"), tr("Ok"), tr("Cancel"), tr("Add system fonts"));
	var gb = createGroupBoxAndDisplay (tr("Text extracted from page"),dg);
	te = new TextEdit;
	te.text = page().getText();

	max = 0;
	lines = te.text.split("\n");
	for (i=0; i<lines.length; ++i) {
		if (lines[i].length > max)
			max = lines[i].length		
	}
	// Set width according to line length (we mutliply it with a magic constane somthing like character width)
	dg.width = max * 7;
	
	te.tooltip = "Text on a page.";
	gb.add (te);

	dg.exec();
	print (tr("Page text displayed"));
}

/**
 * Change page transform matrix.
 */
function setPageTm() {

	if (!isPageAvaliable()) {	
 		warn(tr("No page selected!"));
		return;
	}
	
	var dg = createDialog (tr("Add page transformation matrix"), tr("Ok"), tr("Cancel"), tr("Change page transformation matrix"));
	
	/* == First tab */
	dg.newTab(tr("Page transformation matrix description"));
	var gb = createGroupBoxAndDisplay (tr("Page transformation description"),dg);
	var te = new TextEdit;
	te.text = "Translations are specified as [1 0 0 1 tx ty], where tx and ty are the distances "+
			  "to translate the origin of the coordinate system in the horizontal and vertical "+
			  "dimensions, respectively.\n\n"+
			  "Scaling is obtained by [sx 0 0 sy 0 0]. This scales the coordinates so that 1 "+
			  "unit in the horizontal and vertical dimensions of the new coordinate system is "+
			  "the same size as sx and sy units, respectively, in the previous coordinate system.\n\n"+
			  "Rotations are produced by [cos(a)  sin(a)  -sin(a)  cos(a) 0  0], which has the effect "+
			  " of rotating the coordinate system axes by an angle ?? counterclockwise.\n\n"+
			  "Skew is specified by [1 tan(a) tan(b) 1 0 0], which skews the x axis by an angle "+
			  "and the y axis by an angle.";
	gb.add(te);
	dg.newColumn();
	gb = createGroupBoxAndDisplay (tr("Select which transformations to use"),dg);
	rbtran = createCheckBoxAndDisplay (tr("Translate (shift) page"),gb);
	rbscal = createCheckBoxAndDisplay (tr("Scale page"),gb);
	rbskew = createCheckBoxAndDisplay (tr("Skew page"),gb);
	     

	/* == Second tab */
	dg.newTab("Page translation");
	gb = createGroupBoxAndDisplay (tr("Page translation"),dg);
	te = new TextEdit;
	te.text = "Translations are specified as [1 0 0 1 tx ty], where tx and ty are the distances "+
			  "to translate the origin of the coordinate system in the horizontal and vertical "+
			  "dimensions, respectively.\n\n";
	gb.add(te);
	dg.newColumn();
	gb = createGroupBoxAndDisplay (tr("Set values           [* * * * tx ty]"),dg);
	e = createLineEditAndDisplay (tr("tx"), "0", gb);
	f = createLineEditAndDisplay (tr("ty"), "0",  gb);


	/* == Third tab */
	dg.newTab("Page scaling");
	gb = createGroupBoxAndDisplay (tr("Page scaling"),dg);
	te = new TextEdit;
	te.text = "Scaling is obtained by [sx 0 0 sy 0 0]. This scales the coordinates so that 1 "+
			  "unit in the horizontal and vertical dimensions of the new coordinate system is "+
			  "the same size as sx and sy units, respectively, in the previous coordinate system.";
	gb.add(te);
	dg.newColumn();
	gb = createGroupBoxAndDisplay (tr("Set values           [sx * * sy * *]"),dg);
	a = createLineEditAndDisplay (tr("sx"), "0", gb);
	d = createLineEditAndDisplay (tr("sy"), "0", gb);

	/* == Fourth tab */
	dg.newTab("Page skewing");
	gb = createGroupBoxAndDisplay (tr("Page skewing"),dg);
	te = new TextEdit;
	te.text = "Skew is specified by [1 tan(a) tan(b) 1 0 0], which skews the x axis by an angle "+
			  "and the y axis by an angle.";
	gb.add(te);
	dg.newColumn();
	gb = createGroupBoxAndDisplay (tr("Set values           [* tan(a) tan(b) * * *]"),dg);
	b = createLineEditAndDisplay (tr("tan(a)"), "0", gb);
	c = createLineEditAndDisplay (tr("tan(b)"), "0", gb);

	// Set width
	dg.width = 700;
	if (!dg.exec()) return;

	// Default matrix
	tm = [1,0,0,1,0,0];
	if (rbtran.checked) {
		tm[4] = parseFloat(e.text);
		tm[5] = parseFloat(f.text);
	}
	if (rbscal.checked) {
		tm[0] = parseFloat(a.text);
		tm[3] = parseFloat(d.text);
	}
	if (rbskew.checked) {
		tm[1] = parseFloat(b.text);
		tm[2] = parseFloat(c.text);
	}

	if (!rbtran.checked && !rbscal.checked && !rbskew.checked)
		return;
	for(i = 0; i < tm.length; ++i) {
		if (undefined == tm[i]) {
			warn (tr("Invalid number supplied."));
			return;
		}
	}

	page().setTransformMatrix(tm);
	
	print (tr("Page transformation matrix changed to ")+tm);
	go();
}

/**
 * Set one operator position.
 */
function moveOperPos() {

	if (!isPageAvaliable() || !(isTreeItemSelected())) {
		warn(tr("No page or operator selected!"));
		return;
	}
	
	op=treeitem.item();
	if (!isTextOp(op)) {
 		warn(tr("Not valid")+" "+tr("text operator")+". "+tr("Only text operators allowed!"));
		return;
	}

	var posop = getPosInfoOfOperator (op);
	var x = parseFloat (posop.params().property(0).value());
	var y = parseFloat (posop.params().property(1).value());

	if (undefined == posop) {
		warn(tr("Could not find text positioning."));
	}

	var dialog = createDialog (tr("Change relative text operator position"), tr("Change"), tr("Cancel"), tr("Change realtive text position"));
	 
	var gb = createGroupBoxAndDisplay ("Relative operator position (this is not absolute position)", dialog);
	x = createLineEditAndDisplay ("x position:", x, gb);
	y = createLineEditAndDisplay ("y position", y, gb);
	 
	dialog.width = 100;
	if (!dialog.exec()) return;

	if (!isNumber(x.text) || !isNumber(y.text)) {
		warn(tr("Invalid x or y")+". "+tr("Only real numbers allowed")+".");
		return;
	}
	
	// op, change, change
	operatorSetPosition(op, parseFloat(x.text), parseFloat(y.text));

	print (tr("Operator position changed."));
	// Reload page
	go ();
}

/**
 * Change values of a positioning operator.
 */
function moveTextPos() {

	if (!isPageAvaliable() || !(isTreeItemSelected())) {
		warn(tr("No page or operator selected!"));
		return;
	}
	
	op=treeitem.item();
	if (!isTextOp(op)) {
 		warn(tr("Not valid")+" "+tr("text operator")+". "+tr("Only text operators allowed!"));
		return;
	}

	var posop = getPosInfoOfOperator (op);

	if (undefined == posop) {
		warn(tr("Could not find text positioning."));
	}

	var dialog = createDialog (tr("Change relative text operator position"), tr("Change"), tr("Cancel"), tr("Change realtive text position"));
	 
	var gb = createGroupBoxAndDisplay ("Relative operator position", dialog);
	x = createLineEditAndDisplay ("x position:", "0", gb);
	y = createLineEditAndDisplay ("y position", "0", gb);
	 
	if (!dialog.exec()) return;
	 
	// op, change, change
	operatorMovePosition(posop, parseFloat(x.text), parseFloat(y.text));

	print (tr("Operator position changed."));
	// Reload page
	go ();
}


/* ==== Code to run on start ==== */

//Include testing/debugging items in menus?
var tests=settings.readBool("tests");

//Print welcome message
print("PDF Editor "+version());
