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


//=====================================================================================
namespace pdfobjects {
//=====================================================================================

		
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
		
		actstream->close ();
		oss.close();
	}

	/** Get xpdf object. */
	void getXpdfObject (::Object& obj)
	{
		assert (!actstream->eof());
		
		// Get an object
		actstream->getXpdfObject (obj);

		/** debugging \TODO remove. */
		objread ++;
		std::string tmp;
		if (!obj.isEOF())
		{
			utils::xpdfObjToString (obj,tmp);
			oss << objread << " " << tmp << std::endl;
		}

		// If we are at the end of this stream but another stream is not empty 
		// get the object
		if (actstream->eof() && !eof())
		{
			assert (obj.isEOF());
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

	/** End of actual stream. */
	bool eofOfActualStream ()
		{ return (actstream->eof()); }

	/** Get xpdf stream. */
	::Stream* getXpdfStream ()
		{ return actstream->getXpdfStream(); }

	
};

//=====================================================================================
} /* namespace pdfobjects */
//=====================================================================================



#endif // _CSTREAMSXPDFREADER_H

