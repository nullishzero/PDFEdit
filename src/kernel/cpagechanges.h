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

#ifndef _CPAGECHAGNES_H_
#define _CPAGECHAGNES_H_

// static includes
#include "kernel/static.h"
#include "kernel/cpagemodule.h"


//==========================================================
namespace pdfobjects {
//==========================================================

// Forward declaration
class CPage;
class CContentStream;


/**
 * Class representing changes in a page.
 */
class CPageChanges : public ICPageModule
{
	// typedef
public:
	typedef std::vector<boost::shared_ptr<CContentStream> > Changes;

	// variables
private:
	CPage* _page;

	// ctor & dtor
public:
	/** Ctor. */
	CPageChanges (CPage* page) : _page(page) {}
	/** Dtor. */
	~CPageChanges ()
		{ _page = NULL; }


	//
	// Methods
	//
public:

	/**
	 * Get n-th change.
	 * Higher change means older change.
	 */
	boost::shared_ptr<CContentStream> getChange (size_t nthchange = 0) const;

	/**
	 * Get our changes sorted.
	 * The first change is the last change. If there are no changes
	 * container is empty.
	 */
	void getChanges (Changes& cont) const;

	/**
	 * Get count of our changes.
	 */
	size_t getChangeCount () const;

	/**
	 * Draw nth change on an output device with last used display parameters.
	 *
	 * @param out Output device.
	 * @param cont Container of content streams to display
	 */
	template<typename Container>
	void displayChange (::OutputDev& out, const Container& cont) const;
	void displayChange (::OutputDev& out, const std::vector<size_t>& cs) const;


}; // class CPageChanges


//==========================================================
} // namespace pdfobjects
//==========================================================


#endif // _CPAGECHAGNES_H_

