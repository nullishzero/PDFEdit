/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006, 2007, 2008  PDFedit team: Michal Hocko,
 *                                              Miroslav Jahoda,
 *                                              Jozef Misutka,
 *                                              Martin Petricek
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (in doc/LICENSE.GPL); if not, write to the 
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 * MA  02111-1307  USA
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit
 */
// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80

// static
#include "kernel/static.h"
// xpdf
#include "kernel/xpdf.h"
//
#include "kernel/pdfspecification.h"
#include "kernel/cobject.h"
#include "kernel/cpdf.h"
#include "kernel/cxref.h"


// =====================================================================================
namespace pdfobjects{
// =====================================================================================
namespace utils {
// =====================================================================================

using namespace std;
using namespace xpdf;
using namespace boost;


// =====================================================================================
//  Other functions
// =====================================================================================

//
//
//
bool
objHasParent (const IProperty& ip, boost::shared_ptr<IProperty>& indiObj)
{
	assert (hasValidPdf (ip));
	if (!hasValidPdf (ip))
		throw CObjInvalidOperation ();

	shared_ptr<CPdf> pdf = ip.getPdf ().lock ();
	IndiRef ref = ip.getIndiRef();
	if ( &ip == (indiObj=pdf->getIndirectProperty(ref)).get() )
		return false;
	else
		return true;
}
bool
objHasParent (const IProperty& ip)
{boost::shared_ptr<IProperty> indi;return objHasParent (ip,indi);}


	
//
//
//
template<typename T>
void 
parseStreamToContainer (T& container, ::Object& obj)
{
	assert (container.empty());
	if (!obj.isStream())
	{
		assert (!"Object is not stream.");
		throw XpdfInvalidObject ();
	}

	// Get stream length
	xpdf::XpdfObject xpdfDict; xpdfDict->initDict (obj.streamGetDict());
	xpdf::XpdfObject xpdfLen; xpdfDict->dictLookup ("Length", xpdfLen.get());
	assert (xpdfLen->isInt ());
	assert (0 <= xpdfLen->getInt());
	size_t len = static_cast<size_t> (xpdfLen->getInt());
	utilsPrintDbg (debug::DBG_DBG, "Stream length: " << len);
	// Get stream
	::Stream* xpdfStream = obj.getStream ();
	assert (xpdfStream);
	// Get base stream without filters
	xpdfStream->getBaseStream()->moveStart (0);
	Stream* rawstr = xpdfStream->getBaseStream();
	assert (rawstr);
	// \TODO THIS IS MAGIC (try-fault practise)
	rawstr->reset ();

	// Save chars
	int c;
	while (container.size() < len && EOF != (c = rawstr->getChar())) 
		container.push_back (static_cast<typename T::value_type> (c));
	
	utilsPrintDbg (debug::DBG_DBG, "Container length: " << container.size());
	
	if (len != container.size())
		utilsPrintDbg(debug::DBG_ERR, "Stream buffer length ("<<container.size()<<") doesn't match Length value ("<<len<<").")

	assert (len == container.size());
	// Cleanup
	obj.streamClose ();
	//\TODO is it really ok?
	rawstr->close ();
}
template void parseStreamToContainer<CStream::Buffer> (CStream::Buffer& container, ::Object& obj);


//
//
//
template<typename ITERATOR, typename OUTITERATOR>
void streamToString (const std::string& strDict, ITERATOR begin, ITERATOR end, OUTITERATOR out)
{
	// Insert dictionary
	std::copy (strDict.begin(), strDict.end(), out);
	
	// Insert header
	std::copy (Specification::CSTREAM_HEADER.begin(), Specification::CSTREAM_HEADER.end(), out);
	
	//Insert buffer
	std::copy (begin, end, out);

	// Insert footer
	std::copy (Specification::CSTREAM_FOOTER.begin(), Specification::CSTREAM_FOOTER.end(), out);
}
// Explicit initialization
template void streamToString<CStream::Buffer::const_iterator, std::back_insert_iterator<std::string> > 
	(const std::string& strDict, 
	 CStream::Buffer::const_iterator begin, 
	 CStream::Buffer::const_iterator end,
	 std::back_insert_iterator<std::string> out);

//
//
//
size_t 
stringToCharBuffer (Object & stringObject, CharBuffer & outputBuf)
{
using namespace std;
using namespace debug;

	utilsPrintDbg(DBG_DBG, "");
	if(stringObject.getType()!=objString)
	{
		utilsPrintDbg(DBG_ERR, "Given object is not a string. Object type="<<stringObject.getType());
		return 0;
	}

	// gets all bytes from xpdf string object, makes string valid (to some
	// escaping)
	::GString * s=stringObject.getString(); 
	string str;
	for(int i=0; i<s->getLength(); i++)
		str.append(1, s->getChar(i));
	string validStr;
	simpleValueToString<pString>(str, validStr);

	// creates buffer and fills it byte after byte from valid string
	// representation
	size_t len=validStr.length();
	char* buf = char_buffer_new (len);
	outputBuf = CharBuffer (buf, char_buffer_delete()); 
	for(size_t i=0; i<len; i++)
		buf[i]=validStr[i];
	
	return len;
}




//
//
//
void 
createIndirectObjectStringFromString  ( const IndiRef& rf, const std::string& val, std::string& output)
{
	ostringstream oss;

	oss << rf << " " << Specification::INDIRECT_HEADER << "\n";
	oss << val;
	oss << Specification::INDIRECT_FOOTER;

	output = oss.str ();
} 

// =====================================================================================
} /* namespace utils */
// =====================================================================================
} /* namespace pdfobjects */
// =====================================================================================
