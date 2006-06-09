// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80
/* 
 * =====================================================================================
 *        Filename:  cobject.h
 *     Description:  Header file containing definition of IProperty and CObject classes.
 *         Created:  01/18/2006 
 *          Author:  jmisutka (06/01/19), 
 *
 * =====================================================================================
 */
#ifndef _COBJECTSTREAM_H
#define _COBJECTSTREAM_H

// all basic includes
#include "static.h"
#include "iproperty.h"
// Filters
#include "filters.h"


//=====================================================================================
namespace pdfobjects {
//=====================================================================================

		
//
// Forward declarations of memory checkers
//
class NoMemChecker;
class BasicMemChecker;
class CXRef;


//=====================================================================================
// CObjectStream
//=====================================================================================

//
// Forward declaration
//
template<typename T> class CStreamXpdfReader;

/**
 * Template class representing stream PDF objects from specification v1.5.
 *
 * It is neither a simple object, because it does not
 * contain just simple value, nor a complex object, because it can not be simple represented
 * in that generic class. It contains a dictionary and a stream. It does not have methods common
 * to complex objects.
 *
 * This is a generic class joining implementation of dictionary and array together in in one place.
 *
 * Other xpdf objects like objCmd can not be instantiated although the PropertyType 
 * exists. It is because PropertyTraitSimple is not specialized for these types.
 *
 * We use memory checking with this class which save information about existing IProperties.
 * This technique can be used to detect memory leaks etc. 
 *
 * Xpdf stream objects are the worst from all xpdf objects because of their deallocation politics.
 * It is really not easy to say when, where and who should deallocate an xpdf stream, its buffer etc...
 * 
 * This class does not provide default copy constructor because copying a
 * property could be understood either as deep copy or shallow copy. 
 * Copying complex types could be very expensive so we have made this decision to
 * avoid it.
 */
template <typename Checker = debug::BasicMemChecker>
class CObjectStream : noncopyable, public IProperty
{
	typedef CObjectSimple<pName> 	CName;
	typedef CObjectSimple<pInt> 	CInt;
	typedef CObjectComplex<pDict> 	CDict;
	typedef CObjectComplex<pArray> 	CArray;
	typedef boost::iostreams::filtering_streambuf<boost::iostreams::output> filtering_ostreambuf;
	typedef std::string PropertyId;

public:
	// We can access xpdf stream only through CStreamXpdfReader 
	template<typename T> friend class CStreamXpdfReader;

	typedef std::vector<filters::StreamChar> Buffer;

protected:

	/** Object dictionary. */
	CDict dictionary;
	
	/** Buffer. */
	Buffer buffer;

	/** XpdfParser. */
	::Parser* parser;
	/** Current object in an opened stream. */
	mutable ::Object curObj;
		

	//
	// Constructors
	//
public:
	/**
	 * Constructor. Only kernel can call this constructor
	 *
	 * @param p		Pointer to pdf object.
	 * @param o		Xpdf object. 
	 * @param rf	Indirect id and gen id.
	 */
	CObjectStream (CPdf& p, Object& o, const IndiRef& rf);

	/**
	 * Constructor. Only kernel can call this constructor
	 *
	 * @param o		Xpdf object. 
	 */
	CObjectStream (Object& o);


public:	
	
	/** 
	 * Public constructor. This object will not be associated with a pdf.
	 * It adds one required property to objects dictionary namely "Length". This
	 * is according to the pdf specification.
	 *
	 * @param makeReqEntries If true required entries are added to stream
	 * dictionary.
	 */
	CObjectStream (bool makeReqEntries = true);

	
	//
	// Cloning
	//
protected:

	/**
     * Implementation of clone method. 
	 *
     * @param Deep copy of this object.
	 */
	virtual IProperty* doClone () const;

	/** Return new instance. */
	virtual CObjectStream<Checker>* _newInstance () const
		{ return new CObjectStream<Checker> (false); }

	/** Create required entries. */
	void createReqEntries ();

	//
	// Dictionary methods, delegated to CDict
	//
public:
	/** Delagate this operation to underlying dictionary. \see CObjectComplex */
	size_t getPropertyCount () const
		{return dictionary.getPropertyCount ();}
	
	/** Delagate this operation to underlying dictionary. \see CObjectComplex */
	template<typename Container> 
	void getAllPropertyNames (Container& container) const
		{ dictionary.getAllPropertyNames (container); }
	
	/** Delagate this operation to underlying dictionary. \see CObjectComplex */
	boost::shared_ptr<IProperty> getProperty (PropertyId id) const
		{return dictionary.getProperty (id);}
	
	/** Delagate this operation to underlying dictionary. \see CObjectComplex */
	PropertyType getPropertyType (PropertyId id) const
		{return dictionary.getPropertyType (id);}
	
	/** Delagate this operation to underlying dictionary. \see CObjectComplex */
	boost::shared_ptr<IProperty> setProperty (PropertyId id, IProperty& ip)
		{return dictionary.setProperty (id, ip);}
	
	/** Delagate this operation to underlying dictionary. \see CObjectComplex */
	boost::shared_ptr<IProperty> addProperty (PropertyId id, const IProperty& newIp)
		{return dictionary.addProperty (id, newIp);}
	
	/** Delagate this operation to underlying dictionary. \see CObjectComplex */
	void delProperty (PropertyId id)
		{dictionary.delProperty (id);}


	//
	// Get methods
	//
public:	
	
	/** 
     * Returns type of object. 
     *
     * @return Type of this class.
     */
    virtual PropertyType getType () const {return pStream;}

	/**
	 * Returns string representation of this object.
	 *
	 * @param str Output string representation.
	 */
	virtual void getStringRepresentation (std::string& str) const;

	/**
	 * Returns decoded string representation of this object.
	 *
	 * @param str Output string representation.
	 */
	virtual void getDecodedStringRepresentation (std::string& str) const;

	/**
	 * Get encoded buffer. Can contain non printable characters.
	 *
	 * @return Buffer.
	 */
	const Buffer& getBuffer () const {return buffer;}
	
	/**
	 * Get filters.
	 *
	 * @param container Container of filter names.
	 */
	template<typename Container>
	void getFilters (Container& container) const;


	//
	// Set methods
	//
public:
	/**
	 * Set pdf to itself and also tu all children
	 *
	 * @param pdf New pdf.
	 */
	virtual void setPdf (CPdf* pdf);

	/**
	 * Set ref to itself and also tu all children
	 *
	 * @param pdf New indirect reference numbers.
	 */
	virtual void setIndiRef (const IndiRef& rf);

	/**
	 * Set encoded buffer.
	 *
	 * @param buf New buffer.
	 */
	void setRawBuffer (const Buffer& buf);

	/**
	 * Set decoded buffer. 
	 * Use avaliable filters. If a filter is not avaliable an exception is thrown.
	 *
	 * @param buf New buffer.
	 */
	template<typename Container>
	void setBuffer (const Container& buf);

	//
	// Parsing (use friend CStreamXpdfReader)
	//
private:
	
	/**
	 * Initialize parsing mechanism.
	 *
	 * REMARK: if CObject is not in a pdf, we MUST be sure that it does not
	 * use indirect objects.
	 */
	void open ();

	/**
	 * Close parser.
	 */
	void close ();
	
	/**
	 * Get xpdf object and copy it to obj.
	 *
	 * REMARK: We can not do any buffering (caching) of xpdf objects, because
	 * xpdf already does caching and it will NOT work correctly with inline
	 * images. We would buffer WRONG data.
	 *
	 * @param obj Next xpdf object.
	 */
	void getXpdfObject (::Object& obj);
	
	/**
	 * Get xpdf stream. Be carefull this is not a copy.
	 * 
	 * @return Stream.
	 */
	 ::Stream* getXpdfStream ();

	/**
	 * Is the last object end of stream.
	 *
	 * This is not very common behaviour, but we can not use caching 
	 * \see getXpdfObject
	 * so we can tell if it is the end after fetching an object which means
	 * after calling getXpdfObject.
	 * 
	 * @return True if we no more data avaliable, false otherwise.
	 */
	bool eof () const;
	
	
	//
	// Destructor
	//
public:

	/**
	 * Destructor.
	 */
	~CObjectStream ();



	//
	// Helper methods
	//
public:
	/**
	 * Make xpdf Object from this object. This function allocates and initializes xpdf object.
	 * Caller has to free the xpdf Object (call Object::free and then
	 * deallocating)
	 *
	 * \exception ObjBadValueE Thrown when xpdf can't parse the string representation of this
	 * object correctly.
	 * 
	 * @return Xpdf object representing value of this simple object.
	 */
	virtual Object* _makeXpdfObject () const; 


private:
	/**
	 * Indicate that the object has changed.
	 * Notifies all observers associated with this property about the change.
	 *
	 * @param context Context in which a change occured.
	 */
	void _objectChanged (boost::shared_ptr<const ObserverContext> context);

	/**
	 * Get encoded buffer.
	 *
	 * @param container Output container.
	 */
	template<typename Container>
	void encodeBuffer (const Container& container);

private:
	/**
	 * Get length.
	 *
	 * @return Stream length.
	 */
	size_t getLength () const;

	/**
	 * Set length.
	 *
	 * @param len Stream Length.
	 */
	void setLength (size_t len);

private:
	/**
	 * Create context of a change.
	 *
	 * REMARK: Be carefull. Deallocate this object.
	 * 
	 * @return Context in which a change occured.
	 */
	ObserverContext* _createContext () const;

public:
	/**
	 * Return all object we have access to.
	 *
	 * @param store Container of objects.
	 */
	template <typename Storage>
	void _getAllChildObjects (Storage& store) const;

	
	//
	// Special functions
	//
public:

	/**
	 * Return the list of all supported streams
	 *
	 * @return List of supported stream filters.
	 */
	template<typename Container>
	static void getSupportedStreams (Container& supported) 
		{ filters::CFilterFactory::getSupportedStreams (supported); }

}; /* CObjectStream */


//
// Typedefs
//
typedef CObjectStream<>	CStream;

/**
 * Adapter which is able to read sequentially from more CStreams.
 *
 * It stores a container of streams and when the actual stream does not
 * contain more objects, tries to read the next one if any.
 *
 * This class is an example of Adapter design pattern. We have to be able to
 * read from more streams sequentially and this class provides us with the
 * interface.
 */
template<typename Container>
class CStreamXpdfReader
{
public:
	typedef std::vector<boost::shared_ptr<CStream> > CStreams;

private:
	CStreams streams;	/**< All streams. */
	boost::shared_ptr<CStream> actstream;	/**< Actual stream that is beeing parsed. */
	size_t pos;			/**< Position of actual parsed stream in the stream container. */
	size_t objread; 	/**< Helper variable for debugging. Number of read objects. */
	

public:

	/** Constructor. */
	CStreamXpdfReader (Container& strs) : pos(0), objread (0)
		{ assert (!strs.empty()); std::copy (strs.begin(), strs.end(), std::back_inserter(streams));}
	CStreamXpdfReader (boost::shared_ptr<CStream> str) : pos(0), objread (0)
		{ assert (str); streams.push_back (str); }

	/** Open. */
	void open ()
	{
		assert (!streams.empty());
		if (actstream && 0 != pos)
			{ assert (!"Stream opened before."); }

		actstream = streams.front ();
		actstream->open ();
	}

	/** Close. */
	void close ()
	{
		assert (!streams.empty());
		assert (streams.size() == pos + 1);
		assert (actstream == streams.back());
		assert (actstream->eof());
		
		actstream->close ();
	}

	/** 
	 * Close. 
	 * Save parsed streams to container.
	 *
	 * @param parsedstreams Output buffer that will contain all streams we have
	 * really parsed.
	 */
	template<typename Ctr>
	void close (Ctr& parsedstreams)
	{
		assert (!streams.empty());
		assert (actstream->eof());
		
		for (size_t i = 0; i <= pos; ++i)
			parsedstreams.push_back (streams[i]);
		
		actstream->close ();
	}

	/** Get xpdf object. */
	void getXpdfObject (::Object& obj)
	{
		assert (!actstream->eof());
		
		// Get an object
		actstream->getXpdfObject (obj);

		// If we are at the end of this stream but another stream is not empty 
		// get the object
		if (actstream->eof() && !eof())
		{
			assert (obj.isEOF());
			actstream->getXpdfObject (obj);
		}
		
		/** debugging \TODO remove. */
		objread ++;
	}

	/** 
	 * Is end of stream. 
	 *
	 * We can not cache and due to this fact we can not tell if a stream
	 * is empty without fetching an object. 
	 */
	bool eof ()
	{ 
		if (eofOfActualStream())
		{
			// Do we have another stream
			while (actstream != streams.back())
			{
				assert (pos < streams.size());
				actstream->close();
				// Take next stream
				++pos;
				actstream = streams[pos];
				actstream->open ();
				// Fetch an object and look at it
				xpdf::XpdfObject obj;
				actstream->getXpdfObject (*obj);
				if (!actstream->eof())
				{
					actstream->close();
					actstream->open();
					break;
				}
			}
			return  (actstream == streams.back() && eofOfActualStream()); 
		
		}else
		{
			return false;
		}
	}

	/** End of actual stream. */
	bool eofOfActualStream ()
		{ return (actstream->eof()); }

	/** Get xpdf stream. */
	::Stream* getXpdfStream ()
		{ return actstream->getXpdfStream(); }

	
};

//=====================================================================================
namespace utils {
//=====================================================================================

/**
 * Make stream pdf valid.
 *
 * Not needed now.
 * 
 * @param it Start insert iterator.
 * @param end End iterator.
 */
template<typename Iter>
void
makeStreamPdfValid (Iter it, Iter end, std::string& out)
{
	for (; it != end; ++it)
	{
		//if ( '\\' == (*it))
		//{ // "Escape" every occurence of '\'
		//		out += '\\';
		//}

		out += *it;
	}
}



/**
 * Parse stream object to a container
 *
 * @param container Container of characters (e.g. ints).
 * @param obj Stream object.
 */
template<typename T>
void parseStreamToContainer (T& container, ::Object& obj);


	
/**
 * Create xpdf object from string.
 *
 * @param buffer Stream buffer.
 * @param dict Stream dictionary.
 * @param xref Xref of actual pdf if any.
 *
 * @return Xpdf object.
 */
::Object* xpdfStreamObjFromBuffer (const CStream::Buffer& buffer, ::Object* dict);


/**
 * CStream object to string
 *
 * @param strDict Dictionary string representation.
 * @param begin Buffer begin
 * @param end Buffer end
 * @param out Output string.
 */
template<typename ITERATOR, typename OUTITERATOR>
void streamToString (const std::string& strDict, ITERATOR begin, ITERATOR end, OUTITERATOR out);

/**
 * Makes a valid pdf representation of a stream using streamToString function.
 * 
 * @param strDict Dictionary string representation.
 * @param streambuf Raw stream buffer.
 * @param outbuf Output buffer.
 *
 * @param Length of data.
 */
size_t streamToCharBuffer (const std::string& strDict, const CStream::Buffer& streambuf, CharBuffer& outbuf);

//=========================================================
//	CDict "get type" helper methods
//=========================================================


/** 
 * Get stream from dictionary. If it is CRef fetch the object pointed at.
 */
template<typename IP>
inline boost::shared_ptr<CStream>
getCStreamFromDict (IP& ip, const std::string& key)
	{return getTypeFromDictionary<CStream,pStream> (ip, key);}

//=========================================================
//	CArray "get type" helper methods
//=========================================================

/** 
 * Get stream from array. If it is CRef fetch the object pointed at.
 */
template<typename IP>
inline boost::shared_ptr<CStream>
getCStreamFromArray (IP& ip, size_t pos)
	{return getTypeFromArray<CStream,pStream> (ip, pos);}




//=====================================================================================
} /* namespace utils*/
//=====================================================================================
} /* namespace pdfobjects */
//=====================================================================================




//
// Include the actual implementation of CObject class
//
#include "cobjectstreamI.h"


#endif // _COBJECTSTREAM_H

