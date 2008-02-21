/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006, 2007, 2008  PDFedit team: Michal Hocko,
 *                                              Miroslav Jahoda,
 *                                              Jozef Misutka,
 *                                              Martin Petricek
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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

// static
#include "kernel/static.h"

#include "kernel/cobject.h"

// =====================================================================================
namespace pdfobjects {
// =====================================================================================

using namespace std;
using namespace boost;
using namespace utils;

//
//
//
bool isOutline (boost::shared_ptr<IProperty> ip)
{
	assert (ip);

	if (!isDict (ip))
		throw CObjInvalidObject ();
	
	// Cast to dict
	boost::shared_ptr<CDict> dict = IProperty::getSmartCObjectPtr<CDict> (ip);
	assert (dict);

	// Check type property
	if (!(dict->getProperty("Title")))
		throw CObjInvalidObject ();

	return true;
}

//
//
//
std::string getOutlineText (boost::shared_ptr<IProperty> ip)
{
	assert (ip);
	assert (isOutline(ip));

	if (!isOutline(ip))
		throw CObjInvalidObject ();
	
	// Get the string from the outline
	return utils::getStringFromDict (ip, "Title");
}

// =====================================================================================
} // namespace pdfobjects
// =====================================================================================
