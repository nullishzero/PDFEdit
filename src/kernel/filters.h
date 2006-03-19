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
		
/**
 * This is an implementation of STRATEGY design pattern. We need a set of filters, which
 * behave differently but on the same data with the same information avaliable. We make
 * them interchangeable. [GoF/Strategy]
 *
 * We also make 2 crucial decisions, that allow us this some sort of a "special" imlementation
 * of Strategy pattern.
 * 
 * 1. We can select the filter at compile time
 * 		i) 	we do not want to expose filters outside kernel -- because of simplicity
 * 		ii)	we know which filters are implemented, thanks to i)
 * 2. Filters won't be changed at runtime
 * 		i)	we won't have filters associated with Streams 1:1, they will be created on demand.
 *
 * If we would like to expose this interface outside, we would have to create a Mediator between Filter class
 * and CStream. Nothing more, nothing less.
 * 
 */
template <class SpecialFilter>
class CFilter 
{
typedef std::list<std::string> Params;

private:
	/** Specialized filter. */
	SpecialFilter filter;

	/** Parameters needed for compression. */
	Params params;

public:

	/** Constructor. */
	CFilter (Params pars) : params(pars) { printDbg (0,"Filter constructed."); };

	/** Encode. */
	void operator() () const { printDbg (0,"Filter::operator()."); filter ();};


	~CFilter () { printDbg (0,"Filter destroyed."); }
};
  


/**
 * Specific filter.
 *
 */
class NoFilter
{
public:
	/** Do the endcoding. */
	void operator() () const;
};


//=======================================
} // namespace filters
//=======================================


#endif // _FILTERS_H_
