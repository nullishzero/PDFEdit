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

#ifndef _CONTENTSCHANGETAG_H_
#define _CONTENTSCHANGETAG_H_

// static includes
#include "kernel/static.h"

//==========================================================
namespace pdfobjects {
//==========================================================

// Forward declarations
class PdfOperator;

/**
 * Content stream change tag
 */
class ContentsChangeTag
{
	// Constants
public:
	static const char* CHANGE_TAG_NAME;
	static const char* CHANGE_TAG_ID;


	// Static methods
public:
	/**
	 * Create our own tag indicating time when the change occured. This enables us
	 * to sort our changes according to time.
	 *
	 * @return Operator that identifies our changes
	 */
	static boost::shared_ptr<PdfOperator> create ();

	/**
	 * Get change tag time.
	 */
	static time_t getTime (boost::shared_ptr<PdfOperator> op);

};


//==========================================================
} // namespace pdfobjects
//==========================================================


#endif // _CONTENTSCHANGETAG_H_

