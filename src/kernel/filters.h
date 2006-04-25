// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80
/*
 * =====================================================================================
 *        Filename:  filters.h
 *     Description:   class 
 *         Created:  03/19/2006 14:26:44  CET
 *          Author:  jmisutka ()
 * =====================================================================================
 */

#ifndef _FILTERS_H_
#define _FILTERS_H_

// static includes
#include "static.h"

//=======================================
namespace filters {
//=======================================

//
// Forward declarations
//
struct NoFilter;
	
//=======================================
// Filter factory
//=======================================

	
/**
 * Filter creator class. Factory design pattern implemented here.
 *
 * This is an implementation of STRATEGY design pattern. We need a set of filters, which
 * behave differently but on the same data with the same information avaliable. We make
 * them interchangeable. [GoF/Strategy]
 *
 * If we would like to expose this interface outside, we would have to create a Mediator between Filter class
 * and CStream. Nothing more, nothing less.
 * 
 * REMARK: We do not use template implementation because we do not know at compile time, which implementation will be used.
 * REMARK2: Change getSupportedStreams & setStringRepresentation in order to expose newly created filters.
 */
struct CFilterFactory
{
	/**
	 * Create filter class.
	 *
	 * @param filterName Name of the filter.
	 *
	 * @return Filter, if not found, NoFilter is created.
	 */
	template<typename OUTPUT, typename FILTERS>
	static void addFilters (OUTPUT& out, const FILTERS& filterNames)
	{
		out.push (NoFilter ());
	}

	/**
	 * Get all supported filters.
	 *
	 * @param supported Supported streams.
	 */
	template<typename Container>
	static void getSupportedStreams (Container& supported) 
	{
		supported.push_back ("NoFilter");
	}

};

//=======================================
// Concrete Filters
//=======================================

/**
 * Specific filter.
 */
struct NoFilter : public boost::iostreams::multichar_output_filter
{
	/** Default constructor. */
	NoFilter () { printDbg (debug::DBG_DBG, "NoFilter created."); };

	/** Multi char output function. */
	template<typename Sink>
	void write(Sink& snk, const char* s, std::streamsize n) 
	{ 
		while (n-- > 0)
			boost::iostreams::put (snk, *s);
	}

	/** Destructor. */
	~NoFilter () { printDbg (debug::DBG_DBG, "NoFilter destroyed."); };

};

//=======================================
} // namespace filters
//=======================================


#endif // _FILTERS_H_
