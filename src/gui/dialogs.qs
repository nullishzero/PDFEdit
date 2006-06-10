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
	ef = twonumdialogs (dg,tr("Set values")+"           [* * * * tx ty]","tx","ty");

	/* == Third tab */
	dg.newTab("Page scaling");
	gb = createGroupBoxAndDisplay (tr("Page scaling"),dg);
	te = new TextEdit;
	te.text = scaling_txt;
	gb.add(te);
	dg.newColumn();
	ad = twonumdialogs (dg,tr("Set values")+"           [sx * * sy * *]","sx","sy");

	/* == Fourth tab */
	dg.newTab("Page skewing");
	gb = createGroupBoxAndDisplay (tr("Page skewing"),dg);
	te = new TextEdit;
	te.text = skew_txt;
	gb.add(te);
	dg.newColumn();
	bc = twonumdialogs (dg,tr("Set values")+"           [* tan(a) tan(b) * * *]","tan(a)","tan(b)");

	// Set width
	dg.width = 700;
	if (!dg.exec()) return;

	// Default matrix
	tm = [1,0,0,1,0,0];
	if (rbtran.checked) {
		tm[4] = parseFloat(ef[0].text);
		tm[5] = parseFloat(ef[1].text);
	}
	if (rbscal.checked) {
		tm[0] = parseFloat(ad[0].text);
		tm[3] = parseFloat(ad[1].text);
	}
	if (rbskew.checked) {
		tm[1] = parseFloat(bc[0].text);
		tm[2] = parseFloat(bc[1].text);
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
	 
	var xy = xydialogs (dialog,tr("Relative operator position (this is not absolute position)"));
	 
	dialog.width = 100;
	if (!dialog.exec()) return;

	if (!isNumber(xy[0].text) || !isNumber(xy[1].text)) {
		warn(tr("Invalid x or y")+". "+tr("Only real numbers allowed")+".");
		return;
	}
	
	// op, change, change
	operatorSetPosition(op, parseFloat(xy[0].text), parseFloat(xy[1].text));

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
	 
	var xy = xydialogs (dialog,tr("Relative operator position"));
	 
	if (!dialog.exec()) return;
	 
	// op, change, change
	operatorMovePosition(posop, parseFloat(xy[0].text), parseFloat(xy[1].text));

	print (tr("Operator position changed."));
	// Reload page
	go ();
}


/**
 * Draw line.
 */
function drawLine(_lx,_ly,_rx,_ry,wantedit) {

	if (!isPageAvaliable()) {
		warn(tr("No page selected!"));
		return;
	}

	// State paramters
	var col;
	var width;
	
	if (undefined != _lx && undefined != _ry) {
		lx = PageSpace.convertPixmapPosToPdfPos_x(_lx,_ly);
		ly = PageSpace.convertPixmapPosToPdfPos_y(_lx,_ly);
		rx = PageSpace.convertPixmapPosToPdfPos_x(_rx,_ry);
		ry = PageSpace.convertPixmapPosToPdfPos_y(_rx,_ry);
	}else {
		lx = 0;
		ly = 0;
		rx = 0;
		ry = 0;
	}
	print (lx+" "+ly+" "+rx+" "+ry);

	if (undefined == wantedit || wantedit) { // Ask user 
			
		var dialog = createDialog (tr("Draw line"), tr("Draw"), tr("Cancel"), tr("Draw line"));
		 
		//
		// x y
		//
		var lxy = xydialogs (dialog,tr("Start position"));
		var rxy = xydialogs (dialog,tr("End position"));
	
		lxy[0].text = lx;
		lxy[1].text = ly;
		rxy[0].text = rx;
		rxy[1].text = ry;
		
		//
		// Line width
		//
		dialog.newColumn();
		
		var gb = createGroupBoxAndDisplay ("Line width (0-default)", dialog);
		var sb = createSpinboxAndDisplay ("Line width", 0, 100,gb);
		sb.value = 0;
		var color = createCheckBoxAndDisplay (tr("Change background color"),gb);
	 
		if (!dialog.exec()) return;

		if (color.checked) {
			col = pickColor ();
			if (!col) return;
		}

		if (!isNumber4(lxy[0].text,lxy[1].text,rxy[0].text,rxy[1].text)) {
			warn(tr("Invalid position")+". "+tr("Only real numbers allowed")+".");
			return;
		}
		// op
		lx = parseFloat(lxy[0].text);
		ly = parseFloat(lxy[1].text);
		rx = parseFloat(rxy[0].text);
		ry = parseFloat(rxy[1].text);

		if (sb.value != 0)
			width = parseFloat (sb.value);
	}
	
	print (col);
	operatorDrawLine(lx,ly,rx,ry,width,col);

	print (tr("Line was drawn."));
	// Reload page
	go ();
}

/**
 * Draw rect.
 */
function drawRect(_lx,_ly,_rx,_ry,wantedit) {

	if (!isPageAvaliable()) {
		warn(tr("No page selected!"));
		return;
	}
	
	// State paramters
	var col;
	
	if (undefined != _lx && undefined != _ry) {
		lx = PageSpace.convertPixmapPosToPdfPos_x(_lx,_ly);
		ly = PageSpace.convertPixmapPosToPdfPos_y(_lx,_ly);
		rx = PageSpace.convertPixmapPosToPdfPos_x(_rx,_ry);
		ry = PageSpace.convertPixmapPosToPdfPos_y(_rx,_ry);
		w = rx - lx;
		h = ry - ly;
	}else {
		lx = 0;
		ly = 0;
		w = 0;
		h = 0;
	}

	if (undefined == wantedit || wantedit) { // Ask user 
		var dialog = createDialog (tr("Draw rectangle"), tr("Draw"), tr("Cancel"), tr("Draw rectangle"));
		var lxy = xydialogs (dialog,tr("Upper left corner"));
		lxy[0].text = lx;
		lxy[1].text = ly;

		dialog.newColumn();
		var wh = twonumdialogs (dialog,tr("Metrics"),tr("Width"),tr("Height"));
		wh[0].text = w;
		wh[1].text = h;
		
		var color = createCheckBoxAndDisplay (tr("Change background color"),gb);
	 
		if (!dialog.exec()) return;
		if (color.checked) {
			col = pickColor ();
			if (!col) return;
		}

		if (!isNumber4(lxy[0].text,lxy[1].text,wh[0].text,wh[1].text)) {
			warn(tr("Invalid position")+". "+tr("Only real numbers allowed")+".");
			return;
		}
		// op
		lx = parseFloat(lxy[0].text);
		ly = parseFloat(lxy[1].text);
		w = parseFloat(wh[0].text);
		h = parseFloat(wh[1].text);

	}
	
	print (lx+", "+ly+", "+w+", "+h);
	operatorDrawRect(lx,ly,w,h,col);

	print (tr("Rect was drawn."));
	// Reload page
	go ();
}

/**
 * Add text
 */
function addText (_x1,_y1,_x2,_y2) {

	if (!isPageAvaliable()) {
		warn(tr("No page selected!"));
		return;
	}

	// set default values
	var setpos = false;
	
	if (undefined == _x1 || undefined == _y1 || undefined == _x2 || undefined == _y2) { 
		_x1 = 0;
		_y1 = 0;
		_x2 = 0;
		_y2 = 0;
		setpos = true;
	}
	
	//
	// Text and position selection if needed
	// 
	var dg = createDialog (tr("Add text line"), tr("Add"), tr("Cancel"), tr("Add text line"));
	 
	var gb = createGroupBoxAndDisplay (tr("Text"), dg);
	etxt = createLineEditAndDisplay (tr("Text to add"), "", gb);

	if (setpos) {
		xy = xydialogs (dg,tr("Text position"));
	}

	dg.newColumn();

	//
	// Font selection
	//
	gb = createGroupBoxAndDisplay (tr("Set font"),dg);
	cf = new ComboBox;
	cf.label = tr("Select from all avaliable fonts");
	// Put all avaliable fonts here and select the operator font 
	var fonts = page().getFontIdsAndNames();
	var fontnames = new Array(fonts.length/2);
	// Fill fontnames with symbolic font names
	for(i = 1,j=0; i < fonts.length; ++i) {
        fontnames[j] = fonts[i];
		// Skip id
		++i;++j;
    }
	cf.itemList = fontnames;
	gb.add (cf);

	//
	// Font size
	//
	gb = createGroupBoxAndDisplay (tr("Font size"),dg);
	var fs = createNumbereditAndDisplay (tr("Size"), 0, 100, gb);
	// Why not 10?
	fs.value = 10;

	//
	// Open dialog
	//
	if (!dg.exec()) return;

	//
	// Convert x,y to real x,y
	//
	if (setpos) {
		if (!isNumber2(xy[0].text,xy[1].text)) {
			warn(tr("Invalid position")+". "+tr("Only real numbers allowed")+".");
			return;
		}
		_x = parseFloat(xy[0].text);
		_y = parseFloat(xy[1].text);
		x = PageSpace.convertPixmapPosToPdfPos_x(_x,_y);
		y = PageSpace.convertPixmapPosToPdfPos_y(_x,_y);

	}else {
		x = Math.min (PageSpace.convertPixmapPosToPdfPos_x(_x1,_y1),PageSpace.convertPixmapPosToPdfPos_x(_x2,_y2));
		y = Math.min (PageSpace.convertPixmapPosToPdfPos_y(_x1,_y1),PageSpace.convertPixmapPosToPdfPos_y(_x2,_y2));
		//print (_x1+" "+_y1);
		//x = PageSpace.convertPixmapPosToPdfPos_x(_x1,_y1);
		//y = PageSpace.convertPixmapPosToPdfPos_y(_x1,_y1);
	}

	//
	// Get font id
	//
	var newfontname = cf.currentItem;// get Idx according to a name
	newfontid = "";
	for(i = 1; i < fonts.length; ++i) {
		// Save symbolic name of operator font
		if (fonts[i] == newfontname) {
			fid = fonts[i-1];
			break;
		}
		// Skip id
		++i;
    }
	
	// Draw the text
	print (x+" "+y);
	operatorAddTextLine (etxt.text,x,y,fid,fs.value);
	
	// Update
	go();
}

/** Delete page */
function delPage (i) {

	if (!isPageAvaliable()) {
		warn(tr("No page selected!"));
		return;
	}

	if (undefined == i) {

		var dg = createDialog (tr("Remove page"), tr("Remove"), tr("Cancel"), tr("Remove page"));
		sb = createSpinboxAndDisplay (tr("Remove page")+" [1.."+document.getPageCount()+"]",1,document.getPageCount(),dg);

		if (!dg.exec()) return;

		if (!isNumber(sb.value)) {
			warn (tr("Invalid page"));
			return;
		}

		i = parseInt (sb.value);
	}

	print (i);
	document.removePage (i);


	print ("Page removed");
	
	// Reload
	go()
	
}




