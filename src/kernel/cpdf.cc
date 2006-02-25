/*
 * =====================================================================================
 *        Filename:  cpdf.cc
 *     Description: CPdf class implementation.
 *         Created:  01/28/2006 03:48:14 AM CET
 *          Author:  jmisutka (), 
 *         Changes: 2006/1/28 added mapping support
 *         			2006/1/30 tested objToString () -- ok
 *         			2006/2/8  after a long battle againd g++ and ld + collect I made
 *         					  the implementation of CPdf a .cc file
 * =====================================================================================
 */

// debug
#include "debug.h"

//
#include "iproperty.h"
#include "cobject.h"
#include "cobjectI.h"


// =====================================================================================

namespace pdfobjects
{

//
//
//
CPdf::CPdf () : CDict()
{
	printDbg (0,"CPdf constructor.");
}

		
//
// 
//
IProperty*
CPdf::getExistingProperty (const Object* o) const
{
	assert (NULL != o);
	printDbg (0,"getExistingProperty(" << (unsigned int) o << ");");

	// find the key, if it exists
	ObjectMapping::const_iterator it = objMap.find (o);
	return (it != objMap.end()) ? const_cast<IProperty*>((*it).second) : NULL ;
}

//
// 
//
IProperty*
CPdf::getExistingProperty (const IndiRef& pair) const
{
	printDbg (0,"getExistingProperty(pair);");

	// find the key, if it exists
	IndirectMapping::const_iterator it = indMap.find (pair);
	return (it != indMap.end()) ? const_cast<IProperty*>((*it).second) : NULL ;
}


//
//
//
void
CPdf::setObjectMapping (const Object* o, const IProperty* ip)
{
	assert (NULL != o);
	assert (NULL != ip);
	// Just check if there is no such mapping
	// if there is such mapping that is a problem
	assert (NULL == getExistingProperty(o));
	printDbg (0,"setObjectMapping();");
	
	objMap [o] = ip;
}

//
//
//
void
CPdf::setIndMapping (const IndiRef& pair, const IProperty* ip)
{
	assert (NULL != ip);
	// Just check if there is no such mapping
	// if there is such mapping that is a problem
	assert (NULL == getExistingProperty(pair));
	printDbg (0,"setIndMapping();");
	
	indMap [pair] = ip;
}


//
//
//
void
CPdf::delObjectMapping (const Object* o)
{
	assert (NULL != o);
	// Just check if there is no such mapping
	// if there is such mapping that is a problem
	assert (NULL != getExistingProperty(o));
	printDbg (0,"delPropertyMapping();");
	
	objMap.erase (o);
}


//
//
//
void
CPdf::delIndMapping (const IndiRef& pair)
{
	// Just check if there is no such mapping
	// if there is such mapping that is a problem
	assert (NULL != getExistingProperty(pair));
	printDbg (0,"delIndMapping();");
	
	indMap.erase (pair);
}


} // namespace pdfobjects
