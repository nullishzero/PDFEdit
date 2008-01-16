/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko,
 *                                              Miroslav Jahoda,
 *                                              Jozef Misutka,
 *                                              Martin Petricek
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit
 */
// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80

#include "kernel/xpdf.h"
#include "kernel/static.h"
#include "kernel/pdfwriter.h"
#include "kernel/cobject.h"

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

/* FIXME unify writeDirectObject and writeIndirectObject because they contain 
 * a lot of common code
 */

/** Helper method for xpdf direct object writing to the stream.
 * @param obj Xpdf object to write.
 * @param stream Stream where to write.
 *
 * Creates correct pdf string representation of given object and writes
 * everything to the given stream. Given xpdf object data (like stream
 * or string) can contain unprintable or 0 bytes.
 */
void writeDirectObject(::Object & obj, StreamWriter & stream)
{
using namespace boost;
using namespace std;

	// stream requires special handling, because it may
	// contain binary data
	if(obj.isStream())
	{
		CharBuffer charBuffer;
		::Ref ref={0,0};
		size_t size=streamToCharBuffer(obj, ref, charBuffer, false);
		stream.putLine(charBuffer.get(), size);
	}else
	{
		// converts xpdf object to cobject and gets correct string
		// representation
		scoped_ptr<IProperty> cobj_ptr(createObjFromXpdfObj(obj));
		string objPdfFormat;
		cobj_ptr->getStringRepresentation(objPdfFormat);
		
		// objPdfFormat may contain 0 bytes, so we can't use c_str()
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
/** Helper method for xpdf object writing to the stream.
 * @param obj Xpdf object to write.
 * @param ref Object's reference.
 * @param stream Stream where to write.
 *
 * Creates correct pdf string representation of given object, adds indirect
 * header and footer and writes everything to the given stream. Given xpdf
 * object data (like stream or string) can contain unprintable or 0 bytes.
 */
void writeIndirectObject(::Object & obj, ::Ref ref, StreamWriter & stream)
{
using namespace boost;
using namespace std;

	// stream requires special handling, because it may
	// contain binary data
	if(obj.isStream())
	{
		CharBuffer charBuffer;
		size_t size=streamToCharBuffer(obj, ref, charBuffer, true);
		stream.putLine(charBuffer.get(), size);
	}else
	{
		// converts xpdf object to cobject and gets correct string
		// representation
		scoped_ptr<IProperty> cobj_ptr(createObjFromXpdfObj(obj));
		string objPdfFormat;
		cobj_ptr->getStringRepresentation(objPdfFormat);
		
		// we have to add some more information to write indirect
		// object (this includes header and footer)
		std::string indirectFormat;
		IndiRef indiRef(ref);
		createIndirectObjectStringFromString(indiRef, objPdfFormat, indirectFormat);

		// indirectFormat may contain 0 bytes, so we can't use c_str()
		// method and have to copy all bytes to the CharBuffer (which also
		// handles correct deallocation)
		size_t bufSize=indirectFormat.length();
		char * buf=char_buffer_new(bufSize);
		CharBuffer charBuffer(buf, char_buffer_delete());
		for(size_t i=0; i<bufSize; i++)
			buf[i]=indirectFormat[i];
		stream.putLine(buf, bufSize);
	}
}
	
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
		
		writeIndirectObject(*obj, ref, stream);	
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
			freeXpdfObject(prev);
		utilsPrintDbg(DBG_DBG, "No previous xref section. Removing Trailer::Prev.");
	}else
	{
		Object newPrev;
		newPrev.initInt(prevSection.xrefPos);
		char * key=strdup("Prev");
		Object * originalPrev=trailer.getDict()->update(key, &newPrev);
		if(originalPrev)
		{
			// value has been set to something different, we have to deallocate it
			// and to free key, because it is not stored in update
			utilsPrintDbg(DBG_DBG, "Removing old Trailer::Prev="<<originalPrev->getInt());
			free(key);
			freeXpdfObject(originalPrev);
		}
		utilsPrintDbg(DBG_DBG, "Linking to previous xref section. Trailer::Prev="<<newPrev.getInt());
	}

	// sets Size entry with object maximum object number written to the file
	Object newSize;
	newSize.initInt(std::max(prevSection.objNum, (size_t)(maxObjNum + 1)));
	char * key=strdup("Size");
	Object * originalSize=trailer.getDict()->update(key, &newSize);
	if(originalSize)
	{
		// value has been set to something different, we have to deallocate it
		// and to free key, because it is not stored in update
		utilsPrintDbg(DBG_DBG, "Removing old Trailer::Size="<<originalSize->getInt());
		free(key);
		freeXpdfObject(originalSize);
	}
	utilsPrintDbg(DBG_DBG, "Setting Trailer::Size="<<newSize.getInt());

	// stores changed trailer to the file
	stream.putLine(TRAILER_KEYWORD, strlen(TRAILER_KEYWORD));
	writeDirectObject(trailer, stream);
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
