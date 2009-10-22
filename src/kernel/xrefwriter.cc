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

#include "kernel/static.h"
#include "kernel/xrefwriter.h"
#include "kernel/cpdf.h"
#include "kernel/cxref.h"
#include "kernel/streamwriter.h"
#include "kernel/pdfwriter.h"
#include "kernel/factories.h"

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

bool checkLinearized(StreamWriter & stream, CXref * xref, Ref * ref)
{
	// searches num gen obj entry. Starts from stream begining
	stream.reset();
	Object obj;
	Stream * subStr = stream.makeSubStream(stream.getPos(), false, 0, &obj);
	Parser parser=Parser(xref, new Lexer(NULL, subStr), gTrue);
	
	//  result is false by default - it MUST BE linearized dictionary
	bool result = false;

	// TODO rewrite to use shared_ptr for obj1, obj2, obj3 
	while(subStr->getPos() < FIRST_LINEARIZED_BLOCK)
	{
		Object obj1, obj2, obj3;
		if(!parser.getObj(&obj1))
			goto malformedErr;
		if(obj1.isEOF())
			break;
		if(!parser.getObj(&obj2))
		{
			obj1.free();
			goto malformedErr;
		}
		if(obj2.isEOF())
		{
			obj1.free();
			break;
		}
		if(!parser.getObj(&obj3))
		{
			obj1.free();
			obj2.free();
			goto malformedErr;
		}
		if(obj3.isEOF())
		{
			obj1.free();
			obj2.free();
			break;
		}

		// indirect object must start with 
		// num gen obj line
		if(obj1.isInt() && obj2.isInt() && obj3.isCmd())
		{
			Object obj;
			if(!parser.getObj(&obj))
				goto malformedErr;
 	
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
			}else 
				if(obj.isEOF())
					break;
			obj.free();
		}

		obj1.free();
		obj2.free();
		obj3.free();
		if(result)
			break;
	}

	// no indirect object in the document
	return result;

malformedErr:
	utilsPrintDbg(DBG_ERR, "Unable to parse stream - bad content.");
	throw MalformedFormatExeption("bad data stream");
}


bool isLatestRevision(const XRefWriter &xref)
{
	// The most recent revision has the highest number
	return xref.getActualRevision() == xref.getRevisionCount()-1;
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

	// enables internal fetch for case of encrypted document.
	// Fetching is ok in such a case because we fetching only
	// uncrypted data - trailer and check for linearized dictionary
	enableInternalFetch();

	// check for linearized document is safe also for encrypted
	// documents because only strings are encrypted and the
	// Linearized entry is the name object
	Ref linearizedRef;
	linearized=utils::checkLinearized(*stream, this, &linearizedRef);
	if(linearized)
		kernelPrintDbg(DBG_DBG, "Pdf content is linearized. Linearized dictionary "<<linearizedRef);

	// revisions can be collected also for encrypted documents, because
	// we are parsing only trailer which doesn't contain any directly
	// encrypted data - strings
	// revision is initialized to the most recent one
	collectRevisions();

	// sets internal fetch back to normal
	disableInternalFetch();
}

XRefWriter::~XRefWriter()
{
	kernelPrintDbg(debug::DBG_DBG, "");
	if(pdfWriter)
		delete pdfWriter;
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

bool XRefWriter::paranoidCheck(const ::Ref &ref, ::Object * obj)
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
			boost::shared_ptr< ::Object> original(XPdfObjectFactory::getInstance(), xpdf::object_deleter());
			fetch(ref.num, ref.gen, original.get());
			ObjType originalType=original->getType();
			bool safe=typeSafe(original.get(), obj);
			if(!safe)
			{
				kernelPrintDbg(DBG_WARN, ref<<" type="<<obj->getType()
						<<" is not compatible with original type="<<originalType);
				return false;
			}
		}else
			kernelPrintDbg(DBG_DBG, "Reference is not initialized yet. No checking done.");
	}

	kernelPrintDbg(DBG_DBG, "paranoidCheck successfull");
	return true;
}

void XRefWriter::changeObject(int num, int gen, ::Object * obj)
{
	::Ref ref={num, gen};
	kernelPrintDbg(DBG_DBG, ref);

	check_need_credentials(this);

	if(!utils::isLatestRevision(*this))
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
	if(isEncrypted())
	{
		kernelPrintDbg(DBG_ERR, "Document is encrypted. Changing content is not supported");
		throw NotImplementedException("changeObject is not implemented for encryted document");
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
		xpdf::freeXpdfObject(oldValue);
}

namespace utils {
bool canChangeTrailerEntry(const char * name)
{
	/* Trailer entries which cannot be changed by changeTrailer method.
	 * This check is not done for easy mode where you are allowed to
	 * to do anything you want. Please note that some of the fields changing
	 * will not have any effect because they are are generated automatically
	 * during save (e.g. Prev and Size).
	 */
	static const char *bannedTrailerEntries[] = {"Prev", "Size", "Encrypt", NULL};

	for(int i=0; bannedTrailerEntries[i]; ++i)
		if(!strcmp(name, bannedTrailerEntries[i]))
		{
			kernelPrintDbg(DBG_WARN, name<<" field is not editable in Trailer");
			return false;
		}
	return true;
}

bool typeSafeTrailerEntry(const char *name, ::Object &value, XRef &xref)
{
	// all values have to be checked according to the specification
	if(!strcmp(name, "Prev") || !strcmp(name, "Size"))	// Direct integer values
	{
		if(value.getType() != objInt)
		{
			kernelPrintDbg(DBG_ERR, name<<" cannot be assigned to "<<value.getType());
			return false;
		}
	}else if(!strcmp(name, "Root") || !strcmp(name, "Info")) // Indirect reference to dictionary
	{
		if(value.getType() != objRef)
		{
			kernelPrintDbg(DBG_ERR, name<<" cannot be assigned to "<<value.getType());
			return false;
		}
		boost::shared_ptr<Object> o(XPdfObjectFactory::getInstance(), xpdf::object_deleter());
		xref.fetch(value.getRefNum(), value.getRefGen(), o.get());
		if(o->getType() != objDict)
		{
			kernelPrintDbg(DBG_ERR, name<<" cannot be assigned to "<<o->getType());
			return false;
		}
	}else if(!strcmp(name, "ID"))	// direct array
	{
		if(value.getType() != objArray)
		{
			kernelPrintDbg(DBG_ERR, name<<" cannot be assigned to "<<value.getType());
			return false;
		}
		// TODO maybe we should be more pedantic and check also
		// the size and types of elements (2 strings)
	}else if(!strcmp(name, "Encrypt"))	// direct array
	{
		boost::shared_ptr<Object> o(XPdfObjectFactory::getInstance(), xpdf::object_deleter());
		value.fetch(&xref, o.get());
		if(o->getType() != objDict)
		{
			kernelPrintDbg(DBG_ERR, name<<" cannot be assigned to "<<o->getType());
			return false;
		}

	}

	// All other 

	return true;
}

}

::Object * XRefWriter::changeTrailer(const char * name, ::Object * value)
{
	if (!name || !value)
	{
		kernelPrintDbg(DBG_ERR, "Invalid parameters. name="<<((name)?name:"NULL")
				<<" value="<<(const void*)value);
		throw ElementBadTypeException("");
	}
	kernelPrintDbg(DBG_DBG, "name="<<name);

	check_need_credentials(this);

	if(isEncrypted())
	{
		kernelPrintDbg(DBG_ERR, "Document is encrypted. Changing content is not supported");
		throw NotImplementedException("changeObject is not implemented for encryted document");
	}

	if(!utils::isLatestRevision(*this))
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
		kernelPrintDbg(DBG_DBG, "mode=paranoid type safety is checked");
 
 		if(!utils::canChangeTrailerEntry(name))
 		{
 			throw ElementBadTypeException("banned field");
 		}
 
 		if(!utils::typeSafeTrailerEntry(name, *value, *this))
  		{
  			throw ElementBadTypeException(name);
  		}
	}

	// everything ok
	return CXref::changeTrailer(name, value);
}

RefState XRefWriter::knowsRef(const IndiRef& ref)const
{
	::Ref xpdfRef={ref.num, ref.gen};
	// otherwise use XRef directly
	return knowsRef(xpdfRef);
}

::Ref XRefWriter::reserveRef()
{
	kernelPrintDbg(DBG_DBG, "");

	check_need_credentials(this);

	// checks read-only mode
	
	if(!utils::isLatestRevision(*this))
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

	check_need_credentials(this);

	// checks read-only mode
	
	if(!utils::isLatestRevision(*this))
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

	if(isEncrypted())
	{
		kernelPrintDbg(DBG_ERR, "Document is encrypted. Changing content is not supported");
		throw NotImplementedException("changeObject is not implemented for encryted document");
	}

	// changes are availabe
	// delegates to CXref
	return CXref::createObject(type, ref);
}

void XRefWriter::saveChanges(bool newRevision)
{
	using namespace utils;

	kernelPrintDbg(DBG_DBG, "");

	check_need_credentials(this);

	if(linearized)
		kernelPrintDbg(DBG_WARN, "Pdf is linearized and changes may break rules for linearization.");

	// if changedStorage is empty, there is nothing to do
	if(changedStorage.size()==0)
	{
		kernelPrintDbg(DBG_DBG, "Nothing to be saved - changedStorage is empty");
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
	ChangedStorage::Iterator i;
	for(i=changedStorage.begin(); i!=changedStorage.end(); ++i)
	{
		::Ref ref=i->first;
		Object * obj=i->second->object;
		// for sake of paranoia we should send clones and not the
		// object itself to writer which is allowed to alter object
		changed.push_back(IPdfWriter::ObjectElement(ref, obj->clone()));
	}

	// delegates writing to pdfWriter using streamWriter stream from storePos
	// position and frees all clones from changed storage.
	pdfWriter->writeContent(changed, *streamWriter, storePos);
	for(IPdfWriter::ObjectList::iterator i=changed.begin(); i!=changed.end(); ++i){
		Object *o = i->second;
		xpdf::freeXpdfObject(o);
	}

	// Stores position of the cross reference section to xrefPos
	size_t xrefPos=streamWriter->getPos();
	IPdfWriter::PrevSecInfo secInfo={lastXRefPos, XRef::maxObj+1};
	size_t newEofPos=pdfWriter->writeTrailer(*getTrailerDict(), secInfo, *streamWriter);

	// if new revision should be created, moves storePos behind stored content
	// (more preciselly before pdf end of file marker %%EOF) and forces CXref 
	// reopen to handle new revision - all changed objects are stored in file 
	// now.
	if(newRevision)
	{
		kernelPrintDbg(DBG_INFO, "Saving changes as new revision number "
				<<revisions.size()+1);
		storePos=newEofPos;
		kernelPrintDbg(DBG_DBG, "New storePos="<<storePos);

		// forces reinitialization of XRef and CXref internal structures from
		// last xref position
		CXref::reopen(xrefPos);

		// new revision number is added and current revision is updated - 
		// we insert the newest revision so xrefPos value is stored
		revisions.push_back(xrefPos);
		revision = revisions.size()-1;
	}

	kernelPrintDbg(DBG_DBG, "finished");
}

#define ERR_OFFSET (size_t)(-1UL)
#define isERR_OFFSET(value) (value == ERR_OFFSET)

/** Gets value of the Prev field from given trailer.
 * @param trailer Trailer stream or dictionary.
 *
 * This method correctly distinguishes between trailer dictionary and stream.
 * @return Value of the Prev field or -1 on error (Prev not present or invalid
 * value).
 */
size_t getPrevFromTrailer(Object * trailer)
{
	const Dict * trailerDict = NULL;
	if(trailer->isDict())
		trailerDict = trailer->getDict();
	else if(trailer->isStream())
		trailerDict = trailer->getStream()->getDict();
	else
	{
		kernelPrintDbg(DBG_ERR, "bad trailer type (type="<<
				trailer->getType()<<")");
		return ERR_OFFSET;
	}

	// gets prev field from current trailer and if it is null object (not
	// present) or doesn't have integer value, jumps out of loop
	boost::shared_ptr< ::Object> prev(XPdfObjectFactory::getInstance(), xpdf::object_deleter());
	trailerDict->lookupNF("Prev", prev.get());
	if(prev->getType()!=objInt)
	{
		kernelPrintDbg(DBG_DBG, "Prev doesn't have int value. type="
				<<prev->getType()<<
				". Assuming no more revisions.");
		return ERR_OFFSET;
	}

	// releases prev because we don't need it anymore
	int value=prev->getInt();
	if(value<0)
		return ERR_OFFSET;
	return value;
}

int XRefWriter::getOldStyleTrailer(Object * trailer, size_t off)
{
	// old style cross reference table, we have to skip whole table and
	// then we will get to trailer
	kernelPrintDbg(DBG_DBG, "New old style cross reference section found. off="<<off);
	
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
			return -1;
		}
		
		if(strstr(buffer, TRAILER_KEYWORD))
		{
			// trailer found, parse it and set trailer to parsed one
			kernelPrintDbg(DBG_DBG, "Trailer dictionary found");
			
			// we have to create new parser because we can't set
			// position in parser to current in the stream
			// TODO: can we reuse parser from collectRevisions here?
			Object parseObj;
			::Parser parser = Parser(this,
				new Lexer(NULL, str->makeSubStream(str->getPos(), gFalse, 0, &parseObj)),
				gTrue
				);

			// deallocates previous one before it is filled by new one
			trailer->free();
			if(!parser.getObj(trailer))
			{
				kernelPrintDbg(DBG_ERR, "Unable to parse trailer");
				return -1;
			}
			if(!trailer->isDict())
			{
				kernelPrintDbg(DBG_ERR, "Trailer is not dictionary. (type="
						<<trailer->getType() << ")");
				trailer->free();
				return -1;
			}

			// everything ok, trailer is read and parsed and can be used
			return 0;
		}
	}
	// stream returned NULL, which means that no more data in stream is
	// available
	assert(!ret);
	kernelPrintDbg(DBG_DBG, "end of stream but no trailer found");
	return -1;
}

// TODO is this ok for encrypted documents (streams should be encrypted there -
// but how is this handled in trailer)?
int XRefWriter::getStreamTrailer(Object * trailer, size_t off, ::Parser & parser)
{
	// gen number should follow
	boost::shared_ptr< ::Object> obj(XPdfObjectFactory::getInstance(), xpdf::object_deleter());
	if(!parser.getObj(obj.get()))
		goto malformedErr;
	if(!obj->isInt())
		goto bad_header;
	if(!parser.getObj(obj.get()))
		goto malformedErr;
	if(!obj->isCmd("obj"))
		goto bad_header;

	// header of indirect object is ok, so parse trailer object
	trailer->free();
	if(!parser.getObj(trailer))
		goto malformedErr;
	if(!trailer->isStream())
	{
		kernelPrintDbg(DBG_ERR, "Trailer is supposed to be stream, but "
				<<trailer->getType()<<" found.");
		return -1;
	}
	
	// xref stream object contains also trailer information, so parses
	// stream object and uses just dictionary part
	kernelPrintDbg(DBG_DBG, "New xref stream section. off="<<off);
	return 0;
malformedErr:
	kernelPrintDbg(DBG_ERR, "Unable to parse trailer");
	return -1;
bad_header:
	kernelPrintDbg(DBG_ERR, "Trailer header corrupted.");
	return -1;
}

void XRefWriter::collectRevisions()
{
	kernelPrintDbg(DBG_DBG, "");

	// starts with newest revision
	size_t off=XRef::lastXRefPos;

	// linearized pdf doesn't support multiversion document clearly, so we don't
	// implement collecting for such documents
	// FIXME we might not have linearized flag set at the moment, because
	// checkLinearized is not called for encrypted documents without credentials
	// available. How to handle properly?
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
	Object * trailer = XRef::getTrailerDict()->clone();
	if(!trailer)
	{
		kernelPrintDbg(DBG_ERR, "Unable to clone trailer. Ignoring revision collecting.");
		return;
	}

	bool hybrid_xref = false;
	do
	{
		// Take care about hybrid files which makes situation little bit more 
		// complex, because these documents usually contain 2 sections to 
		// describe one revision. The last (most recent one) has xref with no
		// entries and trailer has /Prev pointing to the xref table and XRefStm
		// pointing to the xref stream (PDFReference16.pdf: Chapter 3, 
		// Compatibility with PDF 1.4). The pervious contains referred xrefs.
		// This, however, doesn't mean that the document has 2 revisions so
		// don't add them in such a case.
		if (!hybrid_xref)
		{
			// process current trailer (store offset and get previous
			// if present) - we are starting with the newest one cloned
			// above
			kernelPrintDbg(DBG_DBG, "XRef offset for "<<
					revisions.size()<<" revision is "<<off);

			revisions.insert(revisions.begin(), off);
		}
		off = getPrevFromTrailer(trailer);
		if(isERR_OFFSET(off))
			// no more previous trailers
			break;
		
		hybrid_xref = false;
		Object stm;
		trailer->dictLookupNF("XRefStm", &stm);
		if (stm.isInt())
		{
			kernelPrintDbg(DBG_INFO, "Document contains hybrid-file XREF.");
			hybrid_xref = true;
		}else if (!stm.isNull())
		{
			kernelPrintDbg(DBG_WARN, "Unexpected value for XRefStm trailer entry with type "
					<< stm.getType());
		}
		stm.free();

		// checks whether given offset is already in revisions, which 
		// would mean corrupted referencies (because of cycle in 
		// trailers)
		if(!hybrid_xref && std::find(revisions.begin(), revisions.end(), 
					(size_t)off) != revisions.end())
		{
			kernelPrintDbg(DBG_ERR, "Trailer Prev points to already"
				       "processed revision (endless loop). "
				       "Assuming no more revisions.");
			break;
		}

		// off is the first byte of a cross reference section. 
		// It can be either xref key word or beginning of the 
		// xref stream object - we have to process it even for
		// hybrid_xref case becase this can contain reference to
		// the previous revision
		str->setPos(off);
		Object parseObj; 
		boost::shared_ptr< ::Object> obj(XPdfObjectFactory::getInstance(), xpdf::object_deleter());
		::Parser parser = Parser(this,
			new Lexer(NULL, str->makeSubStream(str->getPos(), gFalse, 0, &parseObj)),
			gTrue
			);
		if(!parser.getObj(obj.get()))
		{
			kernelPrintDbg(DBG_ERR, "Unable to parse stream");
			throw MalformedFormatExeption("parse data stream");
		}
		if(obj->isCmd(XREF_KEYWORD))
		{
			if(getOldStyleTrailer(trailer, off)<0)
				break;
		}else if(obj->isInt())
		{
			// xref key work no found, it may be xref stream object
			// gen number should follow
			if(getStreamTrailer(trailer, off, parser))
				break;
		}else {
			kernelPrintDbg(DBG_ERR, "Bad trailer definition. Dictionary or stream"
					"expected, but "<<obj->getType()<<"found");
			break;
		}
	// continues only if no problem with trailer occures
	}while(true); 

	// deallocates the oldest one - no problem if the oldest is the newest at
	// the same time, because we have used clone of trailer instance from XRef
	trailer->free();
	gfree(trailer);

	// initiailizes the current revision to the most recent one.
	revision = revisions.size()-1;
	kernelPrintDbg(DBG_INFO, "This document contains "<<revisions.size()<<" revisions.");
}

void XRefWriter::changeRevision(unsigned revNumber)
{
	kernelPrintDbg(DBG_DBG, "revNumber="<<revNumber);

	check_need_credentials(this);
	
	// change to same revision
	if(revNumber==revision)
	{
		// nothing to do, we are already here
		kernelPrintDbg(DBG_INFO, "Revision kept at "<<revNumber);
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
	// and forces keeping all changes
	size_t off=revisions[revNumber];
	reopen(off, false);

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

	kernelPrintDbg(DBG_DBG, "");

	check_need_credentials(this);

	if(isEncrypted())
	{
		kernelPrintDbg(debug::DBG_WARN, "Operation is not supported for encrypted documents");
		throw NotImplementedException("cloneRevision");
	}

	StreamWriter * streamWriter=dynamic_cast<StreamWriter *>(str);
	size_t pos=streamWriter->getPos();

	// gets current revision end
	size_t revisionEOF=getRevisionEnd(revisions[revision]);

	kernelPrintDbg(DBG_DBG, "Copies until "<<revisionEOF<<" offset");
	streamWriter->cloneToFile(file, 0, revisionEOF);

	// adds pdf end of line marker to the output file
	size_t marker_len = strlen(EOFMARKER);
	if(marker_len > fwrite(EOFMARKER, sizeof(char), marker_len, file))
	{
		int err = errno;
		kernelPrintDbg(DBG_ERR, "Unable to write whole EOFMARKER (\"" << 
				strerror(err) << "\").");
	}
	fflush(file);

	// restore stream position
	streamWriter->setPos(pos);
}

size_t XRefWriter::getRevisionSize(unsigned rev, bool includeXref)const
{
using namespace debug;

	kernelPrintDbg(DBG_DBG, "rev="<<rev<<" includeXref="<<includeXref);

	// constrains check
	if(rev>revisions.size()-1)
	{
		kernelPrintDbg(DBG_ERR, "unkown revision with number="<<rev);
		throw OutOfRange();
	}
	
	// gets starting position of current rev
	size_t revStart=revisions[rev];
	size_t prevEnd=0;

	// gets the end of the previous rev - keeps default value 0 (from
	// the stream beginning) if there is no other revision
	if(rev>0)
	{
		prevEnd=getRevisionEnd(revisions[rev-1]);
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
