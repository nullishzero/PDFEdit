// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80
/*
 * =====================================================================================
 *        Filename:  cobjecstreamtI.h
 *         Created:  02/02/2006 
 *          Author:  jmisutka (06/01/19), 
 * 			
 * =====================================================================================
 */

#ifndef _COBJECTSTREAMI_H
#define _COBJECTSTREAMI_H

//=====================================================================================
namespace pdfobjects {
//=====================================================================================


//=====================================================================================
// CObjectStream
//=====================================================================================

//
// Constructors
//

//
//
//
template<typename Checker>
CObjectStream<Checker>::CObjectStream (CPdf& p, ::Object& o, const IndiRef& rf) : IProperty (&p,rf), parser (NULL)
{
	Checker check; check.objectCreated (this);
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
template<typename Checker>
CObjectStream<Checker>::CObjectStream (::Object& o) : parser (NULL)
{
	Checker check; check.objectCreated (this);
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
template<typename Checker>
CObjectStream<Checker>::CObjectStream (bool makeReqEntries) : parser (NULL)
{
	Checker check; check.objectCreated (this);
	kernelPrintDbg (debug::DBG_DBG,"");

	if (makeReqEntries)
		createReqEntries ();
}

//
//
//
template<typename Checker>
void
CObjectStream<Checker>::createReqEntries ()
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
template<typename Checker>
IProperty*
CObjectStream<Checker>::doClone () const
{
	kernelPrintDbg (debug::DBG_DBG,"CObjectStream::doClone");
	assert (NULL == parser  || !"Want to clone opened stream.. Should the stream state be also copied?");
	//assert (getLength() == buffer.size());
	
	// Make new stream object
	// NOTE: We do not want to inherit any IProperty variable
	CObjectStream<Checker>* clone_ = _newInstance ();
	
	//
	// Loop through all items and clone them as well and finally add them to the new object
	//
	CDict::Value::const_iterator it = dictionary.value.begin ();
	for (; it != dictionary.value.end (); ++it)
	{
		boost::shared_ptr<IProperty> newIp = ((*it).second)->clone ();
		assert (newIp);
		CDict::Value::value_type item =  std::make_pair ((*it).first, newIp);
		clone_->dictionary.value.push_back (item);
	}

	std::copy (buffer.begin(), buffer.end(), std::back_inserter (clone_->buffer));
	
	return clone_;
}

//
// Set methods
//

//
//
//
template<typename Checker>
void 
CObjectStream<Checker>::setPdf (CPdf* pdf)
{
	// Set pdf to this object and dictionary it contains
	IProperty::setPdf (pdf);
	dictionary.setPdf (pdf);
}

//
//
//
template<typename Checker>
void 
CObjectStream<Checker>::setIndiRef (const IndiRef& rf)
{
	// Set pdf to this object and dictionary it contains
	IProperty::setIndiRef (rf);
	dictionary.setIndiRef (rf);
}


//
//
//
template<typename Checker>
void 
CObjectStream<Checker>::setRawBuffer (const Buffer& buf)
{
	assert (NULL == parser || !"Stream is open.");
	if (NULL != parser)
		throw CObjInvalidOperation ();
	
	// Create context
	boost::shared_ptr<ObserverContext> context (this->_createContext());

	// Copy buf to buffer
	buffer.clear ();
	std::copy (buf.begin(), buf.end(), std::back_inserter (buffer));
	// Change length
	setLength (buffer.size());
	
	//Dispatch change 
	_objectChanged (context);
}

//
//
//
template<typename Checker>
template<typename Container>
void 
CObjectStream<Checker>::setBuffer (const Container& buf)
{
	kernelPrintDbg (debug::DBG_DBG, "");
	assert (NULL == parser || !"Stream is open.");
	if (NULL != parser)
		throw CObjInvalidOperation ();

	// Create context
	boost::shared_ptr<ObserverContext> context (this->_createContext());
	
	// Make buffer pdf valid, encode buf and save it to buffer
	std::string strbuf;
	utils::makeStreamPdfValid (buf.begin(), buf.end(), strbuf);
	encodeBuffer (strbuf);
	// Change length
	setLength (buffer.size());
	
	//Dispatch change 
	_objectChanged (context);
}


//
//
//
template<typename Checker>
void
CObjectStream<Checker>::setLength (size_t len) 
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
template<typename Checker>
::Object*
CObjectStream<Checker>::_makeXpdfObject () const
{
	kernelPrintDbg (debug::DBG_DBG, "");

	//
	// Set correct length. This can ONLY happen e.g. when length is an indirect
	// object
	// 
	if (getLength() != buffer.size())
		kernelPrintDbg (debug::DBG_CRIT, "Length attribute of a stream is not valid. Changing it to buffer size.");

	// Dictionary will be deallocated in ~BaseStream
	::Object* obj = utils::xpdfStreamObjFromBuffer (buffer, dictionary._makeXpdfObject());
	assert (NULL != obj);
	assert (objStream == obj->getType());
	return obj;
}


//
//
//
template<typename Checker>
template<typename Container>
void 
CObjectStream<Checker>::encodeBuffer (const Container& buf)
{
	kernelPrintDbg (debug::DBG_DBG, "");

	//
	// Create input filtes and add filters according to Filter item in
	// stream dictionary
	// 
	filters::InputStream in;
	std::vector<std::string> filters;
	getFilters (filters);
	
	// Try adding filters if one is not supported use none
	try {
		
		filters::CFilterFactory::addFilters (in, filters);
	
	}catch(FilterNotSupported&)
	{
		kernelPrintDbg (debug::DBG_DBG, "One of the filters is not supported, using none..");
		
		dictionary.delProperty ("Filter");
		// Clear buffer
		buffer.clear ();
		std::copy (buf.begin(), buf.end(), std::back_inserter (buffer));
		return;
	}
	
	// Clear buffer
	buffer.clear ();

	// Create input source from buffer
	boost::iostreams::stream<filters::buffer_source<Container> > input (buf);
	in.push (input);
	// Copy it to container
	Buffer::value_type c;
	in.get (c);
	while (!in.eof())
	{//\TODO Improve this !!!!
		buffer.push_back (c);
		in.get (c);
	}
	// Close the stream
	in.reset ();
}

//
//
//
template<typename Checker>
void
CObjectStream<Checker>::getStringRepresentation (std::string& str) const 
{
	kernelPrintDbg (debug::DBG_DBG, "");
	
	// Empty the string
	str.clear ();

	// Get dictionary string representation
	std::string strDict;
	dictionary.getStringRepresentation (str);

	// Put them together
	return utils::streamToString (strDict, buffer.begin(), buffer.end(), std::back_inserter(str));
}


//
//
//
template<typename Checker>
void
CObjectStream<Checker>::getDecodedStringRepresentation (std::string& str) const 
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
template<typename Checker>
size_t
CObjectStream<Checker>::getLength () const 
{
	boost::shared_ptr<IProperty> len = utils::getReferencedObject (dictionary.getProperty("Length"));
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
template<typename Checker>
IProperty::ObserverContext* 
CObjectStream<Checker>::_createContext () const
{
	return new BasicObserverContext (boost::shared_ptr<IProperty> (new CNull ()));
}

//
//
//
template<typename Checker>
template<typename Container>
void
CObjectStream<Checker>::getFilters (Container& container) const
{
	boost::shared_ptr<IProperty> ip;
	//
	// Get optional value Filter
	//
	try	
	{
		ip = dictionary.getProperty ("Filter");
		
	}catch (ElementNotFoundException&)
	{
		// No filter found
		kernelPrintDbg (debug::DBG_DBG, "No filter found.");
		return;
	}

	//
	// If it is a name just store it
	// 
	if (isName (ip))
	{
		boost::shared_ptr<const CName> name = IProperty::getSmartCObjectPtr<CName>(ip);
		container.push_back (name->getValue());
		
		kernelPrintDbg (debug::DBG_DBG, "Filter name:" << name->getValue());
	//
	// If it is an array, iterate through its properties
	//
	}else if (isArray (ip))
	{
		boost::shared_ptr<CArray> array = IProperty::getSmartCObjectPtr<CArray>(ip);
		// Loop throug all children
		CArray::Value::iterator it = array->value.begin ();
		for (; it != array->value.end(); ++it)
		{
			boost::shared_ptr<CName> name = IProperty::getSmartCObjectPtr<CName>(*it);
			container.push_back (name->getValue());
			
			kernelPrintDbg (debug::DBG_DBG, "Filter name:" << name->getValue());

		} // for (; it != array->value.end(); ++it)
	}
}

//
//
//
template<typename Checker>
template<typename Storage>
void 
CObjectStream<Checker>::_getAllChildObjects (Storage& store) const
{
	typename CDict::Value::const_iterator it = dictionary.value.begin ();
	for	(; it != dictionary.value.end (); ++it)
	{
		boost::shared_ptr<IProperty> ip = utils::getIPropertyFromItem (*it);
		store.push_back (ip);
	}
}

//
//
//
template<typename Checker>
void 
CObjectStream<Checker>::_objectChanged (boost::shared_ptr<const ObserverContext> context)
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
		boost::shared_ptr<IProperty> newValue (this->clone());
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
template<typename Checker>
void
CObjectStream<Checker>::open ()
{
	kernelPrintDbg (debug::DBG_DBG,"");

	if (NULL != parser || !curObj.isNone ())
	{
		assert (!"Open an opened stream.");
		delete parser;
		parser = NULL;
		throw CObjInvalidOperation ();
	}
	
	::XRef* xref = (NULL != this->getPdf ()) ? this->getPdf ()->getCXref() : NULL;
	// Create xpdf object from current stream and parse it
	parser = new ::Parser (xref, new ::Lexer(xref, _makeXpdfObject()));
}

//
// 
//
template<typename Checker>
void
CObjectStream<Checker>::close ()
{
	kernelPrintDbg (debug::DBG_DBG,"");

	if (NULL != parser && !curObj.isNone())
	{
		curObj.free ();
		delete parser;
		parser = NULL;		

	}else
	{
		assert (!"Close a closed stream.");
		throw CObjInvalidOperation ();
	}
}


//
//
//
template<typename Checker>
bool
CObjectStream<Checker>::eof () const
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
template<typename Checker>
void
CObjectStream<Checker>::getXpdfObject (::Object& obj)
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
template<typename Checker>
::Stream*
CObjectStream<Checker>::getXpdfStream ()
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
template<typename Checker>
CObjectStream<Checker>::~CObjectStream ()
{
	Checker check; check.objectDeleted (this);
	kernelPrintDbg (debug::DBG_DBG,"");

	if (NULL != parser)
	{
		assert (!"Stream not closed.");
		curObj.free ();
		delete parser;
		parser = NULL;
	}
}



//=====================================================================================
} // namespace pdfobjects
//=====================================================================================



#endif // _COBJECTSTREAMI_H
