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
// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80

#ifndef _DISPLAYPARAMS_H_
#define _DISPLAYPARAMS_H_

// all basic includes
#include "kernel/static.h"


//=====================================================================================
namespace pdfobjects {
//=====================================================================================


//=====================================================================================
// Display parameters (loose xpdf parameters put into a simple structure)
// 	--  default values are in cpage.cc because we do not want to have global variables.
//=====================================================================================

/** 
 * Graphical state parameters. 
 *
 * These parameters are used by xpdf when updating bounding boxex of content stream operators,
 * displaying page etc.
 */
typedef struct DisplayParams
{
	/** Paramaters */
	double 		hDpi;		/**< Horizontal DPI. */
	double 		vDpi; 		/**< Vertical DPI. 	*/
	libs::Rectangle pageRect;/**< Page rectangle. */
	int 		rotate;		/**< Page rotation. 	*/
	GBool		useMediaBox;/**< Use page media box. */
	GBool		crop;		/**< Crop the page. 	*/
	GBool		upsideDown;	/**< Upside down. 	*/
	
	/** Constructor. Default values are set. */
	DisplayParams () : 
		hDpi (DEFAULT_HDPI), vDpi (DEFAULT_VDPI),
		pageRect (libs::Rectangle (DEFAULT_PAGE_LX, DEFAULT_PAGE_LY, DEFAULT_PAGE_RX, DEFAULT_PAGE_RY)),
		rotate (DEFAULT_ROTATE), useMediaBox (gTrue), crop (gFalse), upsideDown (gTrue) 
		{}



	/** Equality operator. */
	bool operator== (const DisplayParams& dp) const
	{
		return (hDpi == dp.hDpi && vDpi == dp.vDpi &&
				pageRect == dp.pageRect && rotate == dp.rotate &&
				useMediaBox == dp.useMediaBox && crop == dp.crop &&
				upsideDown == dp.upsideDown);
	}

	/** Converting position from pixmap of viewed page to pdf position.
	 * @param fromX	X position on viewed page.
	 * @param fromY	Y position on viewed page.
	 *
	 * @param toX	return X position in pdf page.
	 * @param toY	return Y position in pdf page.
	 *
	 * @see convertPdfPosToPixmapPos
	 */
	void convertPixmapPosToPdfPos( double fromX, double fromY, double & toX, double & toY ) const {
		const double * ctm /*[6]*/;
		double h;
		PDFRectangle pdfRect ( pageRect.xleft, pageRect.yleft, pageRect.xright, pageRect.yright );
		GfxState state (hDpi, vDpi, &pdfRect, rotate, upsideDown );
		ctm = state.getCTM();

		h = (ctm[0]*ctm[3] - ctm[1]*ctm[2]);

		assert( h != 0 );

		toX = (fromX*ctm[3] - ctm[2]*fromY + ctm[2]*ctm[5] - ctm[4]*ctm[3]) / h;
		toY = (ctm[0]*fromY + ctm[1]*ctm[4] - ctm[0]*ctm[5] - ctm[1]*fromX) / h;
	}

	/** Converting pdf position to position on pixmap of viewed page.
	 * @param fromX	X position in pdf page.
	 * @param fromY	Y position in pdf page.
	 *
	 * @param toX	return X position on viewed page.
	 * @param toY	return Y position on viewed page.
	 *
	 * @see convertPixmapPosToPdfPos
	 */
	void convertPdfPosToPixmapPos( double fromX, double fromY, double & toX, double & toY ) const {
		PDFRectangle pdfRect ( pageRect.xleft, pageRect.yleft, pageRect.xright, pageRect.yright );
		GfxState state (hDpi, vDpi, &pdfRect, rotate, upsideDown );

		state.transform( fromX, fromY, &toX, &toY );
	}

	//
	// Default values
	// -- small hack to declare them as ints, to be able to init
	// them here (if double, we could not init them here because of the non
	// integral type compilator error))
	// 
	static const int DEFAULT_HDPI 	= 72;		/**< Default horizontal dpi. */
	static const int DEFAULT_VDPI 	= 72;		/**< Default vertical dpi. */
	static const int DEFAULT_ROTATE	= 0;		/**< No rotatation. */

	static const int DEFAULT_PAGE_LX = 0;		/**< Default x position of left upper corner. */
	static const int DEFAULT_PAGE_LY = 0;		/**< Default y position of right upper corner. */
	static const int DEFAULT_PAGE_RX = 612;		/**< Default A4 width on a device with 72 horizontal dpi. */
	static const int DEFAULT_PAGE_RY = 792;		/**< Default A4 height on a device with 72 vertical dpi. */

} DisplayParams;


//=====================================================================================
} // namespace pdfobjects
//=====================================================================================


#endif // _DISPLAYPARAMS_H_
