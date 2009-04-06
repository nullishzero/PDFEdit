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
/* === Validation functions === */

/** Is this operator text operator. */
function isTextOp(operator) {
	var txtit = operator.textIterator();
	if (!txtit.isEnd() && operator.equals(txtit.current()))
		return true;
	else
		return false;
}

/** Is this operator text operator. */
function isChangeableOp(operator) {
	var txtit = operator.changeableIterator();
	if (!txtit.isEnd() && operator.equals(txtit.current()))
		return true;
	else
		return false;
}

/** Is this operator graphical operator. */
function isGraphicalOp(operator) {
	var txtit = operator.graphicalIterator();
	if (!txtit.isEnd() && operator.equals(txtit.current()))
		return true;
	else
		return false;
}

/* === Content stream helper functions === */
/** Change nonstroking color. */
function putnscolor (op,r,g,b) {
	var operands = createIPropertyArray();
	operands.append(createReal(r/255));
	operands.append(createReal(g/255));
	operands.append(createReal(b/255));
	op.pushBack( createOperator("rg",operands),op.getLastOperator());
}
/** Change stroking color */
function putscolor (op,r,g,b) {
	var operands = createIPropertyArray();
	operands.append(createReal(r/255));
	operands.append(createReal(g/255));
	operands.append(createReal(b/255));
	op.pushBack( createOperator("RG", operands),op.getLastOperator());
}
/** Change font */
function putfont (op,fid,fs) {
	var operands = createIPropertyArray ();
	operands.append(createName(fid));
	operands.append(createReal(fs));
	op.pushBack(createOperator("Tf", operands), op.getLastOperator());
}
/** Add new line */
function putline (op,lx,ly,rx,ry) {
	var operands = createIPropertyArray ();
	operands.append (createReal(lx));
	operands.append (createReal(ly));
	op.pushBack (createOperator("m",operands), op.getLastOperator());

	operands.clear();
	operands.append (createReal(rx));
	operands.append (createReal(ry));
	op.pushBack (createOperator("l",operands),op.getLastOperator());
}
/** Add new rect */
function putrect (op,x,y,w,h) {
	var operands = createIPropertyArray ();
	operands.append (createReal(x));
	operands.append (createReal(y));
	operands.append (createReal(w));
	operands.append (createReal(h));
	op.pushBack (createOperator("re",operands),op.getLastOperator());
}
/** Set line width */
function putlinewidth (op,w) {
	var operands = createIPropertyArray ();
	operands.append (createInt(w));
	op.pushBack (createOperator("w",operands), op.getLastOperator());	
}
/** Change rel.position */
function puttextrelpos (op,dx,dy) {
	var operands = createIPropertyArray ();
	operands.append (createReal(dx));
	operands.append (createReal(dy));
	op.pushBack (createOperator("Td",operands), op.getLastOperator());
}
/** End drawing */
function putenddraw (op) {
	var operands = createIPropertyArray ();
	op.pushBack (createOperator("S",operands),op.getLastOperator());
}
/** Fill */
function putfill (op) {
	var operands = createIPropertyArray ();
	op.pushBack (createOperator("B",operands),op.getLastOperator());
}
/** Add text line */
function puttext (op,txt) {
	var operands = createIPropertyArray ();
	operands.append (createString(txt));
	op.pushBack (createOperator("Tj",operands),op.getLastOperator());
}
/** End text */
function putendtext (op) {
	var operands = createIPropertyArray ();
	op.pushBack (createOperator("ET",operands),op.getLastOperator());
}
/** Restore graphical state. */
function putendq (op) {
	var operands = createIPropertyArray ();
	op.pushBack( createOperator("Q",operands), op.getLastOperator());
}

/** Transformation matrix ( "cm" or "Tm" ) */
function createOperator_transformationMatrix( matrix : Array_of_6_doubles, name_of_op : String ) {
	if (matrix.length != 6) {
		warn( tr("Operator cm must have array with 6 numbers as parameter!") );
		return ;
	}

	if (undefined == name_of_op)
		name_of_op = "cm";

	var operands = createIPropertyArray ();
	operands.append (createReal(matrix[0]));
	operands.append (createReal(matrix[1]));
	operands.append (createReal(matrix[2]));
	operands.append (createReal(matrix[3]));
	operands.append (createReal(matrix[4]));
	operands.append (createReal(matrix[5]));

	return createOperator( name_of_op, operands );
}

/**
 Multiply matrix or vector by another matrix ( a * b )
 Obsoleted by re-implementation in C++
 */
function _transformationMatrixMul( a, b ) {
	var c = [0,0,0,0,0,0];
	if (a.length == 6) {
		// 3x3 matrix multiply 3x3 matrix
		c[0] = a[0]*b[0]+a[1]*b[2];
		c[1] = a[0]*b[1]+a[1]*b[3];
		c[2] = a[2]*b[0]+a[3]*b[2];
		c[3] = a[2]*b[1]+a[3]*b[3];
		c[4] = a[4]*b[0]+a[5]*b[2]+b[4];
		c[5] = a[4]*b[1]+a[5]*b[3]+b[5];
	} else if (a.length == 2) {
		// 1x3 vector multiply 3x3 matrix
		c = [0,0];
		c[0] = a[0]*b[0]+a[1]*b[2] + b[4];
		c[1] = a[0]*b[1]+a[1]*b[3] + b[5];
	}

	return c;
}

/**
 Solve equation  oldCTM * requiredCTM = newCTM
 Obsoleted by re-implementation in C++
 */
function _transformationMatrixDiv( oldCTM : Array_of_6_doubles, newCTM : Array_of_6_doubles ) {
	var nowCTM = [0,0,0,0,0,0];
	var menovatel = oldCTM[0]*oldCTM[3]-oldCTM[1]*oldCTM[2];
	if (menovatel == 0) {
		warn(tr("Matrix is in bad state !"));
		return;
	}
	nowCTM[0] = (-oldCTM[2]*newCTM[1]+newCTM[0]*oldCTM[3]) / menovatel
	nowCTM[1] = (oldCTM[0]*newCTM[1]-oldCTM[1]*newCTM[0]) / menovatel
	nowCTM[2] = (-oldCTM[2]*newCTM[3]+newCTM[2]*oldCTM[3]) / menovatel
	nowCTM[3] = (oldCTM[0]*newCTM[3]-oldCTM[1]*newCTM[2]) / menovatel
	nowCTM[4] = -(-oldCTM[2]*oldCTM[5]+oldCTM[2]*newCTM[5]+oldCTM[4]*oldCTM[3]-newCTM[4]*oldCTM[3]) / menovatel
	nowCTM[5] = (oldCTM[1]*oldCTM[4]-oldCTM[0]*oldCTM[5]+oldCTM[0]*newCTM[5]-oldCTM[1]*newCTM[4]) / menovatel

	return nowCTM;
}

/** Get text transformation matrix using for operator or for next operator.
 * (nothing global transformation)
 */
function get_textTransformationMatrix( operator: PdfOperator, before: bool ) {
	if ((operator == undefined) || (operator.type() != "PdfOperator"))
		return ;

	var it = operator.iterator();
	var end_op = operator;
	var textLeading = 0;
	var ttm = [1,0,0,1,0,0];

	if ((undefined != before) && (before == false))
		end_op = it.next().current();

	it.prev();

	// find begin of text (operator "BT")
	while (! it.isBegin()) {
		var name = it.current().getName();
		if (name == "ET") {
			// operator is not in text space (or contentstream is bad)
			return [1,0,0,1,0,0];
		}
		if (name == "BT")
			break;

		it.prev();
	}

	if (it.isBegin()) {
		// operator is not in text space (or contentstream is bad)
		return [1,0,0,1,0,0];
	}

	var it_cur = it.current();
	while ((! it.isEnd()) && (! it_cur.equals( end_op ))) {
		switch (it_cur.getName()) {
			case "TL":	// Text Leading
					if (it_cur.paramCount() != 1) {
						warn(	tr("Maybe bad stream!") +
								tr("Operator '%1' should have 1 parameter, but has %2 !")
										.arg(it_cur.getName())
										.arg(it_cur.paramCount()));
						return ;
					}
					textLeading = it_cur.params().property(0).value();
					break;
			case "TD":	// Move text position and set leading
					if (it_cur.paramCount() == 2)
						textLeading =  - it_cur.params().property(1).value();
					// next code is equal for TD and Td
			case "Td":	// Move text position
					if (it_cur.paramCount() != 2) {
						warn(	tr("Maybe bad stream!") +
								tr("Operator '%1' should have %2 parameters, but has %3 !")
										.arg(it_cur.getName())
										.arg(2)
										.arg(it_cur.paramCount()));
						return ;
					}

					var h_params = it_cur.params();
					ttm = transformationMatrixMul( [1,0,0,1, h_params.property(0).value(), h_params.property(1).value() ], ttm );
					break;
			case "T*":	// Move to next line
					ttm = transformationMatrixMul( [1,0,0,0, 0,textLeading], ttm );
					break;
			case "Tm":	// Set text transformation matrix
					if (it_cur.paramCount() != 6) {
						warn(	tr("Maybe bad stream!") +
								tr("Operator '%1' should have %2 parameters, but has %3 !")
										.arg(it_cur.getName())
										.arg(6)
										.arg(it_cur.paramCount()));
						return ;
					}

					var h_params = it_cur.params();
					ttm = [	h_params.property(0).value(),
							h_params.property(1).value(),
							h_params.property(2).value(),
							h_params.property(3).value(),
							h_params.property(4).value(),
							h_params.property(5).value() ];
					break;
		}	// switch

		it_cur = it.next().current();
	}

	return ttm;
}

/** Get transformation matrix using for operator or for next operator.
 * Calculate only inside its stream (nothing text transformation)
 */
function get_transformationMatrix( operator: PdfOperator, before: bool, prev_ctm : Array_of_6_doubles ) {
	if ((operator == undefined) || (operator.type() != "PdfOperator"))
		return ;

	var it = operator.iterator();
	var num_of_Q_without_q = 0;
	var ctm = [1,0,0,1,0,0];

	if ((undefined == before) || (before == true))
		it.prev();

	while (! it.isBegin()) {
		var it_cur = it.current();
		var it_cur_name = it_cur.getName();
		switch (it_cur.getName()) {
			case "Q":
					++num_of_Q_without_q;
					break;
			case "q":
					if (num_of_Q_without_q > 0)
						--num_of_Q_without_q;
					break;
			case "cm":
					if (num_of_Q_without_q == 0) {
						if (it_cur.paramCount() != 6) {
							warn(	tr("Maybe bad stream!") +
									tr("Operator '%1' should have %2 parameters, but has %3 !")
											.arg(it_cur.getName())
											.arg(6)
											.arg(it_cur.paramCount()));
							return ;
						}
						it_cur = it_cur.params();
						var h_ctm = [	it_cur.property(0).value(),
										it_cur.property(1).value(),
										it_cur.property(2).value(),
										it_cur.property(3).value(),
										it_cur.property(4).value(),
										it_cur.property(5).value() ];
						ctm = transformationMatrixMul( h_ctm, ctm );
					}
					break;
		}	// switch

		it.prev();
	}

	if (prev_ctm != undefined)
		ctm = transformationMatrixMul( prev_ctm, ctm );

	return ctm;
}

/** */
function get_cmToDetransformation( operator: PdfOperator, before: bool, prev_ctm : Array_of_6_doubles ) {
	if ((operator == undefined) || (operator.type() != "PdfOperator"))
		return ;

	var ctm = [1,0,0,1,0,0];

	if (prev_ctm == undefined)
		ctm = get_transformationMatrix( operator, before );
	else
		ctm = get_transformationMatrix( operator, before, prev_ctm );

	if (ctm == undefined)
		return ;
	
	return transformationMatrixDiv( ctm, [1,0,0,1,0,0] );
}
/** == debug utilities == */
function _dbgprintOpersB() {
	
	op=firstSelected();
	var it = op.iterator();
	while (it.valid()) {
		print (it.current().getName());
		it.prev();
	}
}

/** == debug utilities == */
function _dbgprintOpers() {
	
	op=firstSelected();
	var it = op.iterator();
	while (!it.isEnd()) {
		print (it.current().getName());
		it.next();
	}
}

/** put operator \a op_to_put befor operator \a next_operator to contentstream */
function operatorPutBeforOp( next_operator, op_to_put ) {
	var composite = createCompositeOperator( "", "" );
	composite.pushBack( op_to_put, composite.getLastOperator() );
	composite.pushBack( next_operator.clone(), composite.getLastOperator() );

	next_operator.stream().replace( next_operator, composite );
}
/** put operator \a op_to_put behind operator \a prev_operator to contentstream */
function operatorPutBehindOp( prev_operator, op_to_put ) {
	var composite = createCompositeOperator( "", "" );
	composite.pushBack( prev_operator.clone(), composite.getLastOperator() );
	composite.pushBack( op_to_put, composite.getLastOperator() );

	prev_operator.stream().replace( prev_operator, composite );
}

/** set color of operator
 * @param operator change color of this operator
 * @param r red component of color for set
 * @param g green component of color for set
 * @param b blue component of color for set
 * @param globchange If true change is global.
 *
 * If red,green or blue component is not set, view color dialog for set correct color.
 */
function operatorSetColor(operator,r,g,b, globchange) {
	// --------  check parameters  ---------

	if (operator.type() != "PdfOperator") {
		warn (tr("Operator must be")+" QSPdfOperator !");
		return ;
	}
	if ((r == undefined) || (g == undefined) || (b == undefined)) {
		var col = getColor("fg");
		if (!col) {
			warn( tr("Must define color")+" !" );
			return operator;
		}
		r=col.red;
		g=col.green;
		b=col.blue;
	}
	if (((typeof r ) != "number") ||
		((typeof g ) != "number") ||
		((typeof b ) != "number")) {
		warn( tr("r,g,b must be number")+" !" );
		return operator;
	}
	if (((r<0) || (r>255)) ||
		((g<0) || (g>255)) ||
		((b<0) || (b>255))) {
		warn( tr("r,g,b must be from 0 to 255")+" !" );
		return operator;
	}

	// --------  set color of operator  ---------

	//
	// Create
	//  q
	//  r g b rg/RG
	//  oper
	//  Q
	//
	if (globchange) {
		composite = createCompositeOperator ("","");
	}else {
		composite = createCompositeOperator ("q","Q");
	}


	var cntNon = operator.containsNonStrokingOperator();
	var cntStr = operator.containsStrokingOperator();

	// E.g if text found put rg, if lin put RG, can be both
	if (cntNon) {
		putnscolor (composite,r,g,b);
	}
	if (cntStr) {
		putscolor (composite,r,g,b);
	}
	
	// It the operator does not contain any known operators put them both
	if (!cntNon && !cntStr)	{
		putnscolor (composite,r,g,b);
		putscolor (composite,r,g,b);
	}

	composite.pushBack( operator.clone() );
	if (!globchange)
		putendq(composite);

	operator.stream().replace(operator, composite);
}

/**
 * Get font and font size of a text operator.
 * We have to find preceding Font operator.
 *
 * @param operator Pdf operator.
 */
function operatorGetFont(operator) {
	
	// === Check parameters  ===

	if ("PdfOperator" != operator.type()) {
		warn (tr("Operator must be")+" QSPdfOperator !");
		return;
	}

	// ===  Get the preceding font operator ===

	//  Traverse it backwards
	var fontoper = operator.fontIterator(false).current();
	if (undefined == fontoper) {
		warn (tr("No preceding font operator found..."));
		return;
	}
		
	var firstoper = fontoper.params().property(0);
	var secondoper = fontoper.params().property(1);
	if ('IProperty' != firstoper.type() || 'IProperty' != secondoper.type()) {
		print (firstoper.type());
		warn (tr("Invalid operand type"));
		return;
	}
	var fonttype = firstoper.value();
	var fontsize = secondoper.getText();
	return [fonttype, fontsize];
}

function getFontOperatorForOp( operator ) {
	var it = operator.iterator();

	while (! it.isBegin()) {
		var h_name = it.current().getName();
		switch (h_name) {
			case "Tf":
					return it.current();	// break
			case "BT":
			case "ET":
					return ;
		}

		it.prev();
	}

	return ;
}

/**
 * Set font and font size of a text operator.
 * We have to find preceding Font operator.
 *
 * @param thepage Page in which is operator \a operator viewed.
 * @param operator Pdf operator.
 * @param fontid Font id of the new font. Not the font name.
 * @param fontsize Font size, but this can vary from normal pixel size.
 */
function operatorSetFont(thepage, operator, fontid, fontsize) {

	// == Check type

	if ("PdfOperator" != operator.type()) {
		warn( tr("Operator must be") + " QSPdfOperator !");
		return;
	}

	if (thepage == undefined) {
		warn( tr("Page parameter must be defined!") );
		return;
	}

	var ctm = getTextDetransformationMatrix ( thepage, op );
	var h_null = transformationMatrixMul (  0, 0 , ctm );
	var h_delta = transformationMatrixMul (  0, fontsize , ctm );

	var x = (h_delta[0] - h_null[0]);
	var y = (h_delta[1] - h_null[1]);
	fontsize = Math.sqrt( x * x + y * y );

	old_font = getFontOperatorForOp( operator );
	//
	// fontid fontsize Tf
	// oper
	// old_fontid old_fontsize Tf
	//
	var composite = createCompositeOperator ("q","Q");

	putfont(composite,fontid,fontsize);

    /* Put the changed operator also in the queue */
	composite.pushBack (operator.clone());

	composite.pushBack (old_font.clone());
	putendq(composite);

	// replace it
	operator.stream().replace (operator, composite);
}

/**
 * Set linewidth of an operator.
 *
 * @param operator Pdf operator.
 * @param Width of a line. 0 means tiniest.
 * @param globchange If true change is global.
 */
function operatorSetLineWidth(operator, linewidth, globchange) {

	// == Check type

	if ("PdfOperator" != operator.type()) {
		warn( tr("Operator must be") + " QSPdfOperator !");
		return;
	}

	// == Set new font around this operator

	//
	// q
	// linewidth w
	// oper
	// Q
	//
	if (globchange) {
		composite = createCompositeOperator ("","");
	}else {
		composite = createCompositeOperator ("q","Q");
	}

	putlinewidth (composite,linewidth);

    /* Put the changed operator also in the queue */
	composite.pushBack (operator.clone());

	if (!globchange)
		putendq(composite);

	operator.stream().replace (operator, composite);
}

/**
 * Set simple dash pattern from provided alternatives.
 *
 * @param alt Dash style.
 * @param operator Operator.
 * @param globchange If true changes will be global, if false only selected operator will be changed.
 */
function operatorSetSimpleDashPattern(alt,operator,globchange) {
	var array = [];
	var step = 600;
 	switch(alt) {
	case 0 :
		break;
	case 1 :
		array = [3];
		break;
	case 2 :
		array = [2,1];
		break;
	default:
		warn (tr("This option is not supported."));
		return false;
	}
	operatorSetDashPattern(operator,array,0,globchange);
	return true;
 }

/**
 * Set dash pattern of an operator.
 *
 * @param operator Pdf operator.
 * @param array The dash array's elements are numbers that specify the lengths of alternating dashes and gaps.
 * @param phase The dash phase specifies the distance into the dash pattern at which to start the dash.
 * @param globchange If true changes will be global, if false only selected operator will be changed.
 */
function operatorSetDashPattern(operator, array, phase, globchange) {

	// == Check type

	if ("PdfOperator" != operator.type()) {
		warn( tr("Operator must be") + " QSPdfOperator !");
		return;
	}

	// == Set new font around this operator

	//
	// q
	// array phase d
	// oper
	// Q
	//
	if (globchange) {
		composite = createCompositeOperator("","");
	}else {
		composite = createCompositeOperator("q","Q");
	}

	var operands = createIPropertyArray ();
	var iparray = createArray();
	for (i=0; i<array.length; ++i)
		iparray.add(i,createInt(array[i]));
	operands.append (iparray);
	operands.append (createInt(phase));
	composite.pushBack (createOperator("d",operands), composite);

    /* Put the changed operator also in the queue */
	composite.pushBack (operator.clone());

	operands.clear();
	if (!globchange)
		putendq (composite);

	operator.stream().replace (operator, composite);
}



/** === Font utilities === */

/**
 * Get all system fonts from pdf specification v 1.5.
 * Every viewer should have default metrics for these fonts avaliable.
 */
function getSystemFonts() {
 return ["Times-Roman", "Helvetica", "Courier",
 		 "Symbol", "Times-Bold", "Helvetica-Bold",
		 "Courier-Bold", "ZapfDingbats", "Times-Italic",
		 "Helvetica-Oblique", "Courier-Oblique",
		 "Times-BoldItalic", "Helvetica-BoldOblique",
		 "Courier-BoldOblique"];
}

/**
 * Edit position of a text positioning operator.
 */
function operatorMovePosition(operator, dx, dy) {

	var pars = operator.params();
	var x = parseFloat (pars.property(0).value());
	var y = parseFloat (pars.property(1).value());
	pars.property(0).set(x + dx);
	pars.property(1).set(y + dy);
}

/**
 * Edit position of a text positioning operator.
 */
function operatorSetPosition(operator, dx, dy) {

	// == Set new operator position

	//
	// q
	// array phase d
	// oper
	// Q
	//
	var composite = createCompositeOperator("q","Q");

	puttextrelpos(composite,dx,dy);

    /* Put the changed operator also in the queue */
	composite.pushBack (operator.clone());

	puttextrelpos (composite,-dx,-dy)
	putendq(composite);

	operator.stream().replace (operator, composite);

}


/**
 * Get nearest text position operator and its x, y values
 */
function getPosInfoOfOperator (operator) {
	
	var txtit = operator.iterator();
	while (!txtit.isBegin()) {
		print (txtit.current().getName());
		if ("TD" == txtit.current().getName() ||
				"Td" == txtit.current().getName())
			return txtit.current();
		txtit.prev();
	}

	return undefined;
}

/**
 * Draw line from start position to end position.
 * Argument lines is array of array of 4 double (x1,y1, x2,y2).
 */
function operatorDrawLine ( lines, width, col ) {
	//
	// q
	// array phase d
	// oper
	// Q
	//
	var composite = createCompositeOperator("q","Q");

	var ctm = getDetransformationMatrix( page() );
	if (ctm != [1,0,0,1,0,0])
		composite.pushBack( createOperator_transformationMatrix( ctm ), composite );

	if (undefined != width) {
		putlinewidth (composite,width);
	}

	if (undefined != col) {
		putscolor(composite,col.red,col.green,col.blue);
	}
	
	var i = 0;
	for ( ; i < lines.length ; ++i ) {
		putline (composite, lines[i][0], lines[i][1], lines[i][2], lines[i][3]);
	}
	putenddraw (composite);

	putendq(composite,composite);

	var ops = createPdfOperatorStack();
	ops.append (composite);
	page().prependContentStream(ops);
}

/**
 * Draw rectangles specyfing left upper corner, width and height.
 * Rectangles is array of array of 4 double (x,y, width,height).
 */
function operatorDrawRect ( rectangles ,col ,widthLine, next_operator ) {
	
	//
	// q
	// array phase d
	// oper
	// Q
	//
	var composite = createCompositeOperator("q","Q");
	
	var ctm = [1,0,0,1,0,0];
	if ((undefined == next_operator) || (next_operator.type() != "PdfOperator")) {
		ctm = getDetransformationMatrix( page() );
	} else {
		ctm = getDetransformationMatrix( page(), next_operator );
	}
	if (ctm != [1,0,0,1,0,0])
		composite.pushBack( createOperator_transformationMatrix( ctm ), composite );

	if (undefined != widthLine) {
		putlinewidth (composite,widthLine);
	}

	if (undefined != col) {
		putscolor(composite,col.red,col.green,col.blue);
		putnscolor(composite,col.red,col.green,col.blue);
	}

	var i = 0;
	for ( ; i < rectangles.length ; ++i ) {
		putrect (composite, rectangles[i][0], rectangles[i][1], rectangles[i][2], rectangles[i][3]);
	}

	putfill (composite);
	putenddraw (composite);
	putendq(composite,composite);

	if ((undefined == next_operator) || (next_operator.type() != "PdfOperator")) {
		var ops = createPdfOperatorStack();
		ops.append (composite);
		page().prependContentStream(ops);
	} else
		operatorPutBeforOp( next_operator, composite );
}

/**
 * Add text line.
 */
function operatorAddTextLine (text,x,y,fname,fsize,opToPutBefore,col) {
	//
	// q
	// BT
	// rg col
	// fname fsize Tf
	// x y Td
	// text Tj
	// ET
	// Q
	//
	var q = createCompositeOperator("q","Q");
	var BT = createCompositeOperator("BT","ET");
	
	if ((undefined != opToPutBefore) && (opToPutBefore.type() == "PdfOperator"))
		q.pushBack( opToPutBefore, q );
	q.pushBack (BT,q);
	
	putfont(BT,fname,fsize);
	puttextrelpos (BT,x,y);
	if (undefined != col)
		putnscolor (BT,col.red,col.green,col.blue);
	puttext (BT,text);
	putendtext (BT);
	putendq(q);

	var ops = createPdfOperatorStack();
	ops.append (q);
	page().prependContentStream(ops);
}

/**
 * Function return transformation matrix using to operator.
 */
function getTransformationMatrix( page, op ) {
	var cs_count = page.getContentStreamCount() - 1;
	var ctm = [1,0,0,1,0,0];

	if ((undefined != op) && (op.type() == "PdfOperator")) {
		// find contentstream which contains operator op
		var h_cs_count = cs_count;
		var h_stream = op.stream();
		for ( ; h_cs_count >=0 ; --h_cs_count ) {
			if (page.getContentStream( h_cs_count ).equals( h_stream  )) {
				ctm = get_transformationMatrix( op, true, ctm );
				cs_count = h_cs_count - 1;
				break;
			}
		}
	};

	for ( ; cs_count >= 0 ; --cs_count ) {
		var stream = page.getContentStream( cs_count );
		if (! stream.isEmpty() )
			ctm = get_transformationMatrix( stream.getLastOperator(), false, ctm );
	}

	return ctm;
}

/**
 * Function return transformation matrix to eliminate previous transformation.
 */
function getDetransformationMatrix( page, op ) {
	var ctm = getTransformationMatrix( page, op );
	return transformationMatrixDiv( ctm, [1,0,0,1,0,0] );
}

/**
 * Function return transformation matrix using to operator with text transformation.
 */
function getTextTransformationMatrix( page, op ) {
	var ctm = [1,0,0,1,0,0];
	var ttm = ctm;

	ttm = get_textTransformationMatrix( op );
	ctm = getTransformationMatrix( page, op );

	return transformationMatrixMul( ttm, ctm );
}

/**
 * Function return transformation matrix to eliminate previous transformation with text transformation.
 */
function getTextDetransformationMatrix( page, op ) {
	var ctm = getTextTransformationMatrix( page, op );
	return transformationMatrixDiv( ctm, [1,0,0,1,0,0] );
}

/** Get only text from text operator.
 * @param op Text operator
 *
 * @return Text from operator \a op.
 */
function getTextFromTextOperator( op ) {
	if (! isTextOp( op ))
		return "";

	return op.getEncodedText();
}

/** Move text operator relative [dx,dy] */
function moveTextOp( thepage, op, dx, dy ) {
	if ((undefined == thepage) || (undefined == op) || (!isTextOp(op)) || (undefined == dx) || (undefined == dy))
		return;

	var ctm = getTextDetransformationMatrix ( thepage, op );
	var h_null = transformationMatrixMul ( 0, 0 , ctm );
	var h_delta = transformationMatrixMul ( dx, dy , ctm );

	dx = h_delta[0] - h_null[0];
	dy = h_delta[1] - h_null[1];

	//
	// q
	// dx dy Td
	// op
	// -dx -dy Td
	// Q
	//
	var composite = createCompositeOperator("q","Q");

	puttextrelpos ( composite, dx, dy );
	composite.pushBack( op.clone() );
	puttextrelpos ( composite, -dx, -dy );

	putendq( composite );

	// replace it
	op.stream().replace (op, composite);
}

/** Move operator relative [dx,dy] */
function moveOp( thepage, op, dx, dy ) {
	if ((undefined == thepage) || (undefined == op) || (undefined == dx) || (undefined == dy))
		return;

	var ctm = getDetransformationMatrix ( thepage, op );
	var h_null = transformationMatrixMul ( 0, 0 , ctm );
	var h_delta = transformationMatrixMul ( dx, dy , ctm );

	dx = h_delta[0] - h_null[0];
	dy = h_delta[1] - h_null[1];

	//
	// q
	// array cm
	// op
	// Q
	//
	var composite = createCompositeOperator("q","Q");

	composite.pushBack( createOperator_transformationMatrix( [1,0,0,1,dx,dy], "cm" ), composite );

	composite.pushBack( op.clone() );
	putendq( composite );

	// replace it
	op.stream().replace (op, composite);
}

function moveGraphicalOp( thepage, op, dx, dy ) {
	if ((undefined == op) || (!isGraphicalOp(op))  || (undefined == dx) || (undefined == dy))
		return;

	// TODO

	print("Move graphical operators is not implemented!");
}
