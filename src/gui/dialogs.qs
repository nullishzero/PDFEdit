/* === Dialogs === */

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
        translation_txt =  "Translations are specified as [1 0 0 1 tx ty], where tx and ty are the distances "+
			  "to translate the origin of the coordinate system in the horizontal and vertical "+
			  "dimensions, respectively."
	scaling_txt =	  "Scaling is obtained by [sx 0 0 sy 0 0]. This scales the coordinates so that 1 "+
			  "unit in the horizontal and vertical dimensions of the new coordinate system is "+
			  "the same size as sx and sy units, respectively, in the previous coordinate system.";
	rotation_txt =	  "Rotations are produced by [cos(a)  sin(a)  -sin(a)  cos(a) 0  0], which has the effect "+
			  " of rotating the coordinate system axes by an angle ?? counterclockwise.";
	skew_txt =	  "Skew is specified by [1 tan(a) tan(b) 1 0 0], which skews the x axis by an angle "+
			  "and the y axis by an angle.";
	te.text =	  translation_txt + "\n\n" +
			  scaling_txt + "\n\n" +
			  rotation_txt + "\n\n" +
			  skew_txt;
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
	te.text = translation_txt;
	gb.add(te);
	dg.newColumn();
	gb = createGroupBoxAndDisplay (tr("Set values           ")+"[* * * * tx ty]",dg);
	e = createLineEditAndDisplay ("tx", "0", gb);
	f = createLineEditAndDisplay ("ty", "0",  gb);


	/* == Third tab */
	dg.newTab("Page scaling");
	gb = createGroupBoxAndDisplay (tr("Page scaling"),dg);
	te = new TextEdit;
	te.text = scaling_txt;
	gb.add(te);
	dg.newColumn();
	gb = createGroupBoxAndDisplay (tr("Set values           ")+"[sx * * sy * *]",dg);
	a = createLineEditAndDisplay ("sx", "0", gb);
	d = createLineEditAndDisplay ("sy", "0", gb);

	/* == Fourth tab */
	dg.newTab("Page skewing");
	gb = createGroupBoxAndDisplay (tr("Page skewing"),dg);
	te = new TextEdit;
	te.text = skew_txt;
	gb.add(te);
	dg.newColumn();
	gb = createGroupBoxAndDisplay (tr("Set values           ")+"[* tan(a) tan(b) * * *]",dg);
	b = createLineEditAndDisplay ("tan(a)", "0", gb);
	c = createLineEditAndDisplay ("tan(b)", "0", gb);

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

	var dialog = createDialog (tr("Change relative text operator position"), tr("Change"), tr("Cancel"), tr("Change relative text position"));
	 
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

	var dialog = createDialog (tr("Change relative text operator position"), tr("Change"), tr("Cancel"), tr("Change relative text position"));
	 
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


/**
 * Draw line.
 */
function drawLine(_lx,_ly,_rx,_ry) {

	if (!isPageAvaliable()) {
		warn(tr("No page selected!"));
		return;
	}
	
	if (undefined == _lx || undefined == _ly || undefined == _rx || undefined == _ry)
	{ // Ask user if not defined
		var dialog = createDialog (tr("Draw line"), tr("Draw"), tr("Cancel"), tr("Draw line"));
		 
		var gb = createGroupBoxAndDisplay ("Start position", dialog);
		elx = createLineEditAndDisplay (tr("x position:"), "0", gb);
		ely = createLineEditAndDisplay (tr("y position:"), "0", gb);

		dialog.newColumn();
		gb = createGroupBoxAndDisplay (tr("End position"), dialog);
		erx = createLineEditAndDisplay (tr("x position:"), "0", gb);
		ery = createLineEditAndDisplay (tr("y position:"), "0", gb);
	 
		if (!dialog.exec()) return;

		if (!isNumber4(elx.text,ely.text,erx.text,ery.text)) {
			warn(tr("Invalid position")+". "+tr("Only real numbers allowed")+".");
			return;
		}
		// op
		_lx = parseFloat(elx.text);
		_ly = parseFloat(ely.text);
		_rx = parseFloat(erx.text);
		_ry = parseFloat(ery.text);
	}
	
	lx = PageSpace.convertPixmapPosToPdfPos_x(_lx,_ly);
	ly = PageSpace.convertPixmapPosToPdfPos_y(_lx,_ly);
	rx = PageSpace.convertPixmapPosToPdfPos_x(_rx,_ry);
	ry = PageSpace.convertPixmapPosToPdfPos_y(_rx,_ry);
	
	operatorDrawLine(lx,ly,rx,ry);

	print (tr("Line was drawn."));
	// Reload page
	go ();
}

/**
 * Draw rect.
 */
function drawRect(_lx,_ly,_rx,_ry) {

	if (!isPageAvaliable()) {
		warn(tr("No page selected!"));
		return;
	}
	
	if (undefined == _lx || undefined == _ly || undefined == _rx || undefined == _ry)
	{ // Ask user if not defined
		var dialog = createDialog (tr("Draw rectangle"), tr("Draw"), tr("Cancel"), tr("Draw rectangle"));
		 
		var gb = createGroupBoxAndDisplay (tr("Upper left corner"), dialog);
		elx = createLineEditAndDisplay (tr("x position"), "0", gb);
		ely = createLineEditAndDisplay (tr("y position"), "0", gb);

		dialog.newColumn();
		gb = createGroupBoxAndDisplay (tr("Metrics"), dialog);
		ew = createLineEditAndDisplay (tr("Width"), "0", gb);
		eh = createLineEditAndDisplay (tr("Height"), "0", gb);
	 
		if (!dialog.exec()) return;

		if (!isNumber4(elx.text,ely.text,ew.text,eh.text)) {
			warn(tr("Invalid position")+". "+tr("Only real numbers allowed")+".");
			return;
		}
		// op
		_lx = parseFloat(elx.text);
		_ly = parseFloat(ely.text);
		
		lx = PageSpace.convertPixmapPosToPdfPos_x(_lx,_ly);
		ly = PageSpace.convertPixmapPosToPdfPos_y(_lx,_ly);
		width = parseFloat(ew.text);
		height = parseFloat(eh.text);

	}else {
		rx = PageSpace.convertPixmapPosToPdfPos_x(_rx,_ry);
		ry = PageSpace.convertPixmapPosToPdfPos_y(_rx,_ry);
		lx = PageSpace.convertPixmapPosToPdfPos_x(_lx,_ly);
		ly = PageSpace.convertPixmapPosToPdfPos_y(_lx,_ly);
		width = rx - lx;
		height = ry - ly;
	}
	
	print (lx+", "+ly+", "+width+", "+height);
	operatorDrawRect(lx,ly,width,height);

	print (tr("Rect was drawn."));
	// Reload page
	go ();
}

/**
 * Add text
 */
function addText (x,y) {

	if (!isPageAvaliable()) {
		warn(tr("No page selected!"));
		return;
	}
	
	if (undefined == x || undefined == y) { 
		x = 0;
		y = 0;
	}
		
		
	// Ask user if not defined
	var dialog = createDialog (tr("Add text line"), tr("Add"), tr("Cancel"), tr("Add text line"));
	 
	var gb = createGroupBoxAndDisplay (tr("Text"), dialog);
	etxt = createLineEditAndDisplay (tr("Text to add"), "", gb);
	ex = createLineEditAndDisplay (tr("x position"), "0", gb);
	ey = createLineEditAndDisplay (tr("y position"), "0", gb);

	if (!dialog.exec()) return;

	if (!isNumber2(ex.text,ey.text)) {
		warn(tr("Invalid position")+". "+tr("Only real numbers allowed")+".");
		return;
	}
	// op
	_x = parseFloat(ex.text);
	_y = parseFloat(ey.text);
	
	x = PageSpace.convertPixmapPosToPdfPos_x(_x,_y);
	y = PageSpace.convertPixmapPosToPdfPos_y(_x,_y);

	operatorAddTextLine (etxt.text,x,y);
	
}


