// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80

/*
 * $RCSfile$
 *
 * $Log$
 * Revision 1.7  2006/04/13 18:08:49  hockm0bm
 * * releaseObject method removed
 * * readOnly mode removed - makes no sense in here
 * * documentation updated
 * * contains information about CPdf
 *         - because of pdf read only mode
 * * buildXref method added
 * * XREFROWLENGHT and EOFMARKER macros added
 *
 * Revision 1.6  2006/04/12 17:52:25  hockm0bm
 * * saveChanges method replaces saveXRef method
 *         - new semantic for saving changes
 *              - temporary save
 *              - new revision save
 *         - storePos field added to mark starting
 *           place where to store changes
 *         - saveChanges moves with storePos if new
 *           revision is should be created
 * * findPDFEof function added
 * * StreamWriter in place of BaseStream
 *         - all changes will be done via StreamWriter
 *
 * Revision 1.5  2006/03/23 22:13:51  hockm0bm
 * printDbg added
 * exception handling
 * TODO removed
 * FIXME for revisions handling - throwing exeption
 *
 *
 */
#include "xrefwriter.h"
#include "cobject.h"

using namespace pdfobjects;
using namespace utils;
using namespace debug;

/** Searches for EOFMARKER PDF end of file.
 * @param stream Stream where to find.
 * @param startPos Position in the stream, where to start.
 *
 * Creates substream from given one from given position and searches for %%EOF
 * string. If not found returns position at the end of the stream.
 *
 * @return Postion where it is safe to store changed objects.
 */
size_t findPDFEof(BaseStream * stream, size_t startPos)
{
	Object obj;
	Stream * substream=stream->makeSubStream(startPos, gFalse, 0, &obj);	

	// gets first line of substream - this should be startxref
	char line[BUFSIZ];
	memset(line, '\0', sizeof(line));
	size_t pos=0;
	
	substream->getLine(line, (int)sizeof(line)-1);
	// first line should be startxref
	substream->getLine(line, sizeof(line)-1);
	// second line should be offset
	 
	// Following line should contain %%EOF string, but only according to 
	// specification reality may be different
	// getLine returns NULL if we are at the end of the stream
	while(substream->getLine(line, sizeof(line)-1))
	{
		// gets position from the beggining of the line 
		size_t tmpPos=substream->getPos();

		if(!strstr(EOFMARKER, line))
		{
			// line contains PDF end of file marker
			pos=tmpPos;
			break;
		}
	}

	// If EOF not found we will use last position of the stream.
	// Substream provides only position relative to its start, so we have 
	// to add startPos
	pos=startPos + (pos)?pos:substream->getPos();
	
	// clean up
	// TODO check if it realy does what it should - XPdf code is unclear
	delete substream;
	obj.free();

	return pos;
}

XRefWriter::XRefWriter(StreamWriter * stream, CPdf * _pdf):CXref(stream->getBaseStream()), mode(paranoid), pdf(_pdf)
{
	// gets storePos
	// searches %%EOF element from startxref position.
	storePos=findPDFEof(stream->getBaseStream(), getStartXref());

	// collects all available revisions
	collectRevisions();
}

bool XRefWriter::paranoidCheck(::Ref ref, ::Object * obj)
{
	bool reinicialization=false;

	printDbg(DBG_DBG, "ref=["<<ref.num<<", "<<ref.gen<<"] type="<<obj->getType());

	if(mode==paranoid)
	{
		printDbg(DBG_DBG, "we are in paranoid mode");
		// reference known test
		if(!knowsRef(ref))
		{
			// if reference is not known, it may be new 
			// which is not changed after initialization
			if(!(reinicialization=newStorage.contains(ref)))
			{
				printDbg(DBG_WARN, "ref=["<<ref.num<<", "<<ref.gen<<"] is unknown");
				return false;
			}
		}
		
		// type safety test only if object has initialized 
		// value already (so new and not changed are not test
		if(!reinicialization)
		{
			// gets original value and uses typeSafe to 
			// compare with given value type
			::Object original;
			fetch(ref.num, ref.gen, &original);
			ObjType originalType=original.getType();
			bool safe=typeSafe(&original, obj);
			original.free();
			if(!safe)
			{
				printDbg(DBG_WARN, "ref=["<<ref.num<<", "<<ref.gen<<"] type="<<obj->getType()
						<<" is not compatible with original type="<<originalType);
				return false;
			}
		}
	}

	return true;
}

void XRefWriter::changeObject(int num, int gen, ::Object * obj)
{
	printDbg(DBG_DBG, "ref=["<< num<<", "<<gen<<"]");

	if(revision)
	{
		// we are in later revision, so no changes can be
		// done
		printDbg(DBG_ERR, "no changes available. revision="<<revision);
		throw ReadOnlyDocumentException("Document is not in latest revision.");
	}
	if(pdf->getMode()==CPdf::ReadOnly)
	{
		// document is in read-only mode
		printDbg(DBG_ERR, "pdf is in read-only mode.");
		throw ReadOnlyDocumentException("Document is in Read-only mode.");
	}

	::Ref ref={num, gen};
	
	// paranoid checking
	if(!paranoidCheck(ref, obj))
	{
		printDbg(DBG_ERR, "paranoid check for ref=["<< num<<", "<<gen<<"] not successful");
		throw ElementBadTypeException("" /* FIXME "[" << num << ", " <<gen <<"]" */);
	}

	// everything ok
	CXref::changeObject(ref, obj);
}

::Object * XRefWriter::changeTrailer(char * name, ::Object * value)
{
	printDbg(DBG_DBG, "name="<<name);
	if(revision)
	{
		// we are in later revision, so no changes can be
		// done
		printDbg(DBG_ERR, "no changes available. revision="<<revision);
		throw ReadOnlyDocumentException("Document is not in latest revision.");
	}
	if(pdf->getMode()==CPdf::ReadOnly)
	{
		// document is in read-only mode
		printDbg(DBG_ERR, "pdf is in read-only mode.");
		throw ReadOnlyDocumentException("Document is in Read-only mode.");
	}
		
	// paranoid checking - can't use paranoidCheck because value may be also
	// direct - we are in trailer
	if(mode==paranoid)
	{
		::Object original;
		
		printDbg(DBG_DBG, "mode=paranoid type safety is checked");
		// gets original value of value
		Dict * dict=trailerDict.getDict();
		dict->lookupNF(name, &original);
		bool safe=typeSafe(&original, value);
		original.free();
		if(!safe)
		{
			printDbg(DBG_ERR, "type safety error");
			throw ElementBadTypeException(name);
		}
	}

	// everything ok
	return CXref::changeTrailer(name, value);
}

::Ref XRefWriter::reserveRef()
{
	printDbg(DBG_DBG, "");

	// checks read-only mode
	
	if(revision)
	{
		// we are in later revision, so no changes can be
		// done
		printDbg(DBG_ERR, "no changes available. revision="<<revision);
		throw ReadOnlyDocumentException("Document is not in latest revision.");
	}
	if(pdf->getMode()==CPdf::ReadOnly)
	{
		// document is in read-only mode
		printDbg(DBG_ERR, "pdf is in read-only mode.");
		throw ReadOnlyDocumentException("Document is in Read-only mode.");
	}

	// changes are availabe
	// delegates to CXref
	return CXref::reserveRef();
}


::Object * XRefWriter::createObject(::ObjType type, ::Ref * ref)
{
	printDbg(DBG_DBG, "type="<<type);

	// checks read-only mode
	
	if(revision)
	{
		// we are in later revision, so no changes can be
		// done
		printDbg(DBG_ERR, "no changes available. revision="<<revision);
		throw ReadOnlyDocumentException("Document is not in latest revision.");
	}
	if(pdf->getMode()==CPdf::ReadOnly)
	{
		// document is in read-only mode
		printDbg(DBG_ERR, "pdf is in read-only mode.");
		throw ReadOnlyDocumentException("Document is in Read-only mode.");
	}

	// changes are availabe
	// delegates to CXref
	return CXref::createObject(type, ref);
}

typedef std::map< ::Ref, size_t, RefComparator> OffsetTab;

/** Builds Xref table from offset table.
 * @param table Offset table containing reference to file offset mapping.
 * @param stream StreamWriter which is used to write data.
 *
 * Constructs (old style) cross reference table from given reference to file
 * offset mapping to the given stream accordinf PDF specification.
 * <br>
 * In first step table is used for cross reference table subsection creation.
 * Then xref keyword is written to the stream.
 * Finally so called SubSectionTab is dumpt to the file. Each subsection is
 * preceeded by start object number and subsection size.
 * <p>
 * <b>PDF specification notes:</b>
 * <br>
 * TODO sections and subsections describtion
 */
void buildXref(OffsetTab & table, StreamWriter & stream)
{
using namespace std;

	printDbg(DBG_DBG, "");
	
	// subsection table type
	// 	- key is object number of subsection leader
	// 	- value is an array of entries (file offset and generation number
	// 	  pairs). This array contains sequence of entries each for one object
	// 	  number. This sequence is without holes (n-th element has key+n object
	// 	  number - n starts from 0)
	typedef pair<size_t, int> EntryType;
	typedef vector<EntryType> EntriesType;
	typedef map< int , EntriesType> SubSectionTab;

	// creates subsection table from offset table:
	// Starts with first element from offset table - inserts num as key and gen
	// and offset as entry pair.
	SubSectionTab subSectionTable;
	SubSectionTab::iterator sub=subSectionTable.begin();
	
	printDbg(DBG_DBG, "Creating subsection table");
	
	// goes through rest entries of offset table
	for(OffsetTab::iterator i=table.begin(); i!=table.end(); i++)
	{
		int num=(i->first).num;
		int gen=(i->first).gen;
		size_t off=i->second;
		
		// skips not assigned subsection
		if(sub!=subSectionTable.end())
		{
			// if num can be added to current sub, appends entries array
			// NOTE: num can be added if sub's key+entries size == num, which 
			// means that entries array won't destry sequence without holes 
			// condition after addition
			if(num == sub->first + (sub->second).size())
			{
				printDbg(DBG_DBG, "Appending num="<<num<<" to section starting with num="<<sub->first);
				(sub->second).push_back(EntryType(off, gen));
				continue;
			}
		}

		// num can't be added, so new subsection has to be created and this is
		// used for next offset table elements
		EntriesType entries;
		entries.push_back(EntryType(off, gen));
		pair<SubSectionTab::iterator, bool> ret=subSectionTable.insert(pair<int, EntriesType>(num, entries));
		printDbg(DBG_DBG, "New subsection created with starting num="<<num);
		sub=ret.first;
	}

	// cross reference table starts with xref row
	// FIXME uncoment when ready and tested
	//stream->putLine("xref");
	printf("xref\n");

	// subsection table is created, we can dump it to the file
	// xrefRow represents one line of xref table which is exactly XREFROWLENGHT
	// lenght
	char xrefRow[XREFROWLENGHT];
	memset(xrefRow, '\0', sizeof(xrefRow));
	for(SubSectionTab::iterator i=subSectionTable.begin(); i!=subSectionTable.end(); i++)
	{
		// at first writes head object number and number of elements in the
		// subsection
		EntriesType & entries=i->second;
		int startNum=i->first;
		printDbg(DBG_DBG, "Starting subsection with startPos="<<startNum<<" and size="<<entries.size());

		snprintf(xrefRow, sizeof(xrefRow)-1, "%d %d", startNum, (int)entries.size());
		// FIXME uncoment when ready and tested
		//stream->putLine(xrefRow);
		// FIXME remove when tested
		printf("\"%s\"\n", xrefRow);

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
			snprintf(xrefRow, sizeof(xrefRow)-1, "%010u %05i n\n", entry->first, entry->second);
			// FIXME uncoment when ready and tested
			//stream->putLine(xrefRow);
			// FIXME remove when tested
			printf("\"%s\"\n", xrefRow);
		}
	}
}

void XRefWriter::saveChanges(bool newRevision)
{
	printDbg(DBG_DBG, "");

	// if changedStorage is empty, there is nothing to do
	if(changedStorage.size()==0)
	{
		printDbg(DBG_INFO, "Nothing to be saved - changedStorage is empty");
		return;
	}
	
	// casts stream (from XRef super type) and casts it to the FileStreamWriter
	// instance - it is ok, because it is initialized with this type of stream
	// in constructor
	StreamWriter * streamWriter=dynamic_cast<StreamWriter *>(XRef::str);

	// sets position to the storePos, where new data can be placed
	streamWriter->setPos(storePos);

	// goes over all objects in changedStorage and stores them to the
	// file and builds table which contains reference to file position mapping.
	OffsetTab offTable;
	ObjectStorage< ::Ref, ObjectEntry*, RefComparator>::Iterator i;
	for(i=changedStorage.begin(); i!=changedStorage.end(); i++)
	{
		::Ref ref=i->first;
		// associate given reference with actual position.
		offTable.insert(OffsetTab::value_type(ref, streamWriter->getPos()));		
		printDbg(DBG_DBG, "Object with ref=["<<ref.num<<", "<<ref.gen<<"] stored at offset="<<streamWriter->getPos());
		
		// stores PDF representation of object to current position which is
		// after moved behind written object
		Object * obj=i->second->object;
		std::string objPdfFormat;
		xpdfObjToString(*obj, objPdfFormat);
		// FIXME uncoment when ready and tested
		//streamWriter->putLine(objPdfFormat.c_str());
		// FIXME remove when tested
		printf("\"%s\"\n", objPdfFormat.c_str());
	}

	// all objects are saved xref table is constructed from offTable
	// New xref section starts is stored to have information for startxref
	// element which is behind trailer
	size_t xrefPos=streamWriter->getPos();
	buildXref(offTable, *streamWriter);
	
	// adds Prev field - to say where starts previous xref table 
	// changeTrailer method is called on CXref because it despite of XRefWriter
	// it doesn't do any checking (we know what we are doing)
	Object prevObj;
	prevObj.initInt(XRef::getLastXRefPos());
	Object * oldPrev=CXref::changeTrailer("Prev", &prevObj);
	if(oldPrev)
	{
		// if oldPrev was not 0, deallocates it
		oldPrev->free();
		delete oldPrev;
	}

	// stores changed trialer to the file
	std::string objPdfFormat;
	xpdfObjToString(trailerDict, objPdfFormat);
	// FIXME uncoment when ready and tested
	//streamWriter->putLine(objPdfFormat.c_str());
	printDbg(DBG_DBG, "Trailer saved");

	// stores offset of last (created one) xref table
	// TODO adds also comment with time and date of creation
	// FIXME uncoment when ready and tested
	//streamWriter->putLine("startxref");
	char xrefPosStr[128];
	// FIXME uncoment when ready and tested
	//streamWriter->putLine(objPdfFormat.c_str());
	// FIXME remove when tested
	printf("\"%s\"\n", objPdfFormat.c_str());
	
	// Finaly puts %%EOF behind but keeps position of marker start
	size_t pos=streamWriter->getPos();
	// FIXME uncoment when ready and tested
	//streamWriter->putLine(EOFMARKER);
	// FIXME remove when tested
	printf("\"%s\"\n", EOFMARKER);
	printDbg(DBG_DBG, "PDF end of file marker saved");
	
	// if new revision should be created, moves storePos at PDF end of file
	// marker position and forces CXref reopen to handle new revision - all
	// chnaged objects are stored in file now.
	if(newRevision)
	{
		printDbg(DBG_INFO, "Saving changes as new revision.");
		storePos=pos;

		// forces CXref::reopen
		CXref::reopen();

		// new revision number is added - we are adding number for new oldest
		// revision
		revisions.push_back(revisions.size());
	}

	printDbg(DBG_DBG, "finished");
}


