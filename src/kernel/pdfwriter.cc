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

#include "kernel/xpdf.h"
#include "kernel/static.h"
#include "kernel/pdfwriter.h"
#include "kernel/cobject.h"
#include <zlib.h>

/** Size of buffer for xref table row.
 * This includes also 1 byte for trailing '\0' (end of string marker).
 */
#define XREFROWLENGHT 21

const char * PDFHEADER="%PDF-";

const char * TRAILER_KEYWORD="trailer";

const char * XREF_KEYWORD="xref";

const char * STARTXREF_KEYWORD="startxref";

const char * EOFMARKER="%%EOF";

namespace pdfobjects
{

namespace utils
{
	
void ProgressObserver::notify(boost::shared_ptr<OperationStep> newValue,
		boost::shared_ptr<const observer::IChangeContext<OperationStep> > context)const throw()
{
using namespace boost;
using namespace observer;

	// if no progress visualizator is set or no context is provided,
	// immediatelly returns
	if(!progressBar || !context.get())
		return;
	
	// gets current step and context - which has to have
	// ScopedChangeContextType
	size_t currStep=newValue->currStep;
	if(context->getType()!=ScopedChangeContextType)
	{
		printf("Unsupported context.\n");
		return;
	}
	shared_ptr<const IPdfWriter::ChangeContext> progressContext=
		dynamic_pointer_cast<const IPdfWriter::ChangeContext>(context);
	size_t total=progressContext->getScope()->total;
	if(!started)
	{
		// progress has just started
		// starts progress, sets maximum steps and mark progress as started
		progressBar->start();
		progressBar->setMaxStep(total);
		started=true;
	}

	// updates progress bar
	progressBar->update(currStep);

	// handles last step
	if(currStep==total)
	{
		started=false;
		progressBar->finish();
	}
}

/** Stream writer implementation with no filters.
 * This writer will write raw stream data without any compression
 */
class NullFilterStreamWriter: public FilterStreamWriter
{
	static boost::shared_ptr<NullFilterStreamWriter> instance;
public:
	static boost::shared_ptr<NullFilterStreamWriter> getInstance()
	{
		if(!instance)
			instance=boost::shared_ptr<NullFilterStreamWriter>(
					new NullFilterStreamWriter());

		return instance;
	}

	/** Checks whether given object is supported.
	 * @return allways true as it can write all stream objects.
	 */
	virtual bool supportObject(UNUSED_PARAM Object& obj)const
	{
		assert(obj.isStream());
		return true;
	}

	/** Writes given stream object to the stream.
	 * @param obj Stream object.
	 * @param ref Indirect reference for object (NULL if direct).
	 * @param outStream Stream where to write data.
	 *
	 * Uses streamToCharBuffer with bufferFromStreamData extractor.
	 */
	virtual void compress(Object& obj, Ref* ref, StreamWriter& outStream)const
	{
		assert(obj.isStream());
		CharBuffer charBuffer;
		size_t size=streamToCharBuffer(obj, ref, charBuffer, convertStreamToDecodedData);
		if(!size)
		{
			utilsPrintDbg(debug::DBG_WARN, "zero size stream returned. Probably error in the the object");
			return;
		}
		outStream.putLine(charBuffer.get(), size);
	}
};

/** Implementation of FlateDecode filter stream writer.
 * It is based on zlib implementation of default deflate method.
 */
class ZlibFilterStreamWriter: public FilterStreamWriter
{
	/** Shared writer instance */
	static boost::shared_ptr<ZlibFilterStreamWriter> instance;

	/** Updates given stream object with the applied fiter data.
	 * @param obj Stream object.
	 *
	 * Only for internal use of ZlibFilterStreamWriter class.
	 */
	static void update_dict(Object& obj)
	{
		assert(obj.isStream());
		Object filterArray, filterName;
		filterName.initName("FlateDecode");
		filterArray.initArray(obj.getDict()->getXRef());
		filterArray.arrayAdd(&filterName);
		Dict * dict = obj.streamGetDict();
		dict->add(copyString("Filter"), &filterArray);
	}
public:
	static boost::shared_ptr<ZlibFilterStreamWriter> getInstance()
	{
		if(!instance)
			instance=boost::shared_ptr<ZlibFilterStreamWriter>(
					new ZlibFilterStreamWriter());

		return instance;
	}

	/** Checks whether given stream object is supported by this writer.
	 * @param obj Stream object.
	 * @param allways true.
	 */
	virtual bool supportObject(UNUSED_PARAM Object& obj)const
	{
		assert(obj.isStream());
		return true;
	}

	/** Compress given buffer with deflate method.
	 * @param in Input buffer.
	 * @param in_size Input buffer size.
	 * @param size Size of the output buffer data.
	 * @return allocated buffer with the size data bytes or NULL on failure.
	 *
	 * Uses zlib interface to deflate given data.
	 */
	static unsigned char* deflate_buffer(unsigned char * in, size_t in_size, size_t& size)
	{
		z_stream z;
		z.zalloc = NULL; 
		z.zfree = NULL;
		z.opaque = NULL;
		z.next_in = in; 
		z.avail_in = in_size; 

		// we do expect some comprimation so in_size should be definitely enough
		size_t out_size = in_size;
		unsigned char * out_buff = (unsigned char*)malloc(sizeof(unsigned char)*out_size);
		int ret;
		size_t total=0;

		if(!out_buff)
		{
			utilsPrintDbg(debug::DBG_CRIT, "Unable to allocate buffer with size="<<out_size);
			goto out_error;
		}
		z.next_out = out_buff; 
		z.avail_out = out_size;
		if ((ret = deflateInit(&z, Z_DEFAULT_COMPRESSION)) != Z_OK)
		{
			utilsPrintDbg(debug::DBG_ERR, "deflateInit failed with ret="<<ret);
			goto out_free_error;
		}
		do
		{
			size_t window_size = out_size;
			// we need to reallocate if output buffer run out
			// of space
			if(z.avail_out == 0)
			{
				out_size *= 2;
				utilsPrintDbg(debug::DBG_DBG, 
						"Output buffer size not sufficient, resizing to "
						<< out_size);
				unsigned char *tmp_buff = (unsigned char*)realloc(out_buff, out_size);
				if(!tmp_buff)
				{
					utilsPrintDbg(debug::DBG_CRIT, "Unable to reallocate buffer with size="<<out_size);
					goto out_free_error;
				}
				out_buff = tmp_buff;
				window_size = z.avail_out = out_size - total;
				z.next_out = out_buff + total;
			}
			ret = ::deflate(&z, Z_FINISH);
			if(ret < 0)
			{
				utilsPrintDbg(debug::DBG_ERR, "compression failed with ret="<<ret);
				goto out_free_error;
			}
			total += window_size - z.avail_out;
		}while(z.avail_out == 0);
		assert(z.avail_in == 0);
		assert(ret == Z_STREAM_END);
		deflateEnd(&z);
		size = total;
		return out_buff;

	out_free_error:
		free(out_buff);
	out_error:
		deflateEnd(&z);
		return NULL;
	}

	/** Stream data extractor implementation for streamToCharBuffer function.
	 * @param obj Stream object.
	 * @param size Size of the returned buffer data.
	 * @return allocated buffer with data or NULL on failure.
	 * 
	 * Uses bufferFromStreamData to get raw data without any filters,
	 * compresses returned buffer with the deflate_buffer function and 
	 * updates given stream object's dictionary to contain proper filter 
	 * data.
	 */
	static unsigned char* deflate(Object& obj, size_t& size)
	{
		assert(obj.isStream());
		unsigned char *rawBuffer, *deflateBuff = NULL;
		size_t rawSize;
		if((rawBuffer = convertStreamToDecodedData(obj, rawSize)) == NULL)
			goto out;
		// This should never happen - why would we want to have/change 
		// emty streams? It is much simpler to remove it from the streams
		// if we want to get rid of it. Nevertheless we have already seen
		// streams with zero decoded content (e.g. PDFreference obj. 
		// [4129 0] - content stream from the page 1236)
		// If we let an empty string to the deflate_buffer, this will fail
		// and we will get corrupted document!!!
		if(!rawSize)
		{
			size = rawSize;
			return rawBuffer;
		}
		utilsPrintDbg(debug::DBG_DBG, "Raw buffer size="<<rawSize);
		if((deflateBuff = deflate_buffer(rawBuffer, rawSize, size))==NULL)
			goto free_out;
		utilsPrintDbg(debug::DBG_DBG, "Compressed buffer size="<<size);
		update_dict(obj);

	free_out:
		free(rawBuffer);
	out:
		return deflateBuff;
	}

	virtual void compress(Object& obj, Ref* ref, StreamWriter& outStream)const
	{
		CharBuffer charBuffer;
		assert(obj.isStream());
		size_t size=streamToCharBuffer(obj, ref, charBuffer, deflate);
		if(!size)
		{
			utilsPrintDbg(debug::DBG_WARN, "zero size stream returned. Probably error in the the object");
			return;
		}
		outStream.putLine(charBuffer.get(), size);
		return;
	}
};

// initialization of static data for FilterStreamWriter classes
boost::shared_ptr<NullFilterStreamWriter> NullFilterStreamWriter::instance;
boost::shared_ptr<ZlibFilterStreamWriter> ZlibFilterStreamWriter::instance;
boost::shared_ptr<FilterStreamWriter> FilterStreamWriter::defaultWriter = ZlibFilterStreamWriter::getInstance();
FilterStreamWriter::WritersList FilterStreamWriter::writers;

void FilterStreamWriter::registerFilterStreamWriter(boost::shared_ptr<FilterStreamWriter> streamWriter)
{
	writers.push_back(streamWriter);
}

void FilterStreamWriter::setDefaultStreamWriter(boost::shared_ptr<FilterStreamWriter> streamWriter)
{
	defaultWriter = streamWriter;
}


/** Helper function to find the first filter writer which supports given
 * object.
 * @param obj Stream object to write.
 * @param filters Container of supported filter writers.
 * @return Appropriate filter writer or NULL.
 */
boost::shared_ptr<FilterStreamWriter> lookupFilterStreamWriter(Object& obj, 
		FilterStreamWriter::WritersList& filters)
{
	FilterStreamWriter::WritersList::const_iterator i;
	for(i=filters.begin(); i!=filters.end(); ++i)
	{
		boost::shared_ptr<FilterStreamWriter> writer=*i;
		if(writer->supportObject(obj))
			return writer;
	}
	return boost::shared_ptr<FilterStreamWriter>();
}

boost::shared_ptr<FilterStreamWriter> FilterStreamWriter::getInstance(Object& objStream)
{
	if(!objStream.isStream())
		throw ElementBadTypeException("");
	boost::shared_ptr<FilterStreamWriter> suppWriter = lookupFilterStreamWriter(objStream, writers);
	if(suppWriter)
		return suppWriter;
	if(!defaultWriter || !defaultWriter->supportObject(objStream))
		return NullFilterStreamWriter::getInstance();
	return defaultWriter;
		
}

/** Helper method for xpdf object writing to the stream.
 * @param obj Xpdf object to write.
 * @param ref Object's reference (NULL for indirect object).
 * @param stream Stream where to write.
 * @param indirect Flag for indirect object
 *
 * Creates correct pdf string representation of given object, adds indirect
 * header and footer if indirect flag is specified and writes everything to 
 * the given stream. 
 * <br>
 * Given xpdf object data (like stream or string) can contain unprintable or 
 * 0 bytes.
 */
void writeObject(::Object & obj, StreamWriter & stream, ::Ref* ref, bool indirect)
{
using namespace boost;
using namespace std;

	// stream requires special handling, because it may
	// contain binary data
	if(obj.isStream())
	{
		shared_ptr<FilterStreamWriter> filter = FilterStreamWriter::getInstance(obj);
		assert(filter->supportObject(obj));
		filter->compress(obj, ref, stream);
	}else
	{
		// converts xpdf object to cobject and gets correct string
		// representation
		scoped_ptr<IProperty> cobj_ptr(createObjFromXpdfObj(obj));
		string objPdfFormat;
		cobj_ptr->getStringRepresentation(objPdfFormat);
		
		if(indirect)
		{
			// we have to add some more information to write indirect
			// object (this includes header and footer)
			std::string indirectFormat;
			IndiRef indiRef(*ref);
			createIndirectObjectStringFromString(indiRef, objPdfFormat, indirectFormat);
			objPdfFormat = indirectFormat;
		}

		// indirectFormat may contain 0 bytes, so we can't use c_str()
		// method and have to copy all bytes to the CharBuffer (which also
		// handles correct deallocation)
		size_t bufSize=objPdfFormat.length();
		char * buf=char_buffer_new(bufSize);
		CharBuffer charBuffer(buf, char_buffer_delete());
		for(size_t i=0; i<bufSize; i++)
			buf[i]=objPdfFormat[i];
		stream.putLine(buf, bufSize);
	}
}

const std::string OldStylePdfWriter::CONTENT = "Content phase"; 
const std::string OldStylePdfWriter::TRAILER = "XREF/TRAILER phase";

void OldStylePdfWriter::writeContent(ObjectList & objectList, StreamWriter & stream, size_t off)
{
using namespace debug;
using namespace boost;

	utilsPrintDbg(DBG_DBG, "pos="<<off);
	
	// if off is not 0, uses it to set position in the stream, otherwise uses
	// current position
	if(off)
		stream.setPos(off);

	ObjectList::iterator i;
	size_t index=0;
	
	// creates context for observers
	shared_ptr<OperationScope> scope(new OperationScope());
	scope->total=objectList.size();
	scope->task=CONTENT;
	shared_ptr<ChangeContext> context(new ChangeContext(scope));
	shared_ptr<OperationStep> newValue(new OperationStep());

	// prepares offTable and writes objects
	for(i=objectList.begin(); i!=objectList.end(); i++, index++)
	{
		::Ref ref=i->first;
		Object * obj=i->second;

		// object must be valid
		if(!obj)
		{
			utilsPrintDbg(DBG_WARN, "Object with "<<ref<<" is not valid. Skipping.");
			continue;
		}
		
		// no duplicities are allowed, because previous object wouldn't be
		// available
		if(offTable.find(ref)!=offTable.end())
		{
			utilsPrintDbg(DBG_WARN, "Object with "<<ref<<" is already stored. Skipping.");
			continue;
		}

		// updates maximum Object number if ref is the one
		if(ref.num>maxObjNum)
			maxObjNum=ref.num;

		// associate given reference with current position.
		size_t objPos=stream.getPos();
		offTable.insert(OffsetTab::value_type(ref, objPos));		
		
		writeObject(*obj, stream, &ref, true);	
		utilsPrintDbg(DBG_DBG, "Object with "<<ref<<" stored at offset="<<objPos);
		
		// calls observers
		newValue->currStep=index;
		notifyObservers(newValue, context);
	}
	
	utilsPrintDbg(DBG_DBG, "All objects (number="<<objectList.size()<<") stored.");
}

size_t OldStylePdfWriter::writeTrailer(Object & trailer, PrevSecInfo prevSection, StreamWriter & stream, size_t off)
{
using namespace std;
using namespace debug;
using namespace boost;

	utilsPrintDbg(DBG_DBG, "");
	
	// nothing has been stored, so no need for cross ref and trailer
	if(!offTable.size())
	{
		utilsPrintDbg(DBG_WARN, "No data stored. Skipping cross ref and trailer.");
		return stream.getPos();
	}
		
	// stores start position of the cross reference table
	size_t xrefPos;
	if(!off)
		// use current position from stream if none specified in parameter
		xrefPos=stream.getPos();
	else
	{
		// use position from given parameter
		stream.setPos(off);
		xrefPos=off;
	}

	// subsection offTable type
	// 	- key is object number of subsection leader
	// 	- value is an array of entries (file offset and generation number
	// 	  pairs). This array contains sequence of entries each for one object
	// 	  number. This sequence is without holes (n-th element has key+n object
	// 	  number - n starts from 0)
	typedef pair<size_t, int> EntryType;
	typedef vector<EntryType> EntriesType;
	typedef map< int , EntriesType> SubSectionTab;

	// creates subsection offTable from offset offTable:
	// Starts with first element from offset offTable - inserts num as key and gen
	// and offset as entry pair.
	SubSectionTab subSectionTable;
	SubSectionTab::iterator sub=subSectionTable.begin();
	
	utilsPrintDbg(DBG_DBG, "Creating subsection offTable");
	
	// goes through rest entries of offset offTable
	for(OffsetTab::iterator i=offTable.begin(); i!=offTable.end(); i++)
	{
		int num=(i->first).num;
		int gen=(i->first).gen;
		size_t off=i->second;
		
		// skips not assigned subsection
		if(sub!=subSectionTable.end())
		{
			// if num can be added to current sub, appends entries array
			// NOTE: num can be added if sub's key+entries size == num, which
			// means that entries array won't destroy sequence without holes
			// condition after addition
			if((size_t)num == sub->first + (sub->second).size())
			{
				utilsPrintDbg(DBG_DBG, "Appending num="<<num<<" to section starting with num="<<sub->first);
				(sub->second).push_back(EntryType(off, gen));
				continue;
			}
		}

		// num can't be added, so new subsection has to be created and this is
		// used for next offset offTable elements
		EntriesType entries;
		entries.push_back(EntryType(off, gen));
		pair<SubSectionTab::iterator, bool> ret=subSectionTable.insert(pair<int, EntriesType>(num, entries));
		utilsPrintDbg(DBG_DBG, "New subsection created with starting num="<<num);
		sub=ret.first;
	}

	// cross reference offTable starts with xref row
	stream.putLine(XREF_KEYWORD, strlen(XREF_KEYWORD));

	// subsection offTable is created, we can dump it to the file
	// xrefRow represents one line of xref offTable which is exactly XREFROWLENGHT
	// lenght
	char xrefRow[XREFROWLENGHT];
	memset(xrefRow, '\0', sizeof(xrefRow));
	utilsPrintDbg(DBG_DBG, "Writing "<<subSectionTable.size()<<" subsections");
	
	// creates context for observers
	shared_ptr<OperationScope> scope(new OperationScope());
	scope->total=subSectionTable.size();
	scope->task=TRAILER;
	shared_ptr<ChangeContext> context(new ChangeContext(scope));

	// writes all subsection
	size_t index=1;
	for(SubSectionTab::iterator i=subSectionTable.begin(); i!=subSectionTable.end(); i++, index++)
	{
		// at first writes head object number and number of elements in the
		// subsection
		EntriesType & entries=i->second;
		int startNum=i->first;
		utilsPrintDbg(DBG_DBG, "Starting subsection with startPos="<<startNum<<" and size="<<entries.size());

		snprintf(xrefRow, sizeof(xrefRow)-1, "%d %d", startNum, (int)entries.size());
		stream.putLine(xrefRow, strlen(xrefRow));

		// now prints all entries for this subsection
		// one entry per line
		// according specification (3.4.3 Cross reference table), 
		// line has following format:
		// nnnnnnnnnn ggggg n eoln
		// 	where
		// 		n* stands for file offset of object (padded by leading 0)
		// 		g* is generation number (padded by leading 0)
		// 		n is literal keyword identifying in-use object
		// 		eoln 2 characters end of line. If file uses 1 character
		// 		     end of line character, it is preceeded by one space.
		// Each entry is exactly 20 bytes long including the end-of-line marker.
		// We don't provide information about free objects
		for(EntriesType::iterator entry=entries.begin(); entry!=entries.end(); entry++)
		{
			int ret = snprintf(xrefRow, sizeof(xrefRow)-1, 
					"%010u %05i n ", 
					(unsigned int)entry->first, 
					entry->second);
			if(ret<19)
				utilsPrintDbg(DBG_WARN, "Xref entry to short ("
						<<ret<<") for "<<xrefRow);
			// putLine uses simple LF end-of-line marker
			stream.putLine(xrefRow, strlen(xrefRow));
		}
		
		// notifies observers
		shared_ptr<OperationStep> newValue(new OperationStep());
		newValue->currStep=index;
		notifyObservers(newValue, context);
	}

	// updates Prev field - to say where previous xref table starts
	// if 0, removes Prev entry if present
	if(!prevSection.xrefPos)
	{
		// FIXME prepare for stream trailers - we don't have this
		// problem at the moment, becayse we don't create such 
		// trailers and the first one never contains Prev. However
		// this can be problem if we had incremental update from 
		// other party which uses stream trailers for all.
		Object * prev=trailer.getDict()->del("Prev");
		if(prev)
			xpdf::freeXpdfObject(prev);
		utilsPrintDbg(DBG_DBG, "No previous xref section. Removing Trailer::Prev.");
	}else
	{
		Object newPrev;
		newPrev.initInt(prevSection.xrefPos);
		char * key=copyString("Prev");
		Object * originalPrev=trailer.getDict()->update(key, &newPrev);
		if(originalPrev)
		{
			// value has been set to something different, we have to deallocate it
			// and to free key, because it is not stored in update
			utilsPrintDbg(DBG_DBG, "Removing old Trailer::Prev="<<originalPrev->getInt());
			free(key);
			xpdf::freeXpdfObject(originalPrev);
		}
		utilsPrintDbg(DBG_DBG, "Linking to previous xref section. Trailer::Prev="<<newPrev.getInt());
	}

	// hybrid xref files can contain both xref table and xref stream
	// in such a case startxref points to xreftable and trailer::XrefStm
	// to the additional objects in xref stream. PDF>=1.5 capable readers
	// reads both of them and so we have to remove XRefStm for later 
	// revisions to prevent from confusions.
	Object * xrefStm = trailer.getDict()->del("XRefStm");
	if(xrefStm)
	{
		utilsPrintDbg(DBG_DBG, "Removing old Trailer::XRefStm.");
		xpdf::freeXpdfObject(xrefStm);
	}

	// sets Size entry with the maximum from the original entries count and 
	// the highest changed object reference number
	Object newSize;
	newSize.initInt(std::max(prevSection.entriesNum, (size_t)(maxObjNum + 1)));
	char * key=copyString("Size");
	Object * originalSize=trailer.getDict()->update(key, &newSize);
	if(originalSize)
	{
		// value has been set to something different, we have to deallocate it
		// and to free key, because it is not stored in update
		utilsPrintDbg(DBG_DBG, "Removing old Trailer::Size="<<originalSize->getInt());
		gfree(key);
		xpdf::freeXpdfObject(originalSize);
	}
	utilsPrintDbg(DBG_DBG, "Setting Trailer::Size="<<newSize.getInt());

	// stores changed trailer to the file
	stream.putLine(TRAILER_KEYWORD, strlen(TRAILER_KEYWORD));
	writeObject(trailer, stream, NULL, false);
	kernelPrintDbg(DBG_DBG, "Trailer saved");

	// stores offset of last (created one) xref table
	stream.putLine(STARTXREF_KEYWORD, strlen(STARTXREF_KEYWORD));
	char xrefPosStr[128];
	sprintf(xrefPosStr, "%u", (unsigned int)xrefPos);
	stream.putLine(xrefPosStr, strlen(xrefPosStr));
	
	// Finaly puts %%EOF behind but keeps position of marker start
	size_t pos=stream.getPos();
	stream.putLine(EOFMARKER, strlen(EOFMARKER));
	kernelPrintDbg(DBG_DBG, "PDF end of file marker saved");

	// stream may contain some non sense information behind, so they has to be
	// cleaned.
	size_t currPos=stream.getPos();
	stream.setPos(0, -1);
	size_t eofPos=stream.getPos();
	if(eofPos>currPos)
	{
		size_t size=eofPos-currPos;
		kernelPrintDbg(DBG_DBG, "Cleaning pending ("<<size<<"B) data behind stored revision.");
		stream.trim(currPos);
	}

	// resets internal data
	reset();

	return pos;
}

void OldStylePdfWriter::reset()
{
	offTable.clear();
	maxObjNum=0;
}

} // utils namespace

} // pdfobjects namespace
