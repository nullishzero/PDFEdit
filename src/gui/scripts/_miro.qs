/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006, 2007, 2008  PDFedit team: Michal Hocko,
 *                                              Miroslav Jahoda,
 *                                              Jozef Misutka,
 *                                              Martin Petricek
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
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

