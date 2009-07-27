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

// Page
#include "kernel/cpage.h"
#include "kernel/contentschangetag.h"
#include "kernel/cpageattributes.h"
#include "kernel/cpagefonts.h"
#include "kernel/cpagedisplay.h"

#include "kernel/ccontentstream.h"
#include "kernel/cpdf.h"

#include "kernel/cobject.h"
#include "kernel/cobjecthelpers.h"
#include "kernel/factories.h"
#include "utils/observer.h"



// =====================================================================================
namespace pdfobjects {
// =====================================================================================

using namespace std;
using namespace boost;
using namespace utils;
using namespace observer;
using namespace debug;



//=====================================================================================
// CPage
//=====================================================================================



//
// Constructor
//
CPage::CPage (boost::shared_ptr<CDict>& pageDict) : 
		_dict(pageDict), 
		_valid (true)
{
		kernelPrintDbg (debug::DBG_DBG, "");
	
	// Fill inheritable properties but do not dispatch the change
	// if no change on this document occurs, we do not want to change it (but we
	// do it in the CPageAttributes::setInheritable function)
	// Workaround: pages that miss contents entry
	//  -- it is not a global solution one can delete and add
	//  contents entry of a page
	_dict->lockChange();
	CPageAttributes::setInheritable (_dict);
	if (!_dict->containsProperty(Specification::Page::CONTENTS))
	{
		CArray arr;
		_dict->addProperty (Specification::Page::CONTENTS, arr);
	}
	_dict->unlockChange();

	//
	// Init modules
	//
	_contents = shared_ptr<CPageContents> (new CPageContents(this));
	_modules.push_back (_contents);
	_display = shared_ptr<CPageDisplay> (new CPageDisplay(this));
	_modules.push_back (_display);
	_fonts = boost::shared_ptr<CPageFonts> (new CPageFonts(this));
	_modules.push_back (_fonts);
	_changes = boost::shared_ptr<CPageChanges> (new CPageChanges(this));
	_modules.push_back (_changes);
	_annots = boost::shared_ptr<CPageAnnots> (new CPageAnnots(this));
	_modules.push_back (_annots);

	// init modules
	for (Modules::iterator it = _modules.begin(); it != _modules.end(); ++it)
		(*it)->init ();
}


//
// Destructor
//
CPage::~CPage () 
{ 
		kernelPrintDbg (debug::DBG_INFO, "Page destroyed."); 
	invalidate ();

	// indicate we are ending
	// FIXME remove - added as workaround for 233
	//_dict->setPdf (boost::shared_ptr<CPdf>());

	// inform modules
	for (Modules::iterator it = _modules.begin(); it != _modules.end(); ++it)
		it->reset ();
	// cleanup
	_modules.clear ();
	_contents.reset ();
	_display.reset ();
	_fonts.reset ();
	_changes.reset ();
	_annots.reset ();
	_dict.reset ();
		assert (0 == _contents.use_count());
		assert (0 == _display.use_count());
		assert (0 == _fonts.use_count());
		assert (0 == _annots.use_count());
		assert (0 == _changes.use_count());
}


//
// Fonts
//

//
//
//
void 
CPage::getFontIdsAndNames (FontList& cont) const
{ 
	_fonts->getFontIdsAndNames (cont); 
}

//
//
//
std::string 
CPage::addSystemType1Font (const std::string& fontname, bool winansienc)
{ 
	return _fonts->addSystemType1Font (fontname, winansienc); 
}


//
// Display
//

//
//
//
int 
CPage::getRotation () const
{ 
	return _display->getRotation (); 
}

//
//
//
void 
CPage::setRotation (int rot)
{ 
	_display->setRotation (rot); 
}

//
//
//
libs::Rectangle 
CPage::getMediabox () const
{ 
	return _display->getMediabox(); 
}

void 
CPage::setMediabox (const libs::Rectangle& rc)
{ 
	_display->setMediabox (rc); 
}

//
//
//
void 
CPage::setTransformMatrix (double tm[6])
{ 
	_display->setTransformMatrix (tm); 
}

//
//
//
void 
CPage::setDisplayParams (const DisplayParams& dp)
{ 
	_display->setDisplayParams (dp); 
}

//
//
//
void 
CPage::displayPage (::OutputDev& out, const DisplayParams& params, int x, int y, int w, int h)
{ 
	_display->setDisplayParams (params);
	_display->displayPage (out, x, y, w ,h); 
}

//
//
//
void 
CPage::displayPage (::OutputDev& out, int x, int y, int w, int h)
{ 
	_display->displayPage (out, x, y, w ,h); 
}

//
//
//
void 
CPage::displayPage (::OutputDev& out, 
	 			    boost::shared_ptr<CDict> dict, 
				    int x, int y, int w, int h) const
{ 
	_display->displayPage (out, dict, x, y, w ,h); 
}


//
// Getters
//
size_t
CPage::getPagePosition () const
{
	if (hasValidPdf (_dict))
		return _dict->getPdf().lock()->getPagePosition (shared_ptr<CPage>(const_cast<CPage*>(this),EmptyDeallocator<CPage> ()));

	throw CObjInvalidOperation ();
}

//
//
//
void 
CPage::invalidate ()
{ 
	// indicate change
	_objectChanged (true); 
	_valid = false;
}


//
//
//
void
CPage::_objectChanged (bool invalid)
{
	// Do not notify anything if we are not in a valid pdf
		if (!hasValidPdf (_dict))
			return;
		assert (hasValidRef (_dict));

	boost::shared_ptr<CPage> current (this, EmptyDeallocator<CPage> ());

	// Notify observers
	if (invalid)
		this->notifyObservers (current, shared_ptr<const ObserverContext> ());
	else
		this->notifyObservers (current, shared_ptr<const ObserverContext> (new BasicObserverContext (current)));
}


// =====================================================================================
// Helper functions
// =====================================================================================

//
//
//
bool 
isPage (boost::shared_ptr<IProperty> ip)
{
	assert (ip);
	assert (isDict (ip));

	if (!isDict(ip))
		throw CObjInvalidObject ();

	boost::shared_ptr<CDict> dict = IProperty::getSmartCObjectPtr<CDict> (ip);

	if (Specification::Page::TYPE != getStringFromDict (dict, Specification::Dict::TYPE))
		throw CObjInvalidObject ();

	return true;
}


// =====================================================================================
} // namespace pdfobjects
// =====================================================================================
