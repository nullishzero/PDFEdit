/*
 * =====================================================================================
 *        Filename:  cpdf.cc
 *     Description: CPdf class implementation.
 *         Created:  01/28/2006 03:48:14 AM CET
 *          Author:  jmisutka (), 
 *         Changes: 2006/1/28 added mapping support
 * =====================================================================================
 */


#include "cpdf.h"

// debug
#include "debug.h"

// =====================================================================================
namespace pdfobjects
{

//
// 
//
IProperty*
CPdf::getExistingProperty (const ObjNum id, const GenNum genId) const
{
	assert (0 < id);
	printDbg (0,"getExistingProperty(" << id << "," <<  genId << ");");

	// find the key, if it exists
	Mapping::const_iterator it = mapping.find (std::make_pair(id,genId));
	return (it != mapping.end()) ? const_cast<IProperty*>((*it).second) : NULL ;
}

//
//
//
void
CPdf::setPropertyMapping (const ObjNum n, const GenNum g, const IProperty* ip)
{
	assert (0 < n);
	assert (NULL != ip);

	mapping [std::make_pair(n,g)] = ip;
}



} // namespace pdfobjects
