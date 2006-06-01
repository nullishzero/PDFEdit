/** set color of operator
 * @param operator change color of this operator
 * @param r red component of color for set
 * @param g green component of color for set
 * @param b blue component of color for set
 *
 * If red,green or blue component is not set, view color dialog for set correct color.
 */
function setColorMy(operator,r,g,b) {
	// --------  check parameters  ---------

	if (operator.type() != "PdfOperator") {
		warn( tr("Operator must be QSPdfOperator !!!") );
		return ;
	}
	if ((r == undefined) || (g == undefined) || (b == undefined)) {
		var col = pickColor();
		if (isNaN(col)) {
			warn( tr("Must define color !!!") );
			return operator;
		}
		r=col.red;
		g=col.green;
		b=col.blue;
	}
	if (((typeof r ) != "number") ||
		((typeof g ) != "number") ||
		((typeof b ) != "number")) {
		warn( tr("r,g,b must be number !!!") );
		return operator;
	}
	if (((r<0) || (r>255)) ||
		((g<0) || (g>255)) ||
		((b<0) || (b>255))) {
		warn( tr("r,g,b must be from 0 to 255 !!!") );
		return operator;
	}

	// --------  set color of operator  ---------

	var prev = operator.iterator();
	prev.prev();
	var next = operator.iterator();
	next.next();

	oper.setPrev( createEmptyOperator() );
	oper.getLastOperator().setNext ( createEmptyOperator() );

	var composite = createCompositeOperator("q","Q");

	var operands = createIPropertyArray();
	operands.append(createReal(r/255));
	operands.append(createReal(g/255));
	operands.append(createReal(b/255));

	if (operator.containsNonStrokingOperator()) {
		composite.pushBack( createOperator(operands, "rg"),composite);
	} else if (operator.containsStrokingOperator()) {
		composite.pushBack( createOperator(operands, "RG"),composite);
	} else {
		composite.pushBack( createOperator(operands, "rg"),composite);
		composite.pushBack( createOperator(operands, "RG"),composite);
	}

	composite.pushBack( operator );
	operands.clear();
	composite.pushBack( createOperator(operands, "Q"), composite.getLastOperator() );

	operator.stream().replace(operator, composite, prev, next);
}
