// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80
/*
 * =====================================================================================
 *        Filename:  cstream.cc
 *         Created:  02/02/2006 
 *          Author:  jmisutka (06/01/19), 
 * =====================================================================================
 */

#include "static.h"
#include "cstream.h"
#include "cpdf.h"

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
CStream::CStream (CPdf& p, ::Object& o, const IndiRef& rf) : IProperty (&p,rf), parser (NULL), tmpObj (NULL)
{
	kernelPrintDbg (debug::DBG_DBG,"");
	// Make sure it is a stream
	assert (objStream == o.getType());
	if (objStream != o.getType())
		throw XpdfInvalidObject ();
	
	// Get the dictionary and init CDict with it
	::Object objDict;
	::Dict* dict = o.streamGetDict ();
	assert (NULL != dict);
	objDict.initDict (dict);
	utils::complexValueFromXpdfObj<pDict,CDict::Value&> (dictionary, objDict, dictionary.value);
	objDict.free ();

	// Set pdf and ref
	dictionary.setPdf (&p);
	dictionary.setIndiRef (rf);
	
	// Save the contents of the container
	utils::parseStreamToContainer (buffer, o);
}


//
//
//
CStream::CStream (::Object& o) : parser (NULL), tmpObj (NULL)
{
	kernelPrintDbg (debug::DBG_DBG,"");
	// Make sure it is a stream
	assert (objStream == o.getType());
	if (objStream != o.getType())
		throw XpdfInvalidObject ();
	
	// Get the dictionary and init CDict with it
	::Object objDict;
	::Dict* dict = o.streamGetDict();
	assert (NULL != dict);
	objDict.initDict (dict);
	utils::complexValueFromXpdfObj<pDict,CDict::Value&> (dictionary, objDict, dictionary.value);
	objDict.free ();

	// Save the contents of the container
	utils::parseStreamToContainer (buffer, o);
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
	shared_ptr<CInt> len (new CInt (0));
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
		shared_ptr<IProperty> newIp = ((*it).second)->clone ();
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
CStream::setPdf (CPdf* pdf)
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
	
	// Create context
	shared_ptr<ObserverContext> context (this->_createContext());

	// Copy buf to buffer
	buffer.clear ();
	copy (buf.begin(), buf.end(), back_inserter (buffer));
	// Change length
	setLength (buffer.size());
	
	//Dispatch change 
	_objectChanged (context);
}

//
//
//
void
CStream::setLength (size_t len) 
{
	
	//Lock
	dictionary.lockChange ();

	CInt _len (len);
	dictionary.setProperty ("Length", _len);

	// Unlock
	dictionary.unlockChange ();

}


//
// Helper methods
//

//
// 1) Dictionary of a BaseStream is always freed in ~BaseStream
// 2) In mem stream if needBuf variable is set, buf is freed
// 	needBuf is set in doDecryption
//
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
		kernelPrintDbg (debug::DBG_CRIT, "Length attribute of a stream is not valid. Changing it to buffer size.");

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
	utils::freeXpdfObject (obj);
}

//
//
//
size_t
CStream::getLength () const 
{
	shared_ptr<IProperty> len = utils::getReferencedObject (dictionary.getProperty("Length"));
	if (isInt (len))
	{
		int length;
		IProperty::getSmartCObjectPtr<CInt>(len)->getValue (length);
		assert (0 <= length);
		return length;
	
	}else
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
	return new BasicObserverContext (shared_ptr<IProperty> (new CNull ()));
}


//
//
//
void 
CStream::_objectChanged (shared_ptr<const ObserverContext> context)
{
	// Do not notify anything if we are not in a valid pdf
	if (!hasValidPdf (this))
		return;
	assert (hasValidRef (this));

	// Set correct length
	if (getLength() != buffer.size())
	{
		kernelPrintDbg (debug::DBG_CRIT, "Length attribute of a stream is not valid. Changing it to buffer size.");
		setLength (buffer.size());
	}
	
	// Dispatch the change
	this->dispatchChange ();
	
	if (context)
	{
		// Clone new value
		shared_ptr<IProperty> newValue (this->clone());
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
		freeXpdfObject (tmpObj); tmpObj = NULL;
		throw CObjInvalidOperation ();
	}
	
	::XRef* xref = (NULL != this->getPdf ()) ? this->getPdf ()->getCXref() : NULL;
	// Create xpdf object from current stream and parse it
	tmpObj = _makeXpdfObject ();
	parser = new ::Parser (xref, new ::Lexer(xref, tmpObj));
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
		freeXpdfObject (tmpObj); tmpObj = NULL;

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
		return (curObj.isEOF());

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
		
		parser->getObj (&curObj);
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
		freeXpdfObject (tmpObj); tmpObj = NULL;
		
	}else
	{
		assert (curObj.isNone() || curObj.isNull());
	}
}



//=====================================================================================
} // namespace pdfobjects
//=====================================================================================
