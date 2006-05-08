// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80

/*
 * $RCSfile$
 *
 * $Log$
 * Revision 1.1  2006/05/08 20:12:18  hockm0bm
 * * abstract IPdfWriter class for pdf content writers
 * * OldStylePdfWriter implementation of IPdfWriter
 * * pdf key words defined here now
 *
 *
 */

#include "xpdf.h"
#include "static.h"
#include "pdfwriter.h"
#include "cobject.h"

/** Size of buffer for xref table row.
 * This includes also 1 byte for trailing '\0' (end of string marker).
 */
#define XREFROWLENGHT 21
#define EOFMARKER "%%EOF"

const char * TRAILER_KEYWORD="trailer";

const char * XREF_KEYWORD="xref";

const char * STARTXREF_KEYWORD="startxref";

namespace pdfobjects
{

namespace utils
{

void OldStylePdfWriter::writeContent(ObjectList & objectList, StreamWriter & stream, size_t off)
{
using namespace debug;

	utilsPrintDbg(DBG_DBG, "pos="<<off);
	
	// if off is not 0, uses it to set position in the stream, otherwise uses
	// current position
	if(off)
		stream.setPos(off);

	ObjectList::iterator i;
	for(i=objectList.begin(); i!=objectList.end(); i++)
	{
		// TODO objNull should be marked for marking as free - e.g. position set
		// to 0 and writeTrailer should use those entries handled as free.
		
		::Ref ref=i->first;
		// associate given reference with actual position.
		// TODO check if ref already is in mapping and if so, prints an warning
		offTable.insert(OffsetTab::value_type(ref, stream.getPos()));		
		utilsPrintDbg(DBG_DBG, "Object with ref=["<<ref.num<<", "<<ref.gen<<"] stored at offset="<<stream.getPos());
		
		// stores PDF representation of object to current position which is
		// after moved behind written object
		Object * obj=i->second;
		std::string objPdfFormat;
		xpdfObjToString(*obj, objPdfFormat);

		// we have to add some more information to write indirect object (this
		// includes header and footer
		std::string indirectFormat;
		IndiRef indiRef={ref.num, ref.gen};
		createIndirectObjectStringFromString(indiRef, objPdfFormat, indirectFormat);
		stream.putLine(indirectFormat.c_str());
	}
	
	utilsPrintDbg(DBG_DBG, "All objects (number="<<objectList.size()<<") stored.");
}

void OldStylePdfWriter::writeTrailer(Object & trailer, size_t lastXref, StreamWriter & stream, size_t off)
{
using namespace std;
using namespace debug;

	utilsPrintDbg(DBG_DBG, "");
	
	// nothing has been stored, so no need for cross ref and trailer
	if(!offTable.size())
	{
		utilsPrintDbg(DBG_WARN, "No data stored. Skipping cross ref and trailer.");
		return;
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
	stream.putLine(XREF_KEYWORD);

	// subsection offTable is created, we can dump it to the file
	// xrefRow represents one line of xref offTable which is exactly XREFROWLENGHT
	// lenght
	char xrefRow[XREFROWLENGHT];
	memset(xrefRow, '\0', sizeof(xrefRow));
	utilsPrintDbg(DBG_DBG, "Writing "<<subSectionTable.size()<<" subsections");
	for(SubSectionTab::iterator i=subSectionTable.begin(); i!=subSectionTable.end(); i++)
	{
		// at first writes head object number and number of elements in the
		// subsection
		EntriesType & entries=i->second;
		int startNum=i->first;
		utilsPrintDbg(DBG_DBG, "Starting subsection with startPos="<<startNum<<" and size="<<entries.size());

		snprintf(xrefRow, sizeof(xrefRow)-1, "%d %d", startNum, (int)entries.size());
		stream.putLine(xrefRow);

		// now prints all entries for this subsection
		// one entry on one line
		// according specificat, line has following format:
		// nnnnnnnnnn ggggg n \n
		// 	where 
		// 		n* stands for file offset of object (padded by leading 0)
		// 		g* is generation number (padded by leading 0)
		// 		n is literal keyword identifying in-use object
		// We don't provide information about free objects
		for(EntriesType::iterator entry=entries.begin(); entry!=entries.end(); entry++)
		{
			snprintf(xrefRow, sizeof(xrefRow)-1, "%010u %05i n", entry->first, entry->second);
			stream.putLine(xrefRow);
		}
	}

	// updates Prev field - to say where previous xref table starts 
	Object newPrev;
	newPrev.initInt(lastXref);
	char * key=strdup("Prev");
	Object * originalPrev=trailer.getDict()->update(key, &newPrev);
	if(originalPrev)
	{
		// value has been set to something different, we have to deallocate it
		// and to free key, because it is not stored in update
		free(key);
		originalPrev->free();
		gfree(originalPrev);
	}

	// stores changed trialer to the file
	std::string objPdfFormat;
	xpdfObjToString(trailer, objPdfFormat);
	stream.putLine(TRAILER_KEYWORD);
	stream.putLine(objPdfFormat.c_str());
	kernelPrintDbg(DBG_DBG, "Trailer saved");

	// stores offset of last (created one) xref table
	// TODO adds also comment with time and date of creation
	stream.putLine(STARTXREF_KEYWORD);
	char xrefPosStr[128];
	sprintf(xrefPosStr, "%u", xrefPos);
	stream.putLine(xrefPosStr);
	
	// Finaly puts %%EOF behind but keeps position of marker start
	size_t pos=stream.getPos();
	stream.putLine(EOFMARKER);
	kernelPrintDbg(DBG_DBG, "PDF end of file marker saved");

	// resets internal data
	reset();
}

void OldStylePdfWriter::reset()
{
	offTable.clear();
}

} // utils namespace 

} // pdfobjects namespace
