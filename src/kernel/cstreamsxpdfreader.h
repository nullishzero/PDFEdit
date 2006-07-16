// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80
/* 
 * =====================================================================================
 *        Filename:  cstreamsxpdfreader.h
 *     Description:  Header file containing definition of IProperty and CObject classes.
 *         Created:  01/18/2006 
 *          Author:  jmisutka (06/01/19), 
 *
 * =====================================================================================
 */
#ifndef _CSTREAMSXPDFREADER_H
#define _CSTREAMSXPDFREADER_H

// all basic includes
#include "static.h"
#include "iproperty.h"
// Filters
#include "cstream.h"
#include "cobjecthelpers.h"

/** 
 * Problem only cstream use is that streams can be separated in crazy
 * places e.g. first stream: "... ["	second stream: " fdas  as fs]"
 * and xpdf does not return valid objects when stream ends in the middle of
 * an object.
 */
#define CSTREAMSREADER_USE_XPDF	1

//=====================================================================================
namespace pdfobjects {
//=====================================================================================

#if CSTREAMSREADER_USE_XPDF

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
	CStreams streams;			/**< Array of streams. */
	xpdf::XpdfObject xarr;		/**< Xpdf array of streams. */
	xpdf::XpdfObject curobj;	/**< Current object. */

	boost::shared_ptr<Parser> parser; /**< Xpdf parser. */
	::Lexer* lexer;					  /**< Xpdf lexer. */

	/**\todo debug */
	size_t objread;	/**< Helper variable for debugging. Number of read objects. */
	std::ofstream oss;
	

public:

	/** Constructor. */
	CStreamsXpdfReader (Container& strs) :  lexer(NULL), objread (0)
		{ assert (!strs.empty()); std::copy (strs.begin(), strs.end(), std::back_inserter(streams)); }
	CStreamsXpdfReader (boost::shared_ptr<CStream> str) : lexer(NULL), objread (0)
		{ assert (str); streams.push_back (str); }

	/** Open. */
	void open ()
	{		
		assert (!streams.empty());
		curobj.reset ();
		
		// Get xref
		::XRef* xref = utils::getXRef (streams.front());

		// Create array of streams to parse
		xarr->initArray (xref);
		for (CStreams::iterator it = streams.begin(); it != streams.end(); ++it)
		{
			assert (hasValidRef(*it) && hasValidPdf (*it));
			IndiRef rf = (*it)->getIndiRef();
			xpdf::XpdfObject tmp;
			tmp->initRef (rf.num, rf.gen);
			xarr->arrayAdd (tmp.get());
		}
		assert (streams.size() == (size_t)xarr->arrayGetLength());
		
		// Create parser
		lexer = new ::Lexer (xref, xarr.get());
		parser = boost::shared_ptr<Parser> (new ::Parser (xref, lexer));

		/**\todo DEBUG */
		oss.open ("_stream");
	}

	/** Close. */
	void close ()
	{
		parser.reset();
		lexer = NULL;
		xarr.reset ();
		curobj.reset ();
		
		/**\todo DEBUG */
		oss.close();
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
		
		for (size_t i = 0; i < lexer->strIndex(); ++i)
			parsedstreams.push_back (streams[i]);
		
		close();
		
		/**\todo DEBUG */
		oss.close();
	}

	/** Get xpdf object. */
	void getXpdfObject (::Object& obj)
	{
		curobj.reset ();
		parser->getObj (curobj.get());
		curobj->copy (&obj);

		/** debugging \TODO remove. */
		objread ++;
		if (!obj.isEOF() && !obj.isError())
		{
			std::string tmp;
			utils::xpdfObjToString (obj,tmp);
			oss << objread << " " << tmp << std::endl;
		}
	}

	/** 
	 * Is end of all streams.
	 *
	 * We can not cache and due to this fact we can not tell if a stream
	 * is empty without fetching an object. 
	 */
	bool eof ()
		{ return (curobj->isEOF()); }

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
	/**\todo debug */
	std::ofstream oss;
	

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

		/**\todo DEBUG */
		oss.open ("_stream");
		
	}

	/** Close. */
	void close ()
	{
		assert (!streams.empty());
		
		// True if no exception occurs
		//assert (streams.size() == pos + 1);
		//assert (actstream == streams.back());
		//assert (actstream->eof());
		
		actstream->close ();

		oss.close();
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
		oss.close();
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

		/** debugging \TODO remove. */
		objread ++;
		std::string tmp;
		if (!obj.isEOF() && !obj.isError())
		{
			utils::xpdfObjToString (obj,tmp);
			oss << objread << " " << tmp << std::endl;
		}

		// If we are at the end of this stream but another stream is not empty 
		// get the object
		if (eofOfActualStream() && !eof())
		{
			actstream->getXpdfObject (obj);

			/** debugging \TODO remove. */
			if (!obj.isEOF())
			{
				utils::xpdfObjToString (obj,tmp);
				oss << objread << " NEW STREAM !!! " << actstream->getIndiRef() << tmp << std::endl;
			}
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
} /* namespace pdfobjects */
//=====================================================================================



#endif // _CSTREAMSXPDFREADER_H

