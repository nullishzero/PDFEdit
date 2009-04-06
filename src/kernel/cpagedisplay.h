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

#ifndef _CPAGEDISPLAY_H
#define _CPAGEDISPLAY_H

// all basic includes
#include "kernel/static.h"
#include "kernel/cpagemodule.h"
#include "kernel/displayparams.h"	// DisplayParams

//=====================================================================================
namespace pdfobjects {
//=====================================================================================

// Forward declarations
class CPage;
class CDict;


//=====================================================================================
// CPageDisplay 
//=====================================================================================

/**
 * This class is representing the page display module.
 */
class CPageDisplay : public ICPageModule
{
	// Variables
private:
	/** Pdf dictionary representing a page. */
	CPage* _page;
	/** Actual display parameters. */
	DisplayParams _params;


	// Ctor & Dtor
public:
	CPageDisplay (CPage* page) : _page(page) {}
	~CPageDisplay () 
		{ _page = NULL; }


	//
	// Display properties
	//
public:

	/** 
	 * Returns rotation in degrees.
	 */
	int getRotation () const;

	/** 
	 * Sets rotation in degrees. 
	 */
	void setRotation (int rot);

	/**  
	 * Returns media box of this page. 
	 *
	 * It is a required item in page dictionary (spec p.119) but can be
	 * inherited from a parent in the page tree.
	 *
	 * @return Rectangle specifying the box.
	 */
	libs::Rectangle getMediabox () const;
	
	/** 
	 * Sets media box of this page. 
	 * @param rc Rectangle for the media box.
	 *
	 * As a side effect it changes also CropBox and TrimBox if they are
	 * set.
	 */
	void setMediabox (const libs::Rectangle& rc);

	/**  
	 * Returns rect of this page. 
	 */
	libs::Rectangle getPageRect () const;
	
	/**
	 * Sets transform matrix of a page. This operator will be preceding first cm
	 * operator (see pdf specification), if not found it will be the first operator.
	 *
	 * @param tm Six number representing transform matrix.
	 */
	void setTransformMatrix (double tm[6]);


	//
	// Display methods
	//
public:

	/**
	 * Sets display params.
	 */
	void setDisplayParams (const DisplayParams& dp);

	/**
	 * Draws page on an output device.
	 * Use old display params.
	 */
	void displayPage (::OutputDev& out, int x = -1, int y = -1, int w = -1, int h = -1);

	/**
	 * Draws page using specified page dictionary on an output device with last used display parameters.
	 *
	 * @param out Output device.
	 * @param dict If not null, page is created from dict otherwise
	 * this page dictionary is used. But still some information is gathered from this page dictionary.
	 */
	void displayPage (::OutputDev& out, 
					  boost::shared_ptr<CDict> pagedict, 
					  int x = -1, int y = -1, int w = -1, int h = -1);

	/** 
	 * Creates xpdf's state and resource parameters. 
	 */
	void createXpdfDisplayParams (boost::shared_ptr<GfxResources>& res, 
								  boost::shared_ptr<GfxState>& state);


}; // class CPageDisplay


//=====================================================================================
} // namespace pdfobjects
//=====================================================================================


#endif // _CPAGEDISPLAY_H
