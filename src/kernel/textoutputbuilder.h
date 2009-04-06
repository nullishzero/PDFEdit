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

#ifndef _TEXTOUTPUTBUILDER_H_
#define _TEXTOUTPUTBUILDER_H_

// basic types
#include "kernel/static.h"

// Basic engines
#include "kernel/textoutputengines.h"

//=====================================================================================
namespace textoutput {
//=====================================================================================


//=====================================================================================
// Output builder
//=====================================================================================


//
// Forward declarations
//
struct PageFragment;
struct PageLine;
struct PageColumn;


//
// Page builder base class
//

/**
 * Page builder.
 */

struct OutputBuilder
{
typedef SimpleWordEngine::Iterator 	PageFragmentIterator;
typedef SimpleColumnEngine::Iterator PageColumnIterator;

protected:
	size_t _pagepos;	/**< Page position of actual page. */

	//
	// Ctor
	//
public:
	OutputBuilder () : _pagepos (std::numeric_limits<size_t>::max()) {}

	//
	// Building interface
	//
public:

	/** Build output from columns. */
	virtual void build (PageColumnIterator, PageColumnIterator) = 0;
	
	/** Build output from fragments. */
	virtual void build (PageFragmentIterator, PageFragmentIterator) = 0;

	/** Start page. */
	void start_page (size_t pagepos)
	{ 
		assert (std::numeric_limits<size_t>::max() == _pagepos); 
		_pagepos = pagepos; 
	}

	/** End page. */
	void end_page ()
	{ 
		_pagepos = std::numeric_limits<size_t>::max(); 
	}

	//
	// Dtor
	//
public:
	virtual ~OutputBuilder () {}

};


//
// Xml output
//

/**
 * Page xml builder.
 */

class XmlOutputBuilder : public OutputBuilder
{

private:
	std::string _str;

	//
	// Building interface
	//
public:

	/** Build output from fragments. */
	void build (PageColumnIterator it_s, PageColumnIterator it_e);
	void build (PageFragmentIterator it_s, PageFragmentIterator it_e);

	/** Get result without xml header and footer. */
	std::string str () const 
		{ return _str; }

	//
	// Static functions
	//
public:	
	/** Get xml output. */
	static std::string xml (const XmlOutputBuilder& out);
};


//=====================================================================================
} // namespace textouput
//=====================================================================================

#endif // _TEXTOUTPUTBUILDER_H_
