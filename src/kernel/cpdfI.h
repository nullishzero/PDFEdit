/*
 * =====================================================================================
 *        Filename:  cpdfI.h
 *     Description: CPdf class implementation.
 *         Created:  01/28/2006 03:48:14 AM CET
 *          Author:  jmisutka (), 
 *         Changes: 2006/1/28 added mapping support
 *         			2006/1/30 tested objToString () -- ok
 * =====================================================================================
 */

// debug
#include "debug.h"


// =====================================================================================
namespace pdfobjects
{

//
//
//
CPdf::CPdf () : CDict(NULL,sPdf)
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
	Mapping::const_iterator it = mapping.find (o);
	return (it != mapping.end()) ? const_cast<IProperty*>((*it).second) : NULL ;
}

//
//
//
void
CPdf::setPropertyMapping (const Object* o, const IProperty* ip)
{
	assert (NULL != o);
	assert (NULL != ip);
	// Just check if there is no such mapping
	// if there is such mapping that is a problem
	assert (NULL == getExistingProperty(o));
	printDbg (0,"setPropertyMapping();");
	
	mapping [o] = ip;
}


} // namespace pdfobjects
