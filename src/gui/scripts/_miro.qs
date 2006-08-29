//-----------------------------   O K   -----------------------------
function rotateAndHalf( page ) {
	var mediaBox = page.mediabox();
	var w = mediaBox[2]-mediaBox[0];
	var h = mediaBox[3]-mediaBox[1];
	var pomer = w/h;
	var ctm1 = [0,0,0,0,0,0];
	var ctm2 = [0,0,0,0,0,0];
	if (pomer > 1) {
		var pomer2 = 2/pomer;
		if (pomer < pomer2)
			pomer = 1/pomer2;
		else
			pomer = 1/pomer;
		ctm1 = [0,pomer,-pomer,0,mediaBox[2] - w / 2, 0 ];
		ctm2 = [0,pomer,-pomer,0,mediaBox[2], 0 ];
	} else {
		var pomer2 = 1/(2 * pomer);
		if (pomer > pomer2)
			pomer = pomer2;
		ctm1 = [0,pomer,-pomer,0,mediaBox[2],0];
		ctm2 = [0,pomer,-pomer,0,mediaBox[2], mediaBox[3] - h / 2];
	}
	page.setTransformMatrix( ctm2 );
	return "["+ctm1.toString()+ "]  [" + ctm2.toString() +"]";
}
//print(rotateAndHalf(page()));
//go()

//-------------------------------------------------------------------
//-------------------------    ODLADIT    ---------------------------
//-------------------------------------------------------------------
function ctmToBack( oldCTM : Array_of_6_boubles, newCTM : Array_of_6_boubles ) {
	var nowCTM = [0,0,0,0,0,0];
	var menovatel = oldCTM[0]*oldCTM[3]-oldCTM[1]*oldCTM[2];
	if (menovatel == 0) {
		warm(tr(" Matrix is in bad state !!!"));
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

function matrixMul( a, b ) {
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

function cmToDetransformation( operator: PdfOperator, before: bool, _ctm : Array_of_6_doubles ) {
	if (operator.type() != "PdfOperator")
		return ;

	var it = operator.iterator();
	var num_of_Q_without_q = 0;
	var ctm = [1,0,0,1,0,0];

	if (_ctm != undefined)
		ctm = _ctm;

	if ((undefined == before) || (before == true) || (! it.isBegin()))
		it.prev();

	while (! it.isBegin()) {
		var it_cur = it.current();
		var it_cur_name = it_cur.getName();
		if (it_cur_name == "Q") {
			++num_of_Q_without_q;
		} else if (it_cur_name == "q") {
			if (num_of_Q_without_q > 0)
				--num_of_Q_without_q;
		} else if ((num_of_Q_without_q == 0) && (it_cur_name == "cm")) {
			if (it_cur.paramCount() != 6) {
				warn(tr("Maybe bad stream. Operator 'cm' should have 6 parameters, but has %1 !").arg(it_cur.paramCount()));
				print( it_cur.getName() );
				return ;
			}
			it_cur = it_cur.params();
			var h_ctm = [ it_cur.property(0).value(),
					it_cur.property(1).value(),
					it_cur.property(2).value(),
					it_cur.property(3).value(),
					it_cur.property(4).value(),
					it_cur.property(5).value() ];
			ctm = matrixMul( h_ctm, ctm );
		}
		it.prev();
	}
	return ctmToBack( ctm, [1,0,0,1,0,0] );
}
