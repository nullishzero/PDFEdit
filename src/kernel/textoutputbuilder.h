/*                                                                              
 * PDFedit - free program for PDF document manipulation.                        
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko, 
 *                                              Miroslav Jahoda,       
 *                                              Jozef Misutka, 
 *                                              Martin Petricek                                             
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
class PageFragment;
class PageLine;
class PageColumn;


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
