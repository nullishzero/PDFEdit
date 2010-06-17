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
#ifndef _CSTREAM_H
#define _CSTREAM_H

// all basic includes
#include "kernel/static.h"
#include "kernel/cdict.h"


//=====================================================================================
namespace pdfobjects {
//=====================================================================================


//=====================================================================================
// CStream
//=====================================================================================

//
// Forward declaration
//
template<typename T> class CStreamsXpdfReader;
namespace utils { template<typename Iter> void makeStreamPdfValid (Iter it, Iter end, std::string& out); }

/**
 * Class representing stream object from pdf specification v1.5.
 *
 * We do not want to use xpdf stream because it is a real mess and it is really
 * not suitable for editing. We use xpdf object just for initializing and for
 * reading objects.
 * 
 * According to pdf specification, stream is a dictionary and a stream of characters of specified length.
 * Dictionary is used to describe the stream (its length, filters, parameters, etc..) 
 * 
 * This class does not provide default copy constructor because copying a
 * property could be understood either as deep copy or shallow copy. 
 * Copying complex types could be very expensive so we have made the decision to
 * avoid it.
 */
class CStream : noncopyable, public IProperty
{
	typedef std::string PropertyId;

public:
	// We should be able to access xpdf stream by CStreamsXpdfReader 
	template<typename T> friend class CStreamsXpdfReader;

	typedef char StreamChar;
	typedef std::vector<StreamChar> 		Buffer;
	typedef observer::BasicChangeContext<IProperty> BasicObserverContext;

	/** 
	 * Type of this property object.
	 * This association allows us to get the PropertyType from object type.
	 */
	static const PropertyType type = pStream;

	//
	// Storage
	//
protected:
	/** Stream dictionary. */
	CDict dictionary;
	/** Stream buffer. */
	Buffer buffer;

	//
	// Parsing
	//
protected:
	/** Xpdf parser. */
	::Parser* parser;
	/** Current object in an opened stream. */
	mutable ::Object curObj;
private:
	/** Helper object, because xpdf stream does NOT automatically deallocated specified object. */
	::Object* tmpObj;
		

	//
	// Constructors
	//
public:
	/**
	 * Constructor. Stream will be associated with specified pdf.
	 *
	 * @param p		Pointer to pdf object.
	 * @param o		Xpdf object. 
	 * @param rf	Indirect id and gen id.
	 */
	CStream (boost::weak_ptr<CPdf> p, const Object& o, const IndiRef& rf);

	/**
	 * Constructor. Stream will not be associated with a pdf.
	 * @param o	Xpdf object. 
	 */
	CStream (const Object& o);

	/**
	 * Constructor. Stream will not be associated with a pdf.
	 * @param dict	reference to a dictionary which will be associated with this stream
	 * @param buffer reference to buffer associtated with this stream
	 */
	CStream (const CDict& dict);


public:	
	
	/** 
	 * Public constructor. This object will not be associated with a pdf.
	 * It adds all required properties to object dictionary in compliance with the pdf specification
	 * if needed.
	 *
	 * @param makeReqEntries If true required entries are added to the stream
	 * dictionary.
	 */
	CStream (bool makeReqEntries = true);

	
	//
	// Cloning
	//
protected:

	/**
     * Implementation of clone method. 
     * @return Deep copy of this object.
	 */
	virtual IProperty* doClone () const;

	/** Return new instance. */
	virtual CStream* _newInstance () const
		{ return new CStream (false); }

	/** Create required entries. */
	void createReqEntries ();

	//
	// Dictionary methods, delegated to CDict
	//
public:
	/** Delagate this operation to underlying dictionary. \see CDict */
	size_t getPropertyCount () const
		{return dictionary.getPropertyCount ();}
	
	/** Delagate this operation to underlying dictionary. \see CDict */
	template<typename Container> 
	void getAllPropertyNames (Container& container) const
		{ dictionary.getAllPropertyNames (container); }
	
	/** Delagate this operation to underlying dictionary. \see CDict */
	boost::shared_ptr<IProperty> getProperty (PropertyId id) const
		{return dictionary.getProperty (id);}

	/** Delagate this operation to underlying dictionary. \see CDict */
	template<typename ItemType>
	boost::shared_ptr<ItemType> getProperty (PropertyId id) const
		{return dictionary.getProperty<ItemType> (id);}
	
	/** Delagate this operation to underlying dictionary. \see CDict */
	bool containsProperty (PropertyId id) const
		{return dictionary.containsProperty (id);}
	
	/** Delagate this operation to underlying dictionary. \see CDict */
	PropertyType getPropertyType (PropertyId id) const
		{return dictionary.getPropertyType (id);}
	
	/** Delagate this operation to underlying dictionary. \see CDict */
	boost::shared_ptr<IProperty> setProperty (PropertyId id, IProperty& ip)
		{return dictionary.setProperty (id, ip);}
	
	/** Delagate this operation to underlying dictionary. \see CDict */
	boost::shared_ptr<IProperty> addProperty (PropertyId id, const IProperty& newIp)
		{return dictionary.addProperty (id, newIp);}
	
	/** Delagate this operation to underlying dictionary. \see CDict */
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
	void getFilters (Container& container) const
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
			typedef std::vector<boost::shared_ptr<IProperty> > Names;
			Names names;
			array->_getAllChildObjects (names);
			
			Names::iterator it = names.begin ();
			for (; it != names.end(); ++it)
			{
				boost::shared_ptr<CName> name = IProperty::getSmartCObjectPtr<CName>(*it);
				container.push_back (name->getValue());
				kernelPrintDbg (debug::DBG_DBG, "Filter name:" << name->getValue());
			}
		}
	}



	//
	// Set methods
	//
public:
	/**
	 * Set pdf to itself and also to all children.
	 * @param pdf New pdf.
	 */
	virtual void setPdf (boost::weak_ptr<CPdf> pdf);

	/**
	 * Set ref to itself and also to all children.
	 * @param rf New indirect reference numbers.
	 */
	virtual void setIndiRef (const IndiRef& rf);

	/**
	 * Set encoded buffer.
	 * @param buf New buffer.
	 */
	void setRawBuffer (const Buffer& buf);

	/**
	 * Set decoded (raw) buffer. 
	 * Drops all filters if present.
	 *
	 * @param buf New buffer (can be string or Buffer types).
	 */
	template<typename Container>
	void setBuffer (const Container& buf)
	{
		kernelPrintDbg (debug::DBG_DBG, "");
		assert (NULL == parser || !"Stream is open.");
		if (NULL != parser)
			throw CObjInvalidOperation ();

		// Check whether we can make the change
		this->canChange();
	
		// Create context
		boost::shared_ptr<ObserverContext> context (this->_createContext());
	
		// Make buffer pdf valid, encode buf and save it to buffer
		std::string strbuf;
		utils::makeStreamPdfValid (buf.begin(), buf.end(), strbuf);
		buffer.clear();
		copy(strbuf.begin(), strbuf.end(), back_inserter(buffer));
		// Change length
		std::vector<std::string> filters;
		getFilters(filters);
		if (filters.size()) {
			// TODO when can this happen? Our changes are
			// always made in the separate streams!!!
			kernelPrintDbg(debug::DBG_DBG, "Removing Filter entry from the stream");
			dictionary.delProperty ("Filter");
		}
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
	// Parsing (use friend CStreamsXpdfReader)
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
	 * Get next xpdf object from the stream.
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
	~CStream ();



	//
	// Helper methods
	//
public:
	/**
	 * Make xpdf Object from this object. This function allocates and initializes xpdf object.
	 * Caller has to deallocate the xpdf Object.
	 *
	 * @return Xpdf object representin this object.
	 *
	 * \exception ObjBadValueE Thrown when xpdf can't parse the string representation of this
	 * object correctly.
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
	 * REMARK: Be carefull. Deallocate the object.
	 * 
	 * @return Context in which a change occured.
	 */
	ObserverContext* _createContext () const;

public:
	/**
	 * Return all child objects.
	 *
	 * @param store Container of objects.
	 */
	template <typename Storage>
	void _getAllChildObjects (Storage& store) const
	{
		CDict::Value::const_iterator it = dictionary.value.begin ();
		for	(; it != dictionary.value.end (); ++it)
			store.push_back ((*it).second);
	}
	
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
 * @param out Output valid string.
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
void parseStreamToContainer (T& container, const ::Object& obj);


	
/**
 * Create xpdf object from string.
 *
 * @param buffer Stream buffer.
 * @param dict Stream dictionary.
 *
 * @return Xpdf object.
 */
::Object* xpdfStreamObjFromBuffer (const CStream::Buffer& buffer, const CDict& dict);


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


//=========================================================
//	CDict "get type" helper methods
//=========================================================


/** 
 * Get stream from dictionary. If it is CRef fetch the object pointed at.
 */
template<typename IP>
inline boost::shared_ptr<CStream>
getCStreamFromDict (IP& ip, const std::string& key)
	{return getTypeFromDictionary<CStream> (ip, key);}

//=========================================================
//	CArray "get type" helper methods
//=========================================================

/** 
 * Get stream from array. If it is CRef fetch the object pointed at.
 */
template<typename IP>
inline boost::shared_ptr<CStream>
getCStreamFromArray (IP& ip, size_t pos)
	{return getTypeFromArray<CStream> (ip, pos);}




//=====================================================================================
} // namespace utils
//=====================================================================================
} // namespace pdfobjects
//=====================================================================================


#endif // _CSTREAM_H

