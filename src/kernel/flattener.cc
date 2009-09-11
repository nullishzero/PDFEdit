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
#include "kernel/static.h" // WIN32 port - precompiled headers - REMOVE IN FUTURE!
#include <errno.h>
#include "kernel/flattener.h"
#include "utils/debug.h"
#include "kernel/streamwriter.h"
#include "kernel/factories.h"
#include "kernel/pdfedit-core-dev.h"

using namespace pdfobjects;
using namespace utils;

Flattener::Flattener(FileStreamData &streamData, IPdfWriter * writer)
	:PdfDocumentWriter(streamData, writer)
{
}

boost::shared_ptr<Flattener> Flattener::getInstance(const char * fileName, IPdfWriter * pdfWriter)
{
	// creates instance
	Flattener * instance;
	boost::shared_ptr<FileStreamData> streamData;
	try
	{
		streamData = boost::shared_ptr<FileStreamData>(PdfDocumentWriter::getStreamData(fileName));
		if (!streamData)
			return boost::shared_ptr<Flattener>();
		instance=new Flattener(*streamData, pdfWriter);
	}catch(std::exception & e)
	{
		// exception thrown from CXref so we have to do a cleanup
		utilsPrintDbg(debug::DBG_ERR, "Unable to create Flattener instance. Error message="<<e.what());
		if (streamData->file)
			fclose(streamData->file);
		if (streamData->stream)
			delete streamData->stream;
		throw e;
	}

	return boost::shared_ptr<Flattener>(instance, 
			FileStreamDataDeleter<Flattener>(*streamData));
}

namespace {

/** Equality operator for refList lookup.
 */
bool operator==(const ::Ref& r1, const ::Ref& r2)
{
	return r1.num == r2.num && r1.gen== r2.gen;
}

bool containsRef(const Flattener::RefList & refList, const ::Ref& ref)
{
	Flattener::RefList::const_iterator i;
	for(i=refList.begin(); i!=refList.end(); ++i)
		if(*i == ref)
			return true;
	return false;
}

// fwd declaration
void collectReachableRefs(::XRef& xref, const ::Object &obj, Flattener::RefList &refList);

/** Helper function to find all references from given dictionary.
 * @param xref XRef table.
 * @param dict Dictionary to be examined.
 * @param refList List of already collected references.
 * 
 */
void collectDictRefElems(::XRef &xref, const ::Dict &dict, Flattener::RefList &refList)
{
	boost::shared_ptr< ::Object> elem(XPdfObjectFactory::getInstance(), xpdf::object_deleter());
	for(int i=0; i<dict.getLength(); i++)
	{
		if(!dict.getValNF(i, elem.get()))
		{
			utilsPrintDbg(debug::DBG_ERR, "Unable to get dictionary entry with index "<<i);
			throw MalformedFormatExeption("bad data stream");
		}
		collectReachableRefs(xref, *elem, refList);
		elem->free();
	}
}

/** Collects all reachable objects from the given one.
 * @param xref XRef table.
 * @param obj Object to traverse.
 * @param refList List of alreadt collected references.
 *
 * Fills the given list with references which are recursively reachable 
 * from the given object.
 * <br>
 * If you start with the Trailer then you will collect all reachable 
 * objects.
 */
void collectReachableRefs(XRef& xref, const ::Object &obj, Flattener::RefList &refList)
{
	switch(obj.getType())
	{
		case objArray:
		{
			boost::shared_ptr< ::Object> elem(XPdfObjectFactory::getInstance(), xpdf::object_deleter());
			for(int i=0; i<obj.arrayGetLength(); i++)
			{
				if(!obj.arrayGetNF(i, elem.get()))
				{
					utilsPrintDbg(debug::DBG_ERR, "Unable to get array entry");
					throw MalformedFormatExeption("bad data stream");
				}
				collectReachableRefs(xref, *elem, refList);
				elem->free();
			}
			break;
		}
		case objDict:
		{
			const Dict *dict = obj.getDict();
			collectDictRefElems(xref, *dict, refList);
			break;
		}
		case objStream:
		{
			const Dict *streamDict = obj.streamGetDict();
			collectDictRefElems(xref, *streamDict, refList);
			break;
		}
		case objRef:
		{
			::Ref ref = obj.getRef();
			// check for already seen referencies and skip them
			if (containsRef(refList, ref))
				return;
			// TODO should be sorted by offset to keep the same
			// ordering in the file as the original document
			refList.push_back(ref);
			boost::shared_ptr< ::Object> target(XPdfObjectFactory::getInstance(), xpdf::object_deleter());
			if(!obj.fetch(&xref, target.get()) || !xref.isOk())
			{
				kernelPrintDbg(debug::DBG_ERR, ref<<" object fetching failed with code="
						<<xref.getErrorCode());
				throw MalformedFormatExeption("bad data stream");
			}
			collectReachableRefs(xref, *target, refList);
			break;
		}
		default:
			// nothing really interesting here
			break;
	}
}
} // annonymous namespace

void Flattener::initReachableObjects()
{
	utilsPrintDbg(debug::DBG_DBG, "Creating a list of the reachable objects");
	reachAbleRefs.clear();
	
	// traverses all objects reachable from trailer and put their references
	// to the reachAbleRefs - this should provide complete list of all objects
	// required for document
	const Object *trailer = getTrailerDict();
	collectReachableRefs(*this, *trailer, reachAbleRefs);
	utilsPrintDbg(debug::DBG_INFO, reachAbleRefs.size()<<" indirect objects collected");
	lastIndex=0;
}

int Flattener::flatten(const char * fileName)
{
	initReachableObjects();
	return writeDocument(fileName);
}

int Flattener::flatten(FILE * file)
{
	initReachableObjects();
	return writeDocument(file);
}

int Flattener::fillObjectList(IPdfWriter::ObjectList &objectList, int maxObjectCount)
{
	// collects (fetches) all objects from XRef::entries array, which are not 
	// free, to the objectList. Skips also Linearized dictionary
	utilsPrintDbg(debug::DBG_DBG, "Collecting objects starting from "<<lastIndex);
	objectList.clear();
	for(; lastIndex < reachAbleRefs.size(); lastIndex++)
	{
		::Ref ref = reachAbleRefs[lastIndex];
		// gets object and generation number
		int num=ref.num, gen=ref.gen;

		// stop if we reach the maximum objects
		if(maxObjectCount>0 && objectList.size()>=(size_t)maxObjectCount)
			break;

		::Object * obj=XPdfObjectFactory::getInstance();
		XRef::fetch(num, gen, obj);
		if(!isOk())
		{
			kernelPrintDbg(debug::DBG_ERR, ref<<" object fetching failed with code="
					<<errCode);
			xpdf::freeXpdfObject(obj);
			throw MalformedFormatExeption("bad data stream");
		}
		objectList.push_back(IPdfWriter::ObjectElement(ref, obj));
	}
	utilsPrintDbg(debug::DBG_DBG, "Returned "<<objectList.size()<<" objects");
	return objectList.size();
}
