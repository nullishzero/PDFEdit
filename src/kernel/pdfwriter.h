// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80

/*
 * $RCSfile$
 *
 * $Log$
 * Revision 1.2  2006/05/09 20:10:55  hockm0bm
 * * doc update
 * * writeContent some checking added
 *         - duplicated entries are ignored
 *         - NULL entries are ignored
 *
 * Revision 1.1  2006/05/08 20:12:19  hockm0bm
 * * abstract IPdfWriter class for pdf content writers
 * * OldStylePdfWriter implementation of IPdfWriter
 * * pdf key words defined here now
 *
 *
 */
#ifndef _PDF_WRITER_H_
#define _PDF_WRITER_H_

#include "streamwriter.h"

/** Marker of trailer dictionary.
 *
 * Each trailer dictionary begins immediately after line containing this string.
 */
extern const char * TRAILER_KEYWORD;

/** Marker of cross reference table.
 * Each xref section begins immediately after line containing this string.
 */
extern const char * XREF_KEYWORD;

/** Marker of last cross reference starting offset.
 * This key word marks file offset where xref starts. The number is on following
 * line.
 */
extern const char * STARTXREF_KEYWORD;

namespace pdfobjects {

namespace utils {

/** Interface for pdf content writer.
 *
 * Implementator knows how to put data to the file to create correct pdf
 * content.
 * <br>
 * Pdf content is written in 2 phases:
 * <ul>
 * <li>objects writing - when writeContent method is called (one or more times).
 * This data forms revision.
 * <li>cross reference & trailer writing - when all collected data are used for
 * cross reference and trailer to finish PDF revision.
 * </ul>
 *
 * So each sequence of writeContent [, writeContent]* , writeTrailer forms new 
 * document revision. writeTrailer resets internal data and so it can be
 * repeated. 
 * <br>
 * Offset parameter can  change default stream position. Default parameter value
 * is 0, current position is used.
 */
class IPdfWriter
{
public:
	/** Type for ObjectList element. */
	typedef std::pair<Ref, Object *> ObjectElement;

	/** Type for object list.
	 * Each element is pair of object reference and its value.
	 */
	typedef std::vector<ObjectElement> ObjectList;

	virtual ~IPdfWriter(){};

	/** Puts all objects to given stream.
	 * @param objectList List of objects to store.
	 * @param stream Stream writer where to write.
	 * @param off Stream offset where to start writing (if 0, uses current
	 * position).
	 * 
	 * Stores objects from the list and collects all information needed for 
	 * writeTrailer method. Objects which should be marked as free should have
	 * objNull type. It is up to implementator how it handles free objects, but
	 * writing them as null object is also correct.
	 * <br>
	 * Doesn't write xref and trailer.
	 */
	virtual void writeContent(ObjectList & objectList, StreamWriter & stream, size_t off=0) =0;

	/** Writes xref and trailer section.
	 * @param trailer Trailer object.xrefPos.
	 * @param lastXref Position of previous xref section start.
	 * @param stream Stream writer where to write.
	 * @param off Stream offset where to start writing (if 0, uses current
	 * position).
	 *
	 * Finishes pdf content writing by cross reference table and trailer
	 * dictionary. Uses internal data collected by writeContent method.
	 * reset method is called immediately after all data are written.
	 */
	virtual void writeTrailer(Object & trailer, size_t lastXref, StreamWriter & stream, size_t off=0)=0;

	/** Resets internal data collected in writeContent method.
	 *
	 * Everything collected in writeContent method, which is needed by
	 * writeTrailer for cross reference and trailer section generation, is
	 * cleared here.
	 */
	virtual void reset()=0;
};

/** Implementator of old style cross reference table pdf writer.
 *
 * Writes content with old style cross reference table and trailer.
 *
 */
class OldStylePdfWriter: public IPdfWriter
{
	/** Type for offset table.
	 * Mapping from reference to file offset of indirect object.
	 */
	typedef std::map< ::Ref, size_t, RefComparator> OffsetTab;

	/** Offset table.
	 *
	 * Keeps mapping from objects referencies to their position in the stream
	 * (where they were written).
	 *
	 */
	OffsetTab offTable;
public:
	/** Writes given objects.
	 * @param objectList List of objects to write.
	 * @param stream Stream writer where to write.
	 * @param off Stream offset where to start writing (if 0, uses current
	 * position).
	 *
	 * Uses xpdfObjToString method to get string representation with
	 * createIndirectObjectStringFromString decorator function to get proper PDF
	 * representation of indirect object and writes this string using
	 * stream::putLine method.
	 * <br>
	 * First object is stored at specified position and all others are placed
	 * immediately after.
	 */
	virtual void writeContent(ObjectList & objectList, StreamWriter & stream, size_t off=0);

	/** Writes cross reference table and trailer.
	 * @param trailer Trailer object.
	 * @param lastXref Position of previous xref section start.
	 * @param off Stream offset where to start writing (if 0, uses current
	 * position).
	 * @param stream Stream writer where to write.
	 *
	 * Constructs (old style) cross reference table from internal reference to 
	 * file offset mapping to the given stream according PDF specification.
	 * <br>
	 * In first step offTable is used for cross reference table creation. This
	 * contain set of cross reference subsections.
	 * Then xref keyword is written to the stream followed be cross reference
	 * table constructed from subsections.
	 * Finaly sets trailer's Prev field value according lastXref parameter and
	 * writes it also to the stream followed by lastxref key word with position 
	 * of this cross reference section start.
	 * STARTXREF_KEYWORD is written immediately after.
	 * <p>
	 * <b>PDF specification notes:</b>
	 * <br>
	 * Old style cross reference section is set of cross reference subsections.
	 * Each subsection is preceeded by object number and size values. This is
	 * followed by size number of lines. Each line represents object with
	 * (number+line#-1) object number. Line format is as follows:
	 * <pre>
	 * nnnnnnnnnn ggggg n \n
	 * 
	 * where 
	 *		n* stands for file offset of object (padded by leading 0)
	 *		g* is generation number (padded by leading 0)
	 *		n is literal keyword identifying in-use object
	 * </pre>
	 * One subsection represents continuous sequence of indirect object numbers.
	 * <br>
	 * Cross reference subsections should also mark deleted objects, those which
	 * are not accessible anymore and so can be reused with higher generation
	 * number. This implementation doesn't write such entries.
	 */
	virtual void writeTrailer(Object & trailer, size_t lastXref, StreamWriter & stream, size_t off=0);

	/** Resets all collected data.
	 *
	 * Clears offTable field and so this instance can used for another revision.
	 */
	virtual void reset();
};

} // namespace utils

} // namespace pdfobjects

#endif

