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

using namespace boost;
	
//
// Begin tag and end tag are added by composite pdfoperator
//
const string CINLINEIMAGE_BEGIN = "";
const string CINLINEIMAGE_MIDDLE = "ID";
const string CINLINEIMAGE_END = "";
	
// =====================================================================================
} // namespace
// =====================================================================================

//
// Constructors
//

//
//
//
CInlineImage::CInlineImage (::Object& oDict, const CStream::Buffer& buf) : CStream ()
{
	kernelPrintDbg (debug::DBG_DBG, "");
	assert (objDict == oDict.getType());

	// Init dictionary, we do not have access to dictionary container
	utils::dictFromXpdfObj (this->dictionary, oDict);
	assert (0 < this->dictionary.getPropertyCount ());
	
	// Set buffer, do not use setRawBuffer because CStream would be ... copied
	std::copy (buf.begin(), buf.end(), std::back_inserter (this->buffer));
}

//
//
//
CInlineImage::CInlineImage (CPdf& p, ::Object& oDict, const CStream::Buffer& buf, const IndiRef& rf) : CStream ()
{
	kernelPrintDbg (debug::DBG_DBG, "");
	assert (objDict == oDict.getType());

	// Init dictionary, we do not have access to dictionary container
	utils::dictFromXpdfObj (this->dictionary, oDict);
	assert (0 < this->dictionary.getPropertyCount ());
	
	// Set pdf and ref
	setPdf (&p);
	setIndiRef (rf);
	
	// Set buffer, do not use setRawBuffer because CStream would be ... copied
	std::copy (buf.begin(), buf.end(), std::back_inserter (this->buffer));
}


//
// Get methods
//

//
//
//
void
CInlineImage::getStringRepresentation (std::string& str) const
{
	str += CINLINEIMAGE_BEGIN;
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
	for (Buffer::const_iterator it = buffer.begin(); it != buffer.end(); ++it)
		str +=  static_cast<std::string::value_type> (*it);
	str += CINLINEIMAGE_END;
}

// =====================================================================================
} /* namespace pdfobjects */
// =====================================================================================
