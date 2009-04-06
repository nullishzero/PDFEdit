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

#ifndef _TEXTSEARCHPRAMS_H_
#define _TEXTSEARCHPRAMS_H_

// xpdf includes
#include "kernel/xpdf.h"

//=====================================================================================
namespace pdfobjects {
//=====================================================================================



//=====================================================================================
// Text search parameters (loose xpdf parameters put into a simple structure)
// 	--  default values are in cpage.cc because we do not want to pollute global space.
//=====================================================================================

/** 
 * Text search parameters. 
 *
 * These parameters are used by xpdf when serching a text string.
 */
typedef struct TextSearchParams
{
	/** Paramaters */
	GBool startAtTop;		/**< Start searching from the top.    */
	double xStart; 			/**< Start searching from x position. */
	double yStart; 			/**< Start searching from y position. */
	double xEnd; 			/**< Stop searching from x position.  */
	double yEnd; 			/**< Stop searching from y position.  */

	/** Constructor. Default values are set. */
	TextSearchParams () : 
		startAtTop (DEFAULT_START_AT_TOP),
		xStart (DEFAULT_X_START), yStart (DEFAULT_Y_START), xEnd (DEFAULT_X_END), yEnd (DEFAULT_Y_END)
	{}

	//
	// Default values  
	// -- small hack to declare them as ints, to be able to init
	// them here (if double, we could not init them here because of the non
	// integral type compilator error))
	//
	static const GBool DEFAULT_START_AT_TOP = gTrue;	/**< Start at top. */

	static const int DEFAULT_X_START = 0;	/**< Default x position of left upper corner. */
	static const int DEFAULT_Y_START = 0;	/**< Default y position of left upper corner. */
	static const int DEFAULT_X_END = 0;		/**< Default x position of right upper corner. */
	static const int DEFAULT_Y_END = 0;		/**< Default y position of right upper corner. */


} TextSearchParams;


//=====================================================================================
} // namespace pdfobjects
//=====================================================================================


#endif // _TEXTSEARCHPRAMS_H_
