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

