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
 * If we would like to expose this interface outside, we would have to create a Mediator between Filter class
 * and CStream. Nothing more, nothing less.
 * 
 * REMARK: We do not use template implementation because we do not know at compile time, which implementation will be used.
 * REMARK2: Change getSupportedStreams & setStringRepresentation in order to expose newly created filters.
 */
class CFilter 
{
public:
	typedef std::list<std::string> Params;

protected:
	CFilter () {};
		
public:

	/** Decode. */
	virtual void decode (const Params& /*params , char* ... */) const = 0;

	/** Destructor. */
	virtual ~CFilter () {};

};
  


/**
 * Specific filter.
 *
 */
class NoFilter : public CFilter
{
public:
	/** Default constructor. */
	NoFilter () { printDbg (0,"NoFilter created."); };
		
	/** Do the endcoding. */
	virtual void decode (const Params& /*params , char* ... */) const;

	/** Destructor. */
	~NoFilter () { printDbg (0,"NoFilter destroyed."); };

};


//=======================================
} // namespace filters
//=======================================


#endif // _FILTERS_H_
