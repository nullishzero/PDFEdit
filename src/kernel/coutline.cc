// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80
/*
 * =====================================================================================
 *        Filename:  coutline.cc
 *         Created:  02/06/2006 12:54:43 AM CET
 *          Author:  jmisutka (), 
 * =====================================================================================
 */

// static
#include "static.h"

#include "cobject.h"

// =====================================================================================
namespace pdfobjects {
// =====================================================================================

using namespace std;
using namespace boost;
using namespace utils;

//
//
//
bool isOutline (boost::shared_ptr<IProperty> ip)
{
	assert (ip);

	if (!isDict (ip))
		throw CObjInvalidObject ();
	
	// Cast to dict
	boost::shared_ptr<CDict> dict = IProperty::getSmartCObjectPtr<CDict> (ip);
	assert (dict);

	// Check type property
	if (!(dict->getProperty("Title")))
		throw CObjInvalidObject ();

	return true;
}

//
//
//
std::string getOutlineText (boost::shared_ptr<IProperty> ip)
{
	assert (ip);
	assert (isOutline(ip));

	if (!isOutline(ip))
		throw CObjInvalidObject ();
	
	// Get the string from the outline
	return utils::getStringFromDict (ip, "Title");
}

// =====================================================================================
} // namespace pdfobjects
// =====================================================================================
