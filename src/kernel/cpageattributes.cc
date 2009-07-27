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

// static
#include "kernel/static.h"

// 
#include "kernel/cobject.h"
#include "kernel/cobjecthelpers.h"
#include "kernel/cpageattributes.h"
#include "kernel/displayparams.h"
#include "kernel/factories.h"
#include "kernel/cpdf.h"


// =====================================================================================
namespace pdfobjects {
// =====================================================================================

using namespace std;
using namespace boost;
using namespace utils;

//
//
//
void 
CPageAttributes::fillInherited(const shared_ptr<CDict> pageDict, InheritedAttributes& attrs)
{
	int initialized=0;

	// TODO consolidate code - get rid of copy & paste
	
	// resource field
	if(!attrs._resources.get())
	{
		// attrs.__resources field is not specified yet, so tries this dictionary
		if(pageDict->containsProperty(Specification::Page::RESOURCES))
		{
			shared_ptr<IProperty> prop=pageDict->getProperty(Specification::Page::RESOURCES);
			if(isRef(prop))
			{
				attrs._resources=getCObjectFromRef<CDict>(prop);
				++initialized;
			}
			else
				if(isDict(prop))
				{
					attrs._resources=IProperty::getSmartCObjectPtr<CDict>(prop);
					++initialized;
				}
		}
	}else
		++initialized;

	// mediabox field
	if(!attrs._mediaBox.get())
	{
		// attrs._mediaBox field is not specified yet, so tries this array
		if(pageDict->containsProperty(Specification::Page::MEDIABOX))
		{
			shared_ptr<IProperty> prop=pageDict->getProperty(Specification::Page::MEDIABOX);
			if(isRef(prop))
			{
				attrs._mediaBox=getCObjectFromRef<CArray>(prop);
				++initialized;
			}else
				if(isArray(prop))
				{
					attrs._mediaBox=IProperty::getSmartCObjectPtr<CArray>(prop);
					++initialized;
				}
		}
	}else
		++initialized;

	// cropbox field
	if(!attrs._cropBox.get())
	{
		// attrs._cropBox field is not specified yet, so tries this array
		if(pageDict->containsProperty(Specification::Page::CROPBOX))
		{
			shared_ptr<IProperty> prop=pageDict->getProperty(Specification::Page::CROPBOX);
			if(isRef(prop))
			{
				attrs._cropBox=getCObjectFromRef<CArray>(prop);
				++initialized;
			}else
				if(isArray(prop))
				{
					attrs._cropBox=IProperty::getSmartCObjectPtr<CArray>(prop);
					++initialized;
				}
		}
	}else
		++initialized;

	// rotate field
	if(!attrs._rotate.get())
	{
		// attrs._rotate field is not specified yet, so tries this array
		if(pageDict->containsProperty(Specification::Page::ROTATE))
		{
			shared_ptr<IProperty> prop=pageDict->getProperty(Specification::Page::ROTATE);
			if(isRef(prop))
			{
				attrs._rotate=getCObjectFromRef<CInt>(prop);
				++initialized;
			}else
				if(isInt(prop))
				{
					attrs._rotate=IProperty::getSmartCObjectPtr<CInt>(prop);
					++initialized;
				}
		}
	}else
		++initialized;

	// all values available from this dictionary are set now
	if(initialized<4)
	{
		// not everything from InheritedAttributes is initialized now
		// tries to initialize from parent.
		// If parent is not present (root of page tree hierarchy is reached),
		// stops recursion and initializes values with default
		if(pageDict->containsProperty(Specification::Page::PARENT))
		{
			shared_ptr<IProperty> parentRef=pageDict->getProperty(Specification::Page::PARENT);
			if(!isRef(parentRef))
				// this should not happen - malformed page tree structure
				return;

			shared_ptr<CDict> parentDict=getCObjectFromRef<CDict>(parentRef);
			CPageAttributes::fillInherited(parentDict, attrs);
		}else
		{
			// Resources is required and at least empty dictionary should be
			// specified 
			if(!attrs._resources.get())
				attrs._resources=shared_ptr<CDict>(CDictFactory::getInstance());

			// default A4 sized box
			libs::Rectangle defaultRect(
					DisplayParams::DEFAULT_PAGE_LX, 
					DisplayParams::DEFAULT_PAGE_LY, 
					DisplayParams::DEFAULT_PAGE_RX, 
					DisplayParams::DEFAULT_PAGE_RY
					);

			// MediaBox is required and specification doesn't say anything about
			// default value - we are using standard A4 format
			if(!attrs._mediaBox.get())
				attrs._mediaBox=IProperty::getSmartCObjectPtr<CArray>(getIPropertyFromRectangle(defaultRect));

			// CropBox is optional and specification doesn't say anything about
			// default value - we are using standard A4 format
			if(!attrs._cropBox.get())
			{
					libs::Rectangle mediaRect;
					getRectangleFromProperty(attrs._mediaBox, mediaRect);
					attrs._cropBox=IProperty::getSmartCObjectPtr<CArray>(
							getIPropertyFromRectangle(mediaRect));
			}
			
			// Rotate is optional and specification defines default value to 0
			if(!attrs._rotate.get())
			{
				// gcc workaround
				// direct usage of static DEFAULT_ROTATE value caused linkage
				// error
				int defRot=DisplayParams::DEFAULT_ROTATE;
				attrs._rotate=shared_ptr<CInt>(CIntFactory::getInstance(defRot));
			}
		}
	}
}
	


//
//
//
void 
CPageAttributes::setInheritable(shared_ptr<CDict>& pageDict)
{
	InheritedAttributes attrs;
	CPageAttributes::fillInherited(pageDict, attrs);

	// checks Resources
	if(!pageDict->containsProperty(Specification::Page::RESOURCES))
		pageDict->addProperty(Specification::Page::RESOURCES, *(attrs._resources));
	
	// checks MediaBox
	if(!pageDict->containsProperty(Specification::Page::MEDIABOX))
		pageDict->addProperty(Specification::Page::MEDIABOX, *(attrs._mediaBox));
	
	// checks CropBox
	if(!pageDict->containsProperty(Specification::Page::CROPBOX))
		pageDict->addProperty(Specification::Page::CROPBOX, *(attrs._cropBox));
	
	// checks Rotate
	if(!pageDict->containsProperty(Specification::Page::ROTATE))
		pageDict->addProperty(Specification::Page::ROTATE, *(attrs._rotate));
}


// =====================================================================================
} // namespace pdfobjects
// =====================================================================================
