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

#include "xrefwriter.h"
#include "cpdf.h"
#include "cobject.h"

using namespace debug;

#ifndef FIRST_LINEARIZED_BLOCK
/** Size of first block of linearized pdf where Linearized dictionary must
 * occure. 
 * 
 * If it value is not defined yet, uses 1024 default value. So it may be
 * specified as compile time define for this module.
 */
#define FIRST_LINEARIZED_BLOCK 1024
#endif

namespace pdfobjects {

namespace utils {

bool checkLinearized(StreamWriter & stream, XRef * xref, Ref * ref)
{
	// searches num gen obj entry. Starts from stream begining
	stream.reset();
	Object obj;
	Parser parser=Parser(
			xref, new Lexer(
				NULL, stream.makeSubStream(stream.getPos(), false, 0, &obj)
				)
			);

	while(stream.getPos() < FIRST_LINEARIZED_BLOCK)
	{
		Object obj1, obj2, obj3;
		parser.getObj(&obj1);
		parser.getObj(&obj2);
		parser.getObj(&obj3);

		// indirect object must start with 
		// num gen obj line
		if(obj1.isInt() && obj2.isInt() && obj3.isCmd())
		{
			Object obj;
			parser.getObj(&obj);

			//  result is false by default - it MUST BE linearized dictionary
			bool result=false;
			
			if(obj.isDict())
			{
				// indirect object is dictionary, so it can be Linearized
				// dictionary
				Object version;
				obj.getDict()->lookupNF("Linearized", &version);
				if(!version.isNull())
				{
					// this is realy Linearized dictionary, so stream contains
					// lienarized pdf content.
					// if ref is not NULL, sets reference of this dictionary
					if(ref)
					{
						ref->num=obj1.getInt();
						ref->gen=obj2.getInt();
					}
					result=true;
				}
			}
			obj.free();

			obj1.free();
			obj2.free();
			obj3.free();
			return result;
		}

		obj1.free();
		obj2.free();
		obj3.free();
	}

	// no indirect object in the document
	return false;
}

} // end of utils namespace

XRefWriter::XRefWriter(StreamWriter * stream, CPdf * _pdf)
	:CXref(stream), 
	mode(paranoid), 
	pdf(_pdf), 
	revision(0), 
	pdfWriter(new utils::OldStylePdfWriter())
{
	// gets storePos
	// searches %%EOF element from startxref position.
	storePos=XRef::eofPos;

	// checks whether file is linearized
	Ref linearizedRef;
	linearized=utils::checkLinearized(*stream, this, &linearizedRef);
	if(linearized)
		kernelPrintDbg(DBG_DBG, "Pdf content is linearized. Linearized dictionary "<<linearizedRef);
	
	// collects all available revisions
	collectRevisions();
}

utils::IPdfWriter * XRefWriter::setPdfWriter(utils::IPdfWriter * writer)
{
using namespace utils;

	IPdfWriter * current=pdfWriter;

	// if given writer is non NULL, sets pdfWriter
	if(writer)
		pdfWriter=writer;

	return current;
}

bool XRefWriter::paranoidCheck(::Ref ref, ::Object * obj)
{
	kernelPrintDbg(DBG_DBG, ref<<" type="<<obj->getType());

	if(mode==paranoid)
	{
		// reference known test
		RefState refState=knowsRef(ref);
		if(refState==UNUSED_REF)
		{
			kernelPrintDbg(DBG_WARN, ref<<" is UNUSED_REF");
			return false;
		}
		
		// type safety test only if object has initialized 
		// value already (so new and not changed are not tested)
		if(refState==INITIALIZED_REF)
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
				kernelPrintDbg(DBG_WARN, ref<<" type="<<obj->getType()
						<<" is not compatible with original type="<<originalType);
				return false;
			}
		}else
			kernelPrintDbg(DBG_DBG, "Reference is not initialized yet. No checking done.");
	}

	kernelPrintDbg(DBG_INFO, "paranoidCheck successfull");
	return true;
}

void XRefWriter::changeObject(int num, int gen, ::Object * obj)
{
	::Ref ref={num, gen};
	kernelPrintDbg(DBG_DBG, ref);

	if(revision)
	{
		// we are in later revision, so no changes can be
		// done
		kernelPrintDbg(DBG_ERR, "no changes available. revision="<<revision);
		throw ReadOnlyDocumentException("Document is not in latest revision.");
	}
	if(pdf && pdf->getMode()==CPdf::ReadOnly)
	{
		// document is in read-only mode
		kernelPrintDbg(DBG_ERR, "pdf is in read-only mode.");
		throw ReadOnlyDocumentException("Document is in Read-only mode.");
	}

	
	// paranoid checking
	if(!paranoidCheck(ref, obj))
	{
		kernelPrintDbg(DBG_ERR, "paranoid check for "<<ref<<" not successful");
		throw ElementBadTypeException("" /* FIXME "[" << num << ", " <<gen <<"]" */);
	}

	// everything ok
	Object * oldValue=CXref::changeObject(ref, obj);
	// deallocates previous changed value (if any)
	if(oldValue)
		utils::freeXpdfObject(oldValue);
}

::Object * XRefWriter::changeTrailer(const char * name, ::Object * value)
{
	kernelPrintDbg(DBG_DBG, "name="<<name);
	if(revision)
	{
		// we are in later revision, so no changes can be
		// done
		kernelPrintDbg(DBG_ERR, "no changes available. revision="<<revision);
		throw ReadOnlyDocumentException("Document is not in latest revision.");
	}
	if(pdf && pdf->getMode()==CPdf::ReadOnly)
	{
		// document is in read-only mode
		kernelPrintDbg(DBG_ERR, "pdf is in read-only mode.");
		throw ReadOnlyDocumentException("Document is in Read-only mode.");
	}
		
	// paranoid checking - can't use paranoidCheck because value may be also
	// direct - we are in trailer
	if(mode==paranoid)
	{
		::Object original;
		
		kernelPrintDbg(DBG_DBG, "mode=paranoid type safety is checked");
		// gets original value of value
		Dict * dict=trailerDict.getDict();
		dict->lookupNF(name, &original);
		bool safe=typeSafe(&original, value);
		original.free();
		if(!safe)
		{
			kernelPrintDbg(DBG_ERR, "type safety error");
			throw ElementBadTypeException(name);
		}
	}

	// everything ok
	return CXref::changeTrailer(name, value);
}

::Ref XRefWriter::reserveRef()
{
	kernelPrintDbg(DBG_DBG, "");

	// checks read-only mode
	
	if(revision)
	{
		// we are in later revision, so no changes can be
		// done
		kernelPrintDbg(DBG_ERR, "no changes available. revision="<<revision);
		throw ReadOnlyDocumentException("Document is not in latest revision.");
	}
	if(pdf && pdf->getMode()==CPdf::ReadOnly)
	{
		// document is in read-only mode
		kernelPrintDbg(DBG_ERR, "pdf is in read-only mode.");
		throw ReadOnlyDocumentException("Document is in Read-only mode.");
	}

	// changes are availabe
	// delegates to CXref
	return CXref::reserveRef();
}


::Object * XRefWriter::createObject(::ObjType type, ::Ref * ref)
{
	kernelPrintDbg(DBG_DBG, "type="<<type);

	// checks read-only mode
	
	if(revision)
	{
		// we are in later revision, so no changes can be
		// done
		kernelPrintDbg(DBG_ERR, "no changes available. revision="<<revision);
		throw ReadOnlyDocumentException("Document is not in latest revision.");
	}
	if(pdf && pdf->getMode()==CPdf::ReadOnly)
	{
		// document is in read-only mode
		kernelPrintDbg(DBG_ERR, "pdf is in read-only mode.");
		throw ReadOnlyDocumentException("Document is in Read-only mode.");
	}

	// changes are availabe
	// delegates to CXref
	return CXref::createObject(type, ref);
}

void XRefWriter::saveChanges(bool newRevision)
{
using namespace utils;

	kernelPrintDbg(DBG_DBG, "");

	if(linearized)
		kernelPrintDbg(DBG_WARN, "Pdf is linearized and changes may break rules for linearization.");

	// if changedStorage is empty, there is nothing to do
	if(changedStorage.size()==0)
	{
		kernelPrintDbg(DBG_INFO, "Nothing to be saved - changedStorage is empty");
		return;
	}
	// checks if we have pdf content writer
	if(!pdfWriter)
	{
		kernelPrintDbg(DBG_ERR, "No pdfWriter defined");
		return;
	}
	
	// casts stream (from XRef super type) and casts it to the FileStreamWriter
	// instance - it is ok, because it is initialized with this type of stream
	// in constructor
	StreamWriter * streamWriter=dynamic_cast<StreamWriter *>(XRef::str);

	// gets vector of all changed objects
	IPdfWriter::ObjectList changed;
	ObjectStorage< ::Ref, ObjectEntry *, RefComparator>::Iterator i;
	for(i=changedStorage.begin(); i!=changedStorage.end(); ++i)
	{
		::Ref ref=i->first;
		Object * obj=i->second->object;
		changed.push_back(IPdfWriter::ObjectElement(ref, obj));
	}

	// delegates writing to pdfWriter using streamWriter stream from storePos
	// position.
	// Stores position of the cross reference section to xrefPos
	pdfWriter->writeContent(changed, *streamWriter, storePos);
	size_t xrefPos=streamWriter->getPos();
	IPdfWriter::PrevSecInfo secInfo={lastXRefPos, XRef::getNumObjects()};
	size_t newEofPos=pdfWriter->writeTrailer(trailerDict, secInfo, *streamWriter);

	// if new revision should be created, moves storePos behind stored content
	// (more preciselly before pdf end of file marker %%EOF) and forces CXref 
	// reopen to handle new revision - all changed objects are stored in file 
	// now.
	if(newRevision)
	{
		kernelPrintDbg(DBG_INFO, "Saving changes as new revision.");
		storePos=newEofPos;
		kernelPrintDbg(DBG_DBG, "New storePos="<<storePos);

		// forces reinitialization of XRef and CXref internal structures from
		// last xref position
		CXref::reopen(xrefPos);

		// new revision number is added - we insert the newest revision so
		// xrefPos value is stored
		revisions.insert(revisions.begin(), xrefPos);
	}

	kernelPrintDbg(DBG_DBG, "finished");
}

void XRefWriter::collectRevisions()
{
	kernelPrintDbg(DBG_DBG, "");

	// starts with newest revision
	size_t off=XRef::lastXRefPos;

	// linearized pdf doesn't support multiversion document clearly, so we don't
	// implement collecting for such documents
	if(isLinearized())
	{
		// creates just one revision information with the newest one
		revisions.push_back(off);
		kernelPrintDbg(DBG_WARN, "collectRevisions not implemented for linearized pdf");
		return;
	}

	// clears revisions if non empty
	if(revisions.size())
	{
		kernelPrintDbg(DBG_DBG, "Clearing revisions container.");
		revisions.clear();
	}

	// uses deep copy to prevent problems with original data
	Object * trailer=XRef::trailerDict.clone();
	if(!trailer)
	{
		kernelPrintDbg(DBG_ERR, "Unable to clone trailer. Ignoring revision collecting.");
		return;
	}
	bool cont=true;

	// TODO code clean up
	do
	{
		kernelPrintDbg(DBG_DBG, "XRef offset for "<<revisions.size()<<" revision is "<<off);
		// pushes current offset as last revision
		revisions.push_back(off);

		// gets prev field from current trailer and if it is null object (not
		// present) or doesn't have integer value, jumps out of loop
		Object prev;
		trailer->getDict()->lookupNF("Prev", &prev);
		if(prev.getType()==objNull)
		{
			// objNull doesn't need free
			kernelPrintDbg(DBG_DBG, "No previous revision.");
			break;
		}
		if(prev.getType()!=objInt)
		{
			kernelPrintDbg(DBG_DBG, "Prev doesn't have int value. type="<<prev.getType()<<". Assuming no more revisions.");
			prev.free();
			break;
		}

		// checks whether given offeset already is in revisions, which would
		// mean corrupted referencies (because of cycle in trailers)
		if(std::find(revisions.begin(), revisions.end(), (size_t)prev.getInt())!=revisions.end())
		{
			kernelPrintDbg(DBG_ERR, "Trailer Prev points to already processed revision (endless loop). Assuming no more revisions.");
			break;
		}

		// sets new value for off
		off=prev.getInt();
		prev.free();

		// off is first byte of cross reference section. It can be either xref
		// key word or beginning of xref stream object
		str->setPos(off);
		Object parseObj, obj;
		::Parser parser = Parser(this,
			new Lexer(NULL, str->makeSubStream(str->getPos(), gFalse, 0, &parseObj))
			);
		parser.getObj(&obj);
		if(obj.isCmd(XREF_KEYWORD))
		{
			// old style cross reference table, we have to skip whole table and
			// then we will get to trailer
			obj.free();
			kernelPrintDbg(DBG_INFO, "New old style cross reference section found. off="<<off);
			
			// searches for TRAILER_KEYWORD to be able to parse older trailer (one
			// for xref on off position) - this works only for oldstyle XRef tables
			// not XRef streams
			char * ret; 
			char buffer[1024];
			memset(buffer, '\0', sizeof(buffer));
			// resets position in the stream because parser moves with position
			str->setPos(off);
			while((ret=str->getLine(buffer, sizeof(buffer)-1)))
			{
				if(strstr(buffer, STARTXREF_KEYWORD))
				{
					// we have reached startxref keyword and haven't found trailer
					kernelPrintDbg(DBG_ERR, STARTXREF_KEYWORD<<" found but no trailer.");
					goto xreferror;
				}
				
				if(strstr(buffer, TRAILER_KEYWORD))
				{
					// trailer found, parse it and set trailer to parsed one
					kernelPrintDbg(DBG_DBG, "Trailer dictionary found");
					
					// we have to create new parser because we can't set
					// position in parser to current in the stream
					::Parser parser = Parser(this,
						new Lexer(NULL, str->makeSubStream(str->getPos(), gFalse, 0, &parseObj))
						);

					// deallocates previous one before it is filled by new one
					trailer->free();
					parser.getObj(trailer);
					if(!trailer->isDict())
					{
						kernelPrintDbg(DBG_ERR, "Trailer is not dictionary.");
						goto xreferror;
					}

					// everything ok, trailer is read and parsed and can be used
					break;
				}
			}
			if(!ret)
			{
				// stream returned NULL, which means that no more data in stream is
				// available
				kernelPrintDbg(DBG_DBG, "end of stream but no trailer found");
				cont=false;
			}
			continue;
		}

		// xref key work no found, it may be xref stream object
		if(obj.isInt())
		{
			// gen number should follow
			obj.free();
			parser.getObj(&obj);
			if(!obj.isInt())
			{
				obj.free();
				goto xreferror;
			}
			
			// end of indirect object header is obj key word
			parser.getObj(&obj);
			if(!obj.isCmd("obj"))
			{
				obj.free();
				goto xreferror;
			}

			// header of indirect object is ok, so parser object itself
			Object trailerStream;
			parser.getObj(&trailerStream);
			if(!trailerStream.isStream())
			{
				trailerStream.free();
				goto xreferror;
			}
			trailerStream.getDict()->lookupNF("Type", &obj);
			if(!obj.dictIs("XRef"))
			{
				obj.free();
				trailerStream.free();
				goto xreferror;
			}
			
			// xref stream object contains also trailer information, so parses
			// stream object and uses just dictionary part
			kernelPrintDbg(DBG_INFO, "New xref stream section. off="<<off);
			trailer->free();
			trailer->initDict(trailerStream.getDict());
			continue;
		}

xreferror:
		kernelPrintDbg(DBG_ERR, "Xref section offset doesn't point to xref start");
		cont=false;

	// continues only if no problem with trailer occures
	}while(cont); 

	// deallocates the oldest one - no problem if the oldest is the newest at
	// the same time, because we have used clone of trailer instance from XRef
	trailer->free();
	gfree(trailer);

	kernelPrintDbg(DBG_INFO, "This document contains "<<revisions.size()<<" revisions.");
}

void XRefWriter::changeRevision(unsigned revNumber)
{
	kernelPrintDbg(DBG_DBG, "revNumber="<<revNumber);
	
	// change to same revision
	if(revNumber==revision)
	{
		// nothing to do, we are already here
		kernelPrintDbg(DBG_INFO, "Revision changed to "<<revNumber);
		return;
	}

	if(isLinearized())
	{
		kernelPrintDbg(DBG_WARN, "Document is lenearized and changeRevision is not implemented.");
		throw NotImplementedException("changeRevision is not implemented fro linearizes pdf.");
	}
	
	// constrains check
	if(revNumber>revisions.size()-1)
	{
		kernelPrintDbg(DBG_ERR, "unkown revision with number="<<revNumber);
		throw OutOfRange();
	}
	
	// forces CXRef to reopen from revisions[revNumber] offset
	// which points to start of xref section for that revision
	size_t off=revisions[revNumber];
	reopen(off);

	// everything ok, so current revision can be set
	revision=revNumber;
	kernelPrintDbg(DBG_INFO, "Revision changed to "<<revision);
}

size_t XRefWriter::getRevisionEnd(size_t xrefStart)const
{
	StreamWriter * streamWriter=dynamic_cast<StreamWriter *>(str);
	size_t pos=streamWriter->getPos();

	// starts from given position
	streamWriter->setPos(xrefStart);
	char buffer[BUFSIZ];
	memset(buffer, '\0', sizeof(buffer));
	while(streamWriter->getLine(buffer, sizeof(buffer)))
	{
		if(!strncmp(buffer, STARTXREF_KEYWORD, strlen(STARTXREF_KEYWORD)))
		{
			// we have found start-xref key word, next line should contain
			// value of offset - this information is not important, we just have
			// to get behind and calculates number of bytes
			streamWriter->getLine(buffer, sizeof(buffer));
			break;
		}
	}

	// returns current position
	size_t endPos=streamWriter->getPos();
	
	// restores position in the stream
	streamWriter->setPos(pos);

	return endPos;
}

void XRefWriter::cloneRevision(FILE * file)const
{
using namespace debug;

	kernelPrintDbg(DBG_ERR, "");

	StreamWriter * streamWriter=dynamic_cast<StreamWriter *>(str);
	size_t pos=streamWriter->getPos();

	// gets current revision end
	size_t revisionEOF=getRevisionEnd(revisions[revision]);

	kernelPrintDbg(DBG_DBG, "Copies until "<<revisionEOF<<" offset");
	streamWriter->clone(file, 0, revisionEOF);

	// adds pdf end of line marker to the output file
	fwrite(EOFMARKER, sizeof(char), strlen(EOFMARKER), file);
	fflush(file);

	// restore stream position
	streamWriter->setPos(pos);
}

size_t XRefWriter::getRevisionSize(unsigned rev, bool includeXref)const
{
using namespace debug;

	kernelPrintDbg(DBG_DBG, "rev="<<rev<<" includeXref="<<includeXref);

	// gets starting position of current rev
	size_t revStart=revisions[rev];
	size_t prevEnd=0;

	// gets end of previous rev - if no exists keeps default value 0 (from
	// the stream beginning)
	if(rev+1<getRevisionCount())
	{
		prevEnd=getRevisionEnd(revisions[rev+1]);
		kernelPrintDbg(DBG_DBG, "Previous revision ends at "<<prevEnd);
	}else
		kernelPrintDbg(DBG_DBG, "No previous rev.");

	// if also xref section should be considered, gets end of current rev
	if(includeXref)
	{
		revStart=getRevisionEnd(revisions[rev]);
		kernelPrintDbg(DBG_DBG, "Considering also xref section. Revision ends at "<<revStart);
	}

	assert(revStart>prevEnd);

	// returns the difference
	return revStart-prevEnd;
}

} // end of pdfobjects namespace
