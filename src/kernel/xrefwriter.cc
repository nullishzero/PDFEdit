// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80

/*
 * $RCSfile$
 *
 * $Log$
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

/** Searches for %%EOF PDF end of file.
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

		if(!strstr("%%%%EOF", line))
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

XRefWriter::XRefWriter(StreamWriter * stream):CXref(stream->getBaseStream()), mode(paranoid)
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

void XRefWriter::releaseObject(int num, int gen)
{
	printDbg(DBG_DBG, "num="<<num<<" gen="<<gen);
	
	if(revision)
	{
		// we are in later revision, so no changes can be
		// done
		printDbg(DBG_ERR, "no changes available. revision="<<revision);
		throw ReadOnlyDocumentException("Document is not in latest revision.");
	}
	if(mode==readOnly)
	{
		// we are in read-only mode, no changes can be done
		printDbg(DBG_ERR, "no changes availabe, mode=readOnly");
		throw ReadOnlyDocumentException("Document is in readOnly mode.");
	}
		
	::Ref ref={num, gen};
	// checks if reference exists
	// FIXME really only in paranoid mode?
	if(mode==paranoid && !knowsRef(ref))
	{
		printDbg(DBG_ERR, "ref["<<ref.num<<", "<<ref.gen<<"] not found");
		throw IndirectObjectNotFoundException(ref.num, ref.gen);
	}

	// delegates to CXref
	CXref::releaseObject(ref);
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
	if(mode==readOnly)
	{
		// we are in read-only mode, no changes can be done
		printDbg(DBG_ERR, "no changes availabe, mode=readOnly");
		throw ReadOnlyDocumentException("Document is in readOnly mode.");
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
	if(mode==readOnly)
	{
		// we are in read-only mode, no changes can be done
		printDbg(DBG_ERR, "no changes availabe, mode=readOnly");
		throw ReadOnlyDocumentException("Document is in readOnly mode.");
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
	if(mode==readOnly)
	{
		// we are in read-only mode, no changes can be done
		printDbg(DBG_ERR, "no changes availabe, mode=readOnly");
		throw ReadOnlyDocumentException("Document is in readOnly mode.");
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
	if(mode==readOnly)
	{
		// we are in read-only mode, no changes can be done
		printDbg(DBG_ERR, "no changes availabe, mode=readOnly");
		throw ReadOnlyDocumentException("Document is in readOnly mode.");
	}

	// changes are availabe
	// delegates to CXref
	return CXref::createObject(type, ref);
}

void XRefWriter::saveChanges(bool newRevision)
{
	// FIXME remove when implementation is ready
	throw NotImplementedException("XRefWriter::saveChanges");

	printDbg(DBG_DBG, "");

	// casts stream (from XRef super type) and casts it to the FileStreamWriter
	// instance - it is ok, because it is initialized with this type of stream
	// in constructor
	StreamWriter * streamWriter=dynamic_cast<StreamWriter *>(XRef::str);

	// sets position to the storePos, where new data can be placed
	streamWriter->setPos(storePos);

	// goes over all objects in changedStorage and stores them to the end of 
	// file and builds table which contains reference to file position mapping.
	typedef std::map< ::Ref, size_t, RefComparator> OffsetTab;
	OffsetTab offTable;
	ObjectStorage< ::Ref, ObjectEntry*, RefComparator>::Iterator i;
	for(i=changedStorage.begin(); i!=changedStorage.end(); i++)
	{
		// associate given reference with actual position.
		offTable.insert(OffsetTab::value_type(i->first, streamWriter->getPos()));		

		// stores PDF representation of object to current position which is
		// after moved behind written object
		Object * obj=i->second->object;
		std::string objPdfFormat;
		xpdfObjToString(*obj, objPdfFormat);
		streamWriter->putLine(objPdfFormat.c_str());
	}

	// all objects are saved xref table is constructed from offTable
	// TODO - how to build table if it is stream table
	
	// stores trailer 
	
	// puts %%EOF behind but keeps position of marker start
	size_t pos=streamWriter->getPos();
	streamWriter->putLine("%%%%EOF");
	
	// if new revision should be created, moves storePos at PDF end of file
	// marker position and forces CXref reopen to handle new revision - all
	// chnaged objects are stored in file now.
	if(newRevision)
	{
		storePos=pos;

		// forces CXref::reopen
		CXref::reopen();

		// new revision number is added - we are adding number for new oldest
		// revision
		revisions.push_back(revisions.size());
	}
}


