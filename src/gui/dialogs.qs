// helper functions which handles some common behavior
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
function setOpColor() {

        firstTime=true;
	
	if (!isPageAvaliable() || !(isTreeItemSelected())) {
		warn(tr("No page or operator selected!"));
		return;
	}
	
        // uses currently set foregroud colour
        col=getColor("fg");
        if (!col) 
                return;//Dialog aborted
                
        // set color operator to all selected which are changeable
	op=firstSelected("select");
        count=0;
        while(op)
        {
                if (!isChangeableOp(op)) {
                        // displays just for the first time and then just silently 
                        // ignores
                        if(firstTime)
                        {
                                warn(tr("Selected operator")+" "+tr("is not")+" "+tr("suitable for colour setting.")+" "+tr("Please see the pdf specification for details."));
                                firstTime=false;
                        }
                        continue;
                }

                stream=op.stream();
                operatorSetColor(op,col.red, col.green, col.blue);
                op=nextSelected();
                count++;
        }
	print (tr("Colour changed")+" "+tr("for")+" "+count+" "+tr("operators"));

	//reload the page
	go();
}


/** Change dash style of an operator. */
function setLineWidth() {
	
        firstTime = true;

	if (!isPageAvaliable() || !(isTreeItemSelected())) {
		warn(tr("No page or operator selected!"));
		return;
	}

        width = getNumber("linewidth");
	
	op=firstSelected("select");
        while(op)
        {
                if (!isChangeableOp(op)) {
                        // displays just for the first time and then just silently 
                        // ignores
                        if(firstTime)
                        {
                                warn(tr("Selected operator")+" "+tr("is not")+" "+tr("suitable for colour setting.")+" "+tr("Please see the pdf specification for details."));
                                firstTime=false;
                        }
                        continue;
                }
                
                operatorSetLineWidth(op,width,true);

                op=nextSelected();
        }
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
	
        // TODO multiselect
	op=firstSelected();
        if (!isChangeableOp(op)) {
                // displays just for the first time and then just silently 
                // ignores
                warn(tr("Selected operator")+" "+tr("is not")+" "+tr("suitable for colour setting.")+" "+tr("Please see the pdf specification for details."));
                return;
        }
	
        // TODO remove dialog and use currently set values
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
function editFontProps(page) {

	if (!isPageAvaliable() || !(isTreeItemSelected())) {
		warn(tr("No page or operator selected!"));
		return;
	}

        firstTime=true;

        // gets current font from font tool, if this font is not
        // page part, adds it to the page
        currentFont=getEditText("fontface");
        print(currentFont);
        currentFontId=page.getFontId(currentFont);
        if(!currentFontId)
        {
                // this font is unknown for page, we have to add it
                page.addSystemType1Font(currentFont);
                currentFontId=page.getFontId(currentFont);
                print(currentFont+" "+tr("added to")+" "+tr("page"))
        }

        // gets current font size from font tool
        currentFontSize=getNumber("fontsize");

	// Get selected item
 	op=firstSelected("select");
        while(op)
        {
                if (!isTextOp(op)) 
                {
                        if(firstTime)
                        {
                                warn(tr("Not valid")+" "+tr("text operator")+". "+tr("Only text operators allowed!"));
                                firstTime=false;
                                continue;
                        }
                }

                // Set the font
                operatorSetFont(op,currentFontId, currentFontSize);

                op=nextSelected();
        }
                
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
	
	te.tooltip = tr("Text on a page.");
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
	dg.newTab(tr("Page translation"));
	gb = createGroupBoxAndDisplay (tr("Page translation"),dg);
	te = new TextEdit;
	te.text = translation_txt;
	gb.add(te);
	dg.newColumn();
	ef = twonumdialogs (dg,tr("Set values")+"           [* * * * tx ty]","tx","ty");

	/* == Third tab */
	dg.newTab(tr("Page scaling"));
	gb = createGroupBoxAndDisplay (tr("Page scaling"),dg);
	te = new TextEdit;
	te.text = scaling_txt;
	gb.add(te);
	dg.newColumn();
	ad = twonumdialogs (dg,tr("Set values")+"           [sx * * sy * *]","sx","sy");

	/* == Fourth tab */
	dg.newTab(tr("Page skewing"));
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

        var dialog = createDialog (tr("Change relative text operator position"), tr("Change"), tr("Cancel"), tr("Change relative text position"));
         
        var xy = xydialogs (dialog,tr("Relative operator position (this is not absolute position)"));
         
        dialog.width = 100;
        if (!dialog.exec()) return;
	
        firstTime = true;
	op=firstSelected();
        while(op)
        {
                if (!isTextOp(op)) 
                {
                        if(firstTime)
                        {
                                warn(tr("Not valid")+" "+tr("text operator")+". "+tr("Only text operators allowed!"));
                                firstTime = false;
                        }
                        continue;
                }

                var posop = getPosInfoOfOperator (op);
                if (undefined == posop) {
                        warn(tr("Could not find text positioning."));
                        continue;
                }
                var x = parseFloat (posop.params().property(0).value());
                var y = parseFloat (posop.params().property(1).value());

                if (!isNumber(xy[0].text) || !isNumber(xy[1].text)) {
                        warn(tr("Invalid x or y")+". "+tr("Only real numbers allowed")+".");
                        continue;
                }
                
                // op, change, change
                operatorSetPosition(op, parseFloat(xy[0].text), parseFloat(xy[1].text));

                op=nextSelected();
        }

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

        var dialog = createDialog (tr("Change relative text operator position"), tr("Change"), tr("Cancel"), tr("Change relative text position"));
         
        var xy = xydialogs (dialog,tr("Relative operator position"));
         
        if (!dialog.exec()) return;

        firstTime=true;
	op=firstSelected();
        while(op)
        {
                if (!isTextOp(op)) {
                        if(firstTime)
                        {
                                warn(tr("Not valid")+" "+tr("text operator")+". "+tr("Only text operators allowed!"));
                                firstTime = false;
                        }
                        continue;
                }

                var posop = getPosInfoOfOperator (op);

                if (undefined == posop) {
                        warn(tr("Could not find text positioning."));
                        continue;
                }
                 
                // op, change, change
                operatorMovePosition(posop, parseFloat(xy[0].text), parseFloat(xy[1].text));

                op=nextSelected();
        }

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
	}else 
                return;

	print (lx+" "+ly+" "+rx+" "+ry);

        // uses current foreground colour
        col = getColor("fg");
        if (!col) return;

        // uses current line with value
        width = getNumber("linewidth");

	operatorDrawLine( [[lx,ly,rx,ry]], width, col );

	// Reload page
	print (tr("Line was drawn."));
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

        // Uses current colour
        col = getColor("bg");
        if (!col) return;
	
        // uses current line with value
        width = getNumber("linewidth");

	if (undefined != _lx && undefined != _ry) {
		lx = PageSpace.convertPixmapPosToPdfPos_x(_lx,_ly);
		ly = PageSpace.convertPixmapPosToPdfPos_y(_lx,_ly);
		rx = PageSpace.convertPixmapPosToPdfPos_x(_rx,_ry);
		ry = PageSpace.convertPixmapPosToPdfPos_y(_rx,_ry);
		w = rx - lx;
		h = ry - ly;
	}else 
                return;
	
	print (lx+", "+ly+", "+w+", "+h);
	operatorDrawRect([[lx,ly,w,h]],col,width);

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

/** Search text */
function findText ( text ) {

	if (!isPageAvaliable()) {
		warn(tr("No page selected!"));
		return;
	}

	// set text mode
	onSelectMode_Text();

	var numOfFounded = 0;

	if ( (undefined == text) || (text == "") ) {

		var searchDialog = createDialog( tr("Search text"), tr("Find"), tr("Cancel") );

		var searchText = createLineEditAndDisplay( tr("Text :"), "", searchDialog );
		searchText.toolTip = tr("Find what");

		var gb = createGroupBoxAndDisplay( "", searchDialog );

		var actualPageOnly = createRadioButtonAndDisplay( tr("On currently viewed page only"), gb );
		actualPageOnly.checked = true;

		if (searchDialog.exec()) {
		    numOfFounded = PageSpace.findText( searchText.text );
		} else
			return 0;
	} else
		    numOfFounded = PageSpace.findText( text );

	if (numOfFounded == 0) {
		MessageBox.information( tr("Nothing was found."), MessageBox.Ok );
	}

	return numOfFounded;
}

function highlightingSelectedText() {
	function getFirstTextOp( operator ) {
		// TODO kontrola
		var i = operator.iterator();
		for ( ; ! i.isBegin() ; i.prev() ) {
			if (i.current().getName() == "BT") {
				return i.current();
			}
		}
		return i.current();
	}

	var _op = this.firstSelected();
	var _firstTextOp = undefined;
	var _rectangles = [];
	while (_op) {
		if (isTextOp( _op )) {
			var _pom = getFirstTextOp( _op );

			if (_firstTextOp == undefined)
				_firstTextOp = _pom;

			if ( ! _pom.equals (_firstTextOp) ) {
				this.operatorDrawRect( _rectangles, getColor("bg"), 0, _firstTextOp );
				_firstTextOp = _pom;
				_rectangles = [];
			}

			var _br = _op.getBBox();
			var _x1 = PageSpace.convertPixmapPosToPdfPos_x( _br[0], _br[1] );
			var _y1 = PageSpace.convertPixmapPosToPdfPos_y( _br[0], _br[1] );
			var _x2 = PageSpace.convertPixmapPosToPdfPos_x( _br[2], _br[3] );
			var _y2 = PageSpace.convertPixmapPosToPdfPos_y( _br[2], _br[3] );

			_rectangles.push( [_x1, _y1, _x2-_x1, _y2-_y1 ] );
		}

		_op = this.nextSelected();
	}

	this.operatorDrawRect( _rectangles, getColor("bg"), 0, _firstTextOp );

	go();
}

function strikeTroughSelection() {
	var _op = this.firstSelected();
	var _lines = [];
	while (_op) {
		var _br = _op.getBBox();
		var _x1 = PageSpace.convertPixmapPosToPdfPos_x( _br[0], _br[1] );
		var _y1 = PageSpace.convertPixmapPosToPdfPos_y( _br[0], _br[1] );
		var _x2 = PageSpace.convertPixmapPosToPdfPos_x( _br[2], _br[3] );
		var _y2 = PageSpace.convertPixmapPosToPdfPos_y( _br[2], _br[3] );
		var halfw = (_x2 - _x1) /2;
		var halfh = (_y2 - _y1) /2;

		_lines.push( [_x1, _y1 + halfh, _x2, _y1 + halfh ] );

		_op = this.nextSelected();
	}

	this.operatorDrawLine( _lines, getNumber("linewidth"), getColor("fg") );

	go();
}
