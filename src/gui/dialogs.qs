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
 var exl = createNumberEditAndDisplay(tr("Left upper corner")+", "+tr("x position")+"( "+PageSpace.getDefaultUnits()+" ): ",
 									PageSpace.convertUnits(xleft,"pt"), gb);
 var eyl = createNumberEditAndDisplay(tr("Left upper corner")+", "+tr("y position")+"( "+PageSpace.getDefaultUnits()+" ): ",
									PageSpace.convertUnits(yleft,"pt"), gb);
 var exr = createNumberEditAndDisplay(tr("Right bottom corner")+", "+tr("x position")+"( "+PageSpace.getDefaultUnits()+" ): ",
									PageSpace.convertUnits(xright,"pt"), gb);
 var eyr = createNumberEditAndDisplay(tr("Right bottom corner")+", "+tr("y position")+"( "+PageSpace.getDefaultUnits()+" ): ",
									PageSpace.convertUnits(yright,"pt"), gb);
 var pagePos = createNumberEditAndDisplay(tr("Apply from page"), document.getPagePosition(page()), dialog);
 var pageCount = createNumberEditAndDisplay(tr("How many pages"), 1, dialog);

 if (!dialog.exec()) return;

 // Save media box
 for (;pageCount.value > 0 && pagePos.value <= document.getPageCount(); --pageCount.value)
 {
   var page = document.getPage(pagePos.value++);
   page.setMediabox ( PageSpace.convertUnits( exl.value, undefined, "pt" ),
						PageSpace.convertUnits( eyl.value, undefined, "pt" ),
						PageSpace.convertUnits( exr.value, undefined, "pt" ),
						PageSpace.convertUnits( eyr.value, undefined, "pt" ));
 }
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
	op=firstSelected();
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
                        print(op.getName()+" "+tr("is not")+" "+tr("changeable"));
                        op=nextSelected();
                        continue;
                }

                stream=op.stream();
                operatorSetColor(op,col.red, col.green, col.blue, isGraphicalOp(op));
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
	
	op=firstSelected();
        while(op)
        {
                if (!isChangeableOp(op)) {
                        // displays just for the first time and then just silently
                        // ignores
                        if(firstTime)
                        {
                                warn(tr("Selected operator")+" "+tr("is not")+" "+tr("suitable for line width change.")+" "+tr("Please see the pdf specification for details."));
                                firstTime=false;
                        }
                        print(op.getName()+" "+tr("is not")+" "+tr("changeable"));
                        op=nextSelected();
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
                warn(tr("Selected operator")+" "+tr("is not")+" "+tr("suitable for dash style change.")+" "+tr("Please see the pdf specification for details."));
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
function editFontProps(thepage) {

	if ( !isPageAvaliable() || !isTreeItemSelected()) {
		warn(tr("No page or operator selected!"));
		return;
	}
	if (undefined == thepage)
		thepage = page();

	firstTime=true;

	// gets current font from font tool, if this font is not
	// page part, adds it to the page
	currentFont=getEditText("fontface");
	print(currentFont);
	currentFontId=thepage.getFontId(currentFont);
	if(!currentFontId)
	{
		// this font is unknown for page, we have to add it
		thepage.addSystemType1Font(currentFont);
		currentFontId=thepage.getFontId(currentFont);
		print(tr("%1 added to page").arg(currentFont));
	}

	// gets current font size from font tool
	currentFontSize=getNumber("fontsize");

	// Get selected item
	op=firstSelected();
	while(op)
	{
		if (!isTextOp(op))
		{
			if(firstTime)
			{
				warn(tr("Not valid")+" "+tr("text operator")+". "+tr("Only text operators allowed!"));
				firstTime=false;
			}
			print(tr("Operator %1 is not changeable").arg(op.getName()));
			op=nextSelected();
			continue;
		}

		// Set the font
		operatorSetFont( thepage, op,currentFontId, currentFontSize );

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
	var pagePos = createNumberEditAndDisplay(tr("Apply from page"), document.getPagePosition(page()), dialog);
	var pageCount = createNumberEditAndDisplay(tr("How many pages"), 1, dialog);


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
		tm[4] = ef[0].value;
		tm[5] = ef[1].value;
	}
	if (rbscal.checked) {
		tm[0] = ad[0].value;
		tm[3] = ad[1].value;
	}
	if (rbskew.checked) {
		tm[1] = bc[0].value;
		tm[2] = bc[1].value;
	}

	if (!rbtran.checked && !rbscal.checked && !rbskew.checked)
		return;
	for(i = 0; i < tm.length; ++i) {
		if (undefined == tm[i]) {
			warn (tr("Invalid number supplied."));
			return;
		}
	}

	for (;pageCount.value > 0 && pagePos.value <= document.getPageCount(); --pageCount.value)
	{
		var page = document.getPage(pagePos.value++);
		page.setTransformMatrix(tm);
	}
	
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
                        print(op.getName()+" "+tr("is not")+" "+tr("changeable"));
                        op=nextSelected();
                        continue;
                }

                var posop = getPosInfoOfOperator (op);
                if (undefined == posop) {
                        print(tr("Could not find text positioning."));
                        op=nextSelected();
                        continue;
                }
                var x = parseFloat (posop.params().property(0).value());
                var y = parseFloat (posop.params().property(1).value());

                // op, change, change
                operatorSetPosition(op, xy[0].value, xy[1].value);

                op=nextSelected();
        }

	print (tr("Operator position changed."));
	// Reload page
	go ();
}

/**
 * Change values of a positioning operator.
 */
function moveTextPos( delta_x, delta_y ) {

	if (!isPageAvaliable() || !(isTreeItemSelected())) {
		warn(tr("No page or operator selected!"));
		return [0,0];
	}

	if ((undefined == delta_x) || (undefined == delta_y)) {
		var dialog = createDialog (tr("Change relative text operator position"), tr("Change"), tr("Cancel"), tr("Change relative text position"));

		var xy = xydialogs (dialog,tr("Relative operator position"),PageSpace.getDefaultUnits());

		var exec_dialog = true;
		while (exec_dialog) {
			if (!dialog.exec())
				return [0,0];

			delta_x = xy[0].value;
			delta_y = xy[1].value;

			if (isNumber( delta_x ) && isNumber( delta_y )) {
				exec_dialog = false;
				delta_x = PageSpace.convertUnits( delta_x, undefined, "pt" );
				delta_y = PageSpace.convertUnits( delta_y, undefined, "pt" );
			} else
				MessageBox.critical( tr("X and Y position must be a number"), MessageBox.Ok );
		}
	}

	var thepage = page();
	var firstTime = true;
	op=firstSelected();
	while(op)
	{
		if (!isTextOp(op)) {
			if(firstTime)
			{
				warn(tr("Not valid")+" "+tr("text operator")+". "+tr("Only text operators allowed!"));
				firstTime = false;
			}
			print(op.getName()+" "+tr("is not")+" "+tr("changeable"));
			op=nextSelected();
			continue;
		}

		moveTextOp( thepage, op, delta_x, delta_y );

		op=nextSelected();
	}

	print (tr("Operator position changed."));
	// Reload page
	go ();
}


/**
 * Draw line.
 */
function drawLine(_lx,_ly,_rx,_ry,wantedit,_global_x,_global_y) {

	if (!isPageAvaliable()) {
		warn(tr("No page selected!"));
		return;
	}

	// State paramters
	var col;
	var width;
	
	if (undefined != _lx && undefined != _ry) {
		lx = convertPixmapPosToPdfPos_x(_lx,_ly);
		ly = convertPixmapPosToPdfPos_y(_lx,_ly);
		rx = convertPixmapPosToPdfPos_x(_rx,_ry);
		ry = convertPixmapPosToPdfPos_y(_rx,_ry);
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
 * Draw an arrow.
  */
function drawArrow(_lx,_ly,_rx,_ry,wantedit,_global_x,_global_y) {
	// Get the end points
    var _a1 = _rx+(_lx-_ry+_ly-_rx)/10;
    var _a2 = _ry+(_ly+_rx-_lx-_ry)/10;
    var _b1 = _rx+(_lx+_ry-_ly-_rx)/10;
    var _b2 = _ry+(_ly-_rx+_lx-_ry)/10;

	if (undefined != _lx && undefined != _ry) {
		
		// Convert main line points
		lx = convertPixmapPosToPdfPos_x(_lx,_ly);
		ly = convertPixmapPosToPdfPos_y(_lx,_ly);
		rx = convertPixmapPosToPdfPos_x(_rx,_ry);
		ry = convertPixmapPosToPdfPos_y(_rx,_ry);
		
		// Convert arrow end points
		a1 = convertPixmapPosToPdfPos_x(_a1,_a2);
		a2 = convertPixmapPosToPdfPos_y(_a1,_a2);
		b1 = convertPixmapPosToPdfPos_x(_b1,_b2);
		b2 = convertPixmapPosToPdfPos_y(_b1,_b2);
		
	}else
        return;


    operatorDrawLine ([ [lx,ly,rx,ry], [rx,ry,a1,a2], [rx,ry,b1,b2]], getNumber("linewidth"), getColor("fg"));
	// Reload page
	go()
}

/**
 * Draw rect.
 */
function drawRect(_lx,_ly,_rx,_ry,wantedit,_global_x,_global_y) {

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
		lx = convertPixmapPosToPdfPos_x(_lx,_ly);
		ly = convertPixmapPosToPdfPos_y(_lx,_ly);
		rx = convertPixmapPosToPdfPos_x(_rx,_ry);
		ry = convertPixmapPosToPdfPos_y(_rx,_ry);
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
function addText (_x1,_y1,_x2,_y2, _glob_left,_glob_top) {

	if (!isPageAvaliable()) {
		warn(tr("No page selected!"));
		return;
	}

	if (undefined == _x1 || undefined == _y1 || undefined == _x2 || undefined == _y2 ||
		undefined == _glob_left || undefined == _glob_top) {
		return;
	}
	
	//
	// Convert x,y to real x,y
	//
	global_addText_x = Math.min (convertPixmapPosToPdfPos_x(_x1,_y1),convertPixmapPosToPdfPos_x(_x2,_y2));
	global_addText_y = Math.min (convertPixmapPosToPdfPos_y(_x1,_y1),convertPixmapPosToPdfPos_y(_x2,_y2));

	if (_x1 < _x2)
		_glob_left = _glob_left - _x2 + _x1;

	if (_y1 > _y2)
		_glob_top = _glob_top + _y1 - _y2;

	var lineEdit = PageSpace.getTextLine( _glob_left, _glob_top, getNumber( "fontsize" ), getEditText( "fontface" ) );
	lineEdit.resize( Math.max( 50, Math.abs (_x2 - _x1)), lineEdit.height );

	connect( lineEdit, "returnPressed(const QString&)", _AddTextSlot );
	connect( lineEdit, "lostFocus(const QString&)", _AddTextSlot );
}
function _AddTextSlot ( text ) {
	if ((undefined == text) || (text.isEmpty())) {
		return;
	}
	
	var thepage  = page();
	var fname = getEditText( "fontface" );
	var fid=thepage.getFontId( fname );
	if (fid.isEmpty()) {
		thepage.addSystemType1Font( fname );
		fid = thepage.getFontId( fname );
	}
	var fs=getNumber( "fontsize" );

	var ctm = getDetransformationMatrix( thepage );

	operatorAddTextLine ( text, global_addText_x, global_addText_y, fid, fs, createOperator_transformationMatrix( ctm ), getColor("fg"));

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

function highlightingSelectedText(_lx, _ly, _rx, _ry, _global_x, _global_y) {
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

	var _op = firstSelected();
	var _firstTextOp = undefined;
	var _rectangles = [];
	while (_op) {
		if (isTextOp( _op )) {
			var _pom = getFirstTextOp( _op );

			if (_firstTextOp == undefined)
				_firstTextOp = _pom;

			if ( ! _pom.equals (_firstTextOp) ) {
				operatorDrawRect( _rectangles, getColor("bg"), 0, _firstTextOp );
				_firstTextOp = _pom;
				_rectangles = [];
			}

			var _br = _op.getBBox();
			var _x1 = convertPixmapPosToPdfPos_x( _br[0], _br[1] );
			var _y1 = convertPixmapPosToPdfPos_y( _br[0], _br[1] );
			var _x2 = convertPixmapPosToPdfPos_x( _br[2], _br[3] );
			var _y2 = convertPixmapPosToPdfPos_y( _br[2], _br[3] );

			_rectangles.push( [_x1, _y1, _x2-_x1, _y2-_y1 ] );
		}

		_op = nextSelected();
	}

	operatorDrawRect( _rectangles, getColor("bg"), 0, _firstTextOp );

	go();
}

/** Strike trougn text which are selected. */
function strikeTroughSelection(_lx, _ly, _rx, _ry, _global_x, _global_y) {
	var thepage = page();

	var _op = firstSelected();
	var _lines = [];
	while (_op) {
		var _br = _op.getBBox();
		var _x1 = convertPixmapPosToPdfPos_x( _br[0], _br[1] );
		var _y1 = convertPixmapPosToPdfPos_y( _br[0], _br[1] );
		var _x2 = convertPixmapPosToPdfPos_x( _br[2], _br[3] );
		var _y2 = convertPixmapPosToPdfPos_y( _br[2], _br[3] );
		var halfw = (_x2 - _x1) /2;
		var halfh = (_y2 - _y1) /2;
		var _sx = (_x2 + _x1) /2;
		var _sy = (_y2 + _y1) /2;

		var ctm = [1,0,0,1,0,0];
		if (isTextOp( _op ))
			ctm = getTextTransformationMatrix( thepage, _op );
		else
			ctm = getTransformationMatrix( thepage, _op );
		var h_null	= transformationMatrixMul( 0,0, ctm );
		var h_hor	= transformationMatrixMul( 1,0, ctm );
		h_hor[0]	-= h_null[0];
		h_hor[1]	-= h_null[1];

		var dx = 0;
		var dy = 0;
		if ((h_hor[1] != 0) && (Math.abs(h_hor[0] / h_hor[1]) < 1)) {
			dx	= h_hor[0] / h_hor[1] * halfh;
			dy	= halfh;
		} else {
			dx	= halfw;
			dy	= h_hor[1] / h_hor[0] * halfw;
		}

		// add line to draw
		_lines.push( [_sx - dx, _sy - dy, _sx + dx, _sy + dy ] );

		_op = nextSelected();
	}

	operatorDrawLine( _lines, getNumber("linewidth"), getColor("fg") );

	go();
}

/** Fill line edit named "text" on toolbox with text from all selected text operators. */
function fillTextFromSelectedOperators() {
	if ((!isItemInTreeSelected()) ||
		(firstSelected().type()!="PdfOperator") ||
		(! isTextOp( firstSelected() )))
		return false;

	var all_is_text = true;
	var text = "";
	var op = firstSelected();
	while (op) {
		if (! isTextOp( op ) ) {
			all_is_text = false;
			break;
		}
		text = text + getTextFromTextOperator( op );
		op = nextSelected();
	}
	if (! all_is_text)
		return false;

	setEditText( "text", text );
	global_EditText_last_for_text = text;
	return true;
}
/** Remove all selected operators (all must be text) and put operator "Tj" with text from "text" */
function changeSelectedText () {
	if (getEditText("text") == global_EditText_last_for_text)
		return false;

	if ((!isTreeItemSelected()) ||
		(firstSelected().type()!="PdfOperator") ||
		(! isTextOp( firstSelected() )))
		return false;

	var all_is_text = true;
	var op = firstSelected();
	while (op) {
		if (! isTextOp( op ) )
			all_is_text = false;
		op = nextSelected();
	}
	if (! all_is_text)
		return false;

	op = firstSelected();
	var op_to_delete = nextSelected();
	while (op_to_delete) {
		op_to_delete.remove();
		op_to_delete = nextSelected();
	};

	global_EditText_last_for_text = getEditText("text");

	if (op.getName() == "Tj") {
		op.params().property(0).set( getEditText("text") );
		return true;
	};
	var params = createIPropertyArray();
	params.append( createString(getEditText("text")) );
	var new_op = createOperator("Tj",params);
	op.stream().replace( op, new_op );

	return true;
};

function moveSelectedObject( _dx, _dy ) {
	var op = firstSelected();

	if ((_dx == undefined) || (_dy == undefined))
		return ;

	var dx = convertPixmapPosToPdfPos_x( _dx, _dy ) - convertPixmapPosToPdfPos_x( 0, 0 );
	var dy = convertPixmapPosToPdfPos_y( _dx, _dy ) - convertPixmapPosToPdfPos_y( 0, 0 );

	while (op) {
		switch (op.type()) {
			case "PdfOperator":
					if (isTextOp( op ))
						moveTextOp( page(), op, dx, dy );
					else if (isGraphicalOp( op )) {
						moveGraphicalOp( page(), op, dx, dy );
					} else
						moveOp( page(), op, dx, dy );

					break;
			case "Dict":
					var h_type = op.child("Type");
					if (h_type == undefined) {
						warn( tr("Dictionary has no type!") );
						return ;
					}

					switch (h_type.value()) {
						case "Annot" :
									// annotation todo
									var rect = op.child("Rect");
									if (rect == undefined) {
										warn( tr("Maybe bad stream!") +
												tr("%1 is requirement for %2, but not found for it !")
													.arg("Rect")
													.arg(h_type.value()) );
										return ;
									}

									if (rect.count() != 4) {
										warn( tr("Maybe bad stream!") +
												tr("For child %1 of %2 is requirement %3 subchild, but has %4")
													.arg("Rect")
													.arg(h_type.value())
													.arg(4)
													.arg(rect.count()) );
										return ;
									}

									// set rect
									rect.child(0).set( rect.child(0).value() + dx );
									rect.child(1).set( rect.child(1).value() + dy );
									rect.child(2).set( rect.child(2).value() + dx );
									rect.child(3).set( rect.child(3).value() + dy );

									break;
					}
					break;
		}

		op = nextSelected();
	}

	go();
}
