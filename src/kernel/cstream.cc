/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006-2009  PDFedit team: Michal Hocko,
 *                                        Jozef Misutka,
 *                                        Martin Petricek
 *                   Former team members: Miroslav Jahoda
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

#include "kernel/static.h"
#include "kernel/cstream.h"
#include "kernel/cpdf.h"
#include "kernel/exceptions.h"
#include "kernel/factories.h"

//=====================================================================================
namespace pdfobjects {
//=====================================================================================

using namespace std;
using namespace boost;
using namespace utils;

//=====================================================================================
// CStream
//=====================================================================================

//
// Constructors
//

//
//
//
CStream::CStream (boost::weak_ptr<CPdf> p, const ::Object& o, const IndiRef& rf) : IProperty (p,rf), parser (NULL), tmpObj (NULL)
{
	kernelPrintDbg (debug::DBG_DBG,"");
	// Make sure it is a stream
	assert (objStream == o.getType());
	if (objStream != o.getType())
		throw XpdfInvalidObject ();
	
	// Get the dictionary and init CDict with it
	boost::shared_ptr< ::Object> objDict(XPdfObjectFactory::getInstance(), xpdf::object_deleter());
	const ::Dict* dict = o.streamGetDict ();
	assert (NULL != dict);
	objDict->initDict ((Dict *)dict);
	utils::complexValueFromXpdfObj<pDict,CDict::Value&> (dictionary, *objDict, dictionary.value);

	// Set pdf and ref
	dictionary.setPdf (p);
	dictionary.setIndiRef (rf);
	
	// Save the contents of the container
	utils::parseStreamToContainer (buffer, o);
}


//
//
//
CStream::CStream (const ::Object& o) : parser (NULL), tmpObj (NULL)
{
	kernelPrintDbg (debug::DBG_DBG,"");
	// Make sure it is a stream
	assert (objStream == o.getType());
	if (objStream != o.getType())
		throw XpdfInvalidObject ();
	
	// Get the dictionary and init CDict with it
	boost::shared_ptr< ::Object> objDict(XPdfObjectFactory::getInstance(), xpdf::object_deleter());
	const ::Dict* dict = o.streamGetDict();
	assert (NULL != dict);
	objDict->initDict ((Dict *)dict);
	utils::complexValueFromXpdfObj<pDict,CDict::Value&> (dictionary, *objDict, dictionary.value);

	// Save the contents of the container
	utils::parseStreamToContainer (buffer, o);
}


//
//
//
CStream::CStream (const CDict& dict) : parser (NULL), tmpObj (NULL)
{
	kernelPrintDbg (debug::DBG_DBG,"");

	dictionary.init (dict);
}

//
//
//
CStream::CStream (bool makeReqEntries) : parser (NULL)
{
	kernelPrintDbg (debug::DBG_DBG,"");

	if (makeReqEntries)
		createReqEntries ();
}

//
//
//
void
CStream::createReqEntries ()
{
	// Add one required entry in the stream dictionary (according to pdf specification)
	boost::shared_ptr<CInt> len (new CInt (0));
	assert (len);
	dictionary.addProperty ("Length", *len);
}

//
// Cloning
//

//
//
//
IProperty*
CStream::doClone () const
{
	kernelPrintDbg (debug::DBG_DBG,"CStream::doClone");
	assert (NULL == parser  || !"Want to clone opened stream.. Should the stream state be also copied?");
	//assert (getLength() == buffer.size());
	
	// Make new stream object
	// NOTE: We do not want to inherit any IProperty variable
	CStream* clone_ = _newInstance ();
	
	//
	// Loop through all items and clone them as well and finally add them to the new object
	//
	CDict::Value::const_iterator it = dictionary.value.begin ();
	for (; it != dictionary.value.end (); ++it)
	{
		boost::shared_ptr<IProperty> newIp = ((*it).second)->clone ();
		assert (newIp);
		CDict::Value::value_type item =  make_pair ((*it).first, newIp);
		clone_->dictionary.value.push_back (item);
	}

	copy (buffer.begin(), buffer.end(), back_inserter (clone_->buffer));
	
	return clone_;
}

//
// Set methods
//

//
//
//
void 
CStream::setPdf (boost::weak_ptr<CPdf> pdf)
{
	// Set pdf to this object and dictionary it contains
	IProperty::setPdf (pdf);
	dictionary.setPdf (pdf);
}

//
//
//
void 
CStream::setIndiRef (const IndiRef& rf)
{
	// Set pdf to this object and dictionary it contains
	IProperty::setIndiRef (rf);
	dictionary.setIndiRef (rf);
}


//
//
//
void 
CStream::setRawBuffer (const Buffer& buf)
{
	assert (NULL == parser || !"Stream is open.");
	if (NULL != parser)
		throw CObjInvalidOperation ();
	
	// Check whether we can make the change
	this->canChange();

	// Create context
	boost::shared_ptr<ObserverContext> context (this->_createContext());

	// Copy buf to buffer
	buffer.clear ();
	copy (buf.begin(), buf.end(), back_inserter (buffer));
	// Change length
	setLength (buffer.size());
	
	try {
		//Dispatch change 
		_objectChanged (context);
		
	}catch (PdfException&)
	{
		assert (!"Should not happen.. Condition must be included in CPdf::canChange()...");
		throw;
	}
}

//
//
//
void
CStream::setLength (size_t len) 
{
	
	//Lock
	dictionary.lockChange ();

	CInt _len (static_cast<const int>(len));
	dictionary.setProperty ("Length", _len);

	// Unlock
	dictionary.unlockChange ();

}


//
// Helper methods
//

//
// 1) Dictionary of a BaseStream is always freed in ~BaseStream
//
//
// FIXME update - is this still true?
// 2) In mem stream if needBuf variable is set, buf is freed
// 	needBuf is set in doDecryption
//
::Object*
CStream::_makeXpdfObject () const
{
	kernelPrintDbg (debug::DBG_DBG, "");

	//
	// Set correct length. This can ONLY happen e.g. when length is an indirect
	// object
	// 
	if (getLength() != buffer.size())
		kernelPrintDbg (debug::DBG_WARN, "Length attribute of a stream is not valid. Changing it to buffer size.");

	// Dictionary will be deallocated in ~BaseStream
	::Object* obj = utils::xpdfStreamObjFromBuffer (buffer, dictionary);
	assert (NULL != obj);
	assert (objStream == obj->getType());
	return obj;
}


//
//
//
void
CStream::getStringRepresentation (string& str) const 
{
	kernelPrintDbg (debug::DBG_DBG, "");
	
	// Empty the string
	str.clear ();

	// Get dictionary string representation
	string strDict;
	dictionary.getStringRepresentation (str);

	// Put them together
	return utils::streamToString (strDict, buffer.begin(), buffer.end(), back_inserter(str));
}


//
//
//
void
CStream::getDecodedStringRepresentation (string& str) const 
{
	kernelPrintDbg (debug::DBG_DBG, "");
	
	// Empty the string
	str.clear ();

	//
	// Make xpdf object and use its filters to get sane characters
	// 
	::Object* obj = _makeXpdfObject ();
	assert (NULL != obj);
	
	// Get the contents
	utils::getStringFromXpdfStream (str, *obj);

	// Clean-up
	xpdf::freeXpdfObject (obj);
}

//
//
//
size_t
CStream::getLength () const 
{
	try {
		int length;
		dictionary.getProperty<CInt>("Length")->getValue (length);
		assert (0 <= length);
		return length;
	}catch(...)
	{
		assert (!"Bad Length type in stream.");
		throw CObjInvalidObject ();
	}
}

//
//
//
IProperty::ObserverContext* 
CStream::_createContext () const
{
	return new BasicObserverContext (boost::shared_ptr<IProperty> (new CNull ()));
}


//
//
//
void 
CStream::_objectChanged (boost::shared_ptr<const ObserverContext> context)
{
	// Do not notify anything if we are not in a valid pdf
	if (!hasValidPdf (this))
		return;
	assert (hasValidRef (this));

	// Set correct length
	if (getLength() != buffer.size())
	{
		kernelPrintDbg (debug::DBG_WARN, "Length attribute of a stream is not valid. Changing it to buffer size.");
		setLength (buffer.size());
	}
	
	// Dispatch the change
	this->dispatchChange ();
	
	if (context)
	{
		// Clone new value
		// TODO why to clone? This may be preformance problem. Streams are
		// rather big
		//boost::shared_ptr<IProperty> newValue (this->clone());
		
		// doesn't clone but rather wrap this with shared pointer
		// and empty deallocator to prevent from this instance
		// deallocation
		boost::shared_ptr<IProperty> newValue(this, EmptyDeallocator<IProperty>());
		// Fill it with correct values
		newValue->setPdf (this->getPdf());
		newValue->setIndiRef (this->getIndiRef());

		// Notify everybody about this change
		this->notifyObservers (newValue, context);

	}else
	{
		assert (!"Invalid context");
		throw CObjInvalidOperation ();
	}
}


//
// Parsing
//

//
//
//
void
CStream::open ()
{
	kernelPrintDbg (debug::DBG_DBG,"");

	// if parser not null or object is in use we have done something wrong
	if (NULL != parser || !curObj.isNone () || tmpObj)
	{
		assert (!"Open an opened stream.");
		curObj.free ();
		delete parser; parser = NULL;
		xpdf::freeXpdfObject (tmpObj); tmpObj = NULL;
		throw CObjInvalidOperation ();
	}
	
	::XRef* xref = NULL;
	boost::shared_ptr<CPdf> p = this->getPdf ().lock ();
	if (p)
		xref = p->getCXref();
	// Create xpdf object from current stream and parse it
	tmpObj = _makeXpdfObject ();
	parser = new ::Parser (xref, new ::Lexer(xref, tmpObj), gFalse);
}

//
// 
//
void
CStream::close ()
{
	kernelPrintDbg (debug::DBG_DBG,"");

	if (parser && !curObj.isNone() && tmpObj)
	{
		curObj.free ();
		delete parser; parser = NULL;
		xpdf::freeXpdfObject (tmpObj); tmpObj = NULL;

	}else
	{
		assert (!"Close a closed stream.");
		throw CObjInvalidOperation ();
	}
}


//
//
//
bool
CStream::eof () const
{
	//kernelPrintDbg (debug::DBG_DBG,"");

	if (NULL != parser)
	{
		return (gTrue == curObj.isEOF());

	}else
	{
		assert (!"Operation on closed stream.");
		throw CObjInvalidOperation ();
	}
}

//
//
//
void
CStream::getXpdfObject (::Object& obj)
{
	assert (!eof());
	//kernelPrintDbg (debug::DBG_DBG,"");

	if (NULL != parser)
	{
		curObj.free ();
		
		if(!parser->getObj (&curObj)) {	
			kernelPrintDbg(debug::DBG_ERR, "Unable to parse content stream");
			throw MalformedFormatExeption("bad data stream");
		}
		assert (!curObj.isNone ());
		assert (!curObj.isNull ());
		assert (!curObj.isError ());
		
		curObj.copy (&obj);

	}else
	{
		assert (!"Operation on closed stream.");
		throw CObjInvalidOperation ();
	}
}

//
//
//
::Stream*
CStream::getXpdfStream ()
{
	kernelPrintDbg (debug::DBG_DBG,"");

	if (NULL != parser)
	{
		return parser->getStream ();

	}else
	{
		assert (!"Operation on closed stream.");
		throw CObjInvalidOperation ();
	}
}

//
// Destructor
//
CStream::~CStream ()
{
	kernelPrintDbg (debug::DBG_DBG,"");

	if (NULL != parser)
	{
		assert (!"Stream not closed.");
		curObj.free ();
		delete parser; parser = NULL;
		xpdf::freeXpdfObject (tmpObj); tmpObj = NULL;
		
	}else
	{
		assert (curObj.isNone() || curObj.isNull());
	}
}



//=====================================================================================
} // namespace pdfobjects
//=====================================================================================
