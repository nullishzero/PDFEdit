// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80
/*
 * =====================================================================================
 *        Filename:  cinlineimage.cc
 *     Description:  
 *         Created:  05/01/2006 11:41:43 AM CET
 *          Author:  jmisutka (), 
 * =====================================================================================
 */

// static
#include "static.h"
//
#include "cinlineimage.h"
// 
//#include "cobject.h"

// =====================================================================================
namespace pdfobjects{
// =====================================================================================

using namespace std;
using namespace boost;
using namespace utils;

// =====================================================================================
namespace {
// =====================================================================================

const string CINLINEIMAGE_MIDDLE = "ID";
	
// =====================================================================================
} // namespace
// =====================================================================================
	
//
//
//
void
CInlineImage::getStringRepresentation (std::string& str) const
{
	//assert (!"Not implemented yet.");
	typedef std::vector<std::string> Names;
	Names names;
	CStream::dictionary.getAllPropertyNames (names);
	for (Names::const_iterator it = names.begin(); it != names.end(); ++it)
	{
		std::string tmp;
		CStream::dictionary.getProperty (*it)->getStringRepresentation (tmp);
		str += string ("/") + *it + string (" ") + tmp + string ("\n");
	}
	
	str += CINLINEIMAGE_MIDDLE;
	str += "<<< IMAGE >>>";
	
}

//
// 
//
::Object*
CInlineImage::_makeXpdfObject () const
{
	assert (!"Not implemented yet.");	
	return NULL;
}

// =====================================================================================
} /* namespace pdfobjects */
// =====================================================================================
