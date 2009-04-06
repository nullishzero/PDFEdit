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

#ifndef _CPAGEATTRIBUTES_H
#define _CPAGEATTRIBUTES_H

#include "kernel/cobject.h"
#include "kernel/cpagemodule.h"

//=====================================================================================
namespace pdfobjects {
//=====================================================================================

// Forward declarations
class CDict;
class CArray;

/**
 * Helper class for page attributes.
 */
class CPageAttributes
{

	//
	// Typedefs
	//
public:
	/** 
	 * Page attributes structure of dictionary properties which can be inherited from a parent 
	 * in the page tree.
	 *
	 * If an inheritable property is not present in a page it is defined in one of
	 * its parents in the page tree.
	 */
	struct InheritedAttributes
	{
		boost::shared_ptr<CDict> _resources;
		boost::shared_ptr<CArray> _mediaBox;
		boost::shared_ptr<CArray> _cropBox;
		boost::shared_ptr<CInt> _rotate;
	};

	//
	// Methods
	//
public:
	/** 
	 * Fills InheritedAttributes structure for a given page dictionary.
	 *
	 * Recursive function which checks given pageDict whether it contains
	 * uninitialized (NULL values) from a given attribute structure. If true, sets
	 * the value from the dictionary. If at least one property is still not 
	 * initialized, repeats the process for the parent dictionary (dereferenced 
	 * "Parent" property). End if the "Parent" property is not present (in root 
	 * of a page tree).
	 * <br>
	 * Use default value when a property was not initialized.
	 * <br>
	 * Note that attrs structure comes out allways initialized when recursion is
	 * finished.
	 *
	 * @param pageDict Page dictionary.
	 * @param attrs Output attribute structure where correct values are put.
	 *
	 * @throw NotImplementedException at this moment.
	 */
	static void fillInherited(const boost::shared_ptr<CDict> pageDict, InheritedAttributes& attrs);

	/** 
	 * Sets unitialized inheritable page attributes.
	 * @param pageDict Page dictionary reference where to set values.
	 *
	 * Gets InheritedAttributes structure for given pageDict (uses
	 * fillInheritedPageAttr helper function) and sets all fields which are not
	 * present in given dictionary to found values.
	 */
	static void setInheritable(boost::shared_ptr<CDict>& pageDict);

};




//=====================================================================================
} // namespace pdfobjects
//=====================================================================================


#endif // _CPAGEATTRIBUTES_H
