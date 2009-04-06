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

#ifndef _CPAGEMODULE_H
#define _CPAGEMODULE_H

#include "kernel/static.h"

//=====================================================================================
namespace pdfobjects {
//=====================================================================================

/**
 * Base class for all page modules.
 *
 * CPage represents a page according to pdf specification. Page is a dictionary 
 * containing several mandatory and several optional entries. The basic idea
 * behind page modules is that most of these entries are complex and standalone and 
 * therefore they are detached from CPage implementation.
 *
 * We can choose which optional entries to support and create new module for it. The
 * implementation of CPage is kept simple and clear.
 *
 * A module initialisation can fail e.g. when the pdf file is damaged. This should be 
 * announced in the init() function. 
 *
 * Module deinitialisation can fail too and it should be announced in the reset() function.
 *
 * TODO: 
 * 1) for performance issues these functions can be called multiple times during the module
 * lifecycle
 * 2) create a page module initialisation/deinitialisation exception
 *
 */
class ICPageModule : public noncopyable
{
	// 
	// Interface methods
	//
public:
	void init () {};
	void reset () {};
	// virutal dtor
	virtual ~ICPageModule () {};

}; // class ICPageModule


//=====================================================================================
} // namespace pdfobjects
//=====================================================================================


#endif // _CPAGEMODULE_H
