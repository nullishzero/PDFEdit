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
#ifndef _CSTREAMSXPDFREADER_H
#define _CSTREAMSXPDFREADER_H

// all basic includes
#include "kernel/static.h"
#include "kernel/cobject.h"
#include "kernel/cobjecthelpers.h"
#include "kernel/factories.h"

/** 
 * Content streams can be separated in crazy
 * places e.g. first stream: "... ["	second stream: " fdas  as fs]" and the
 * result must be one array of 3 objects. Xpdf does not make possible to read separately each stream and then
 * combine it somehow. We have to use xpdf parser and supply all streams at the
 * beginning. 
 */
#define CSTREAMSREADER_USE_XPDF	1

//=====================================================================================
namespace pdfobjects {
//=====================================================================================

#if CSTREAMSREADER_USE_XPDF

/**
 * Adapter which is able to read sequentially from more cstreams.
 *
 * It stores a container of streams and when the actual stream does not
 * contain more objects, tries to read the next one if any.
 *
 * This class is an example of Adapter design pattern. We have to be able to
 * read from more streams sequentially and this class provides us with the
 * interface.
 */
template<typename Container>
class CStreamsXpdfReader
{
public:
	typedef std::vector<boost::shared_ptr<CStream> > CStreams;

private:
	CStreams streams;			/**< Array of streams. */
	boost::shared_ptr< ::Object> xarr;	/**< Xpdf array of streams. */
	boost::shared_ptr< ::Object> curobj;	/**< Current object. */

	boost::shared_ptr<Parser> parser; /**< Xpdf parser. */
	::Lexer* lexer;					  /**< Xpdf lexer. */

public:

	/** Constructor. */
	CStreamsXpdfReader (Container& strs) :  lexer(NULL)
	{ 
		assert (!strs.empty()); 
		std::copy (strs.begin(), strs.end(), std::back_inserter(streams)); 
		xarr = boost::shared_ptr< ::Object>(XPdfObjectFactory::getInstance(), xpdf::object_deleter());
		curobj = boost::shared_ptr< ::Object>(XPdfObjectFactory::getInstance(), xpdf::object_deleter());
	}
	CStreamsXpdfReader (boost::shared_ptr<CStream> str) : lexer(NULL)
	{ 
		assert (str); 
		streams.push_back (str); 
		xarr = boost::shared_ptr< ::Object>(XPdfObjectFactory::getInstance(), xpdf::object_deleter());
		curobj = boost::shared_ptr< ::Object>(XPdfObjectFactory::getInstance(), xpdf::object_deleter());
	}

	/** Open. */
	void open ()
	{		
		assert (!streams.empty());
		curobj->free ();
		
		// Get xref
		::XRef* xref = utils::getXRef (streams.front());

		// Create array of streams to parse
		xarr->initArray (xref);
		for (CStreams::iterator it = streams.begin(); it != streams.end(); ++it)
		{
			assert (hasValidRef(*it) && hasValidPdf (*it));
			IndiRef rf = (*it)->getIndiRef();
			boost::shared_ptr< ::Object> tmp(XPdfObjectFactory::getInstance(), xpdf::object_deleter());
			tmp->initRef (rf.num, rf.gen);
			xarr->arrayAdd (tmp.get());
		}
		assert (streams.size() == (size_t)xarr->arrayGetLength());
		
		// Create parser
		lexer = new ::Lexer (xref, xarr.get());
		parser = boost::shared_ptr<Parser> (new ::Parser (xref, lexer, 
					gFalse  // TODO gfalse should be ok here
				       		// because content stream must
						// not contain stream objects
					)
				);
	}

	/** Close. */
	void close ()
	{
		parser.reset();
		lexer = NULL;
		xarr->free ();
		curobj->free ();
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
		// True only if no exception occurs
		//assert (actstream->eof());
		
		// If anything went bad
		if (0 == lexer->strIndex())
		{
			parsedstreams.push_back (streams[0]);
		
		}else		
		{
			for (size_t i = 0; i < lexer->strIndex(); ++i)
				parsedstreams.push_back (streams[i]);
		}
		
		close();
	}

	/** Get xpdf object.
	 * @throw MalformedFormatExeption if not able to parse object.
	 */
	void getXpdfObject (::Object& obj)
	{
		curobj->free ();
		if(!parser->getObj (curobj.get()))
		{
			kernelPrintDbg(debug::DBG_ERR, "Unabel to parse object");
			throw MalformedFormatExeption("content stream parse");
		}
		curobj->copy (&obj);
	}

	/** 
	 * Look at next xpdf object. 
	 * It need not return the real next object, but it does at the start
	 * of a stream.
	 */
	void lookXpdfObject (::Object& obj)
		{ parser->getNextObj (obj);	}

	/** 
	 * Is end of all streams.
	 *
	 * We can not cache and due to this fact we can not tell if a stream
	 * is empty without fetching an object. 
	 */
	bool eof ()
		{ return gTrue == (curobj->isEOF()); }

	/** Get xpdf stream. */
	::Stream* getXpdfStream ()
		{ return parser->getStream(); }

	/**  
	 * Are we at the end of actual stream. We need to ask parser, because of
	 * object caching.
	 */
	bool eofOfActualStream ()
		{ return (parser->eofOfActualStream()); }
	
};

	
#else // #if CSTREAMSREADER_USE_XPDF
		
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
class CStreamsXpdfReader
{
public:
	typedef std::vector<boost::shared_ptr<CStream> > CStreams;

private:
	CStreams streams;	/**< All streams. */
	boost::shared_ptr<CStream> actstream;	/**< Actual stream that is beeing parsed. */
	size_t pos;			/**< Position of actual parsed stream in the stream container. */
	size_t objread; 	/**< Helper variable for debugging. Number of read objects. */
	::Object nxtObj;	/**< Sometimes it is good to know if next object is the end. */

public:

	/** Constructor. */
	CStreamsXpdfReader (Container& strs) : pos(0), objread (0)
		{ assert (!strs.empty()); std::copy (strs.begin(), strs.end(), std::back_inserter(streams));}
	CStreamsXpdfReader (boost::shared_ptr<CStream> str) : pos(0), objread (0)
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
		// True only if no exception occurs
		//assert (actstream->eof());
		
		for (size_t i = 0; i <= pos; ++i)
			parsedstreams.push_back (streams[i]);
		
		nxtObj.free ();
		actstream->close ();
	}

	/** Get xpdf object. */
	void getXpdfObject (::Object& obj)
	{
		assert (nxtObj.isEOF() || !eofOfActualStream());
		
		// Get an object
		if (nxtObj.isNone())
		{
			actstream->getXpdfObject (obj);
		}else
		{
			nxtObj.copy (&obj);
			nxtObj.free();
		}

		// If we are at the end of this stream but another stream is not empty 
		// get the object
		if (eofOfActualStream() && !eof())
		{
			actstream->getXpdfObject (obj);
		}
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
				boost::shared_ptr< ::Object> obj(XPdfObjectFactory::getInstance(), xpdf::object_deleter());
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

	/** 
	 * Are we at the end of actual stream. 
	 *
	 * This function looks at the next object if it is the end. 
	 * REMARK: This function has to be called very carefully because we can not
	 * simply cache objects e.g. because of the inline image where all cached
	 * objects are cleared.
	 */
	bool nextEofOfActualStream ()
	{ 
		if (nxtObj.isNone())
			actstream->getXpdfObject (nxtObj);
		return (nxtObj.isEOF()); 
	}

	/** Get xpdf stream. */
	::Stream* getXpdfStream ()
		{ return actstream->getXpdfStream(); }

	/** 
	 * Are we at the end of actual stream. 
	 */
	bool eofOfActualStream ()
		{ return (actstream->eof()); }

	
};

#endif // #if CSTREAMSREADER_USE_XPDF

//=====================================================================================
} // namespace pdfobjects
//=====================================================================================



#endif // _CSTREAMSXPDFREADER_H

