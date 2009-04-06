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
#include "kernel/delinearizator.h"
#include "kernel/pdfwriter.h"
#include "utils/debug.h"
#include "kernel/cxref.h"
#include "kernel/xrefwriter.h"
#include "kernel/streamwriter.h"
#include "kernel/factories.h"
#include "kernel/pdfedit-core-dev.h"


namespace pdfobjects {
namespace utils {

Delinearizator::Delinearizator(FileStreamData &streamData, IPdfWriter * writer)
	:PdfDocumentWriter(streamData, writer)
{
	if(!checkLinearized(*streamData.stream, this, &linearizedRef))
		throw NotLinearizedException();
}

boost::shared_ptr<Delinearizator> Delinearizator::getInstance(
		const char * fileName, IPdfWriter * pdfWriter)
{
	// creates instance
	Delinearizator * instance;
 	boost::shared_ptr<FileStreamData> streamData;
	try
	{
		streamData = boost::shared_ptr<FileStreamData>(PdfDocumentWriter::getStreamData(fileName));
		if (!streamData)
			return boost::shared_ptr<Delinearizator>();
		instance=new Delinearizator(*streamData, pdfWriter);
	}catch(NotLinearizedException &e)
	{
		utilsPrintDbg(debug::DBG_ERR, "Document is not linearized.");
		// exception from Delinearizator (~CXref deallocates stream
		// and we have to close file handle
		if (streamData->file)
			fclose(streamData->file);
		return boost::shared_ptr<Delinearizator>();
	}catch(std::exception & e)
	{
		// exception thrown from CXref so we have to do a cleanup
		utilsPrintDbg(debug::DBG_ERR, "Unable to create Delinearizator instance. Error message="<<e.what());
		if (streamData->file)
			fclose(streamData->file);
		if (streamData->stream)
			delete streamData->stream;
		throw e;
	}

	// Uses FileStreamDataDeleter to correctly delete delinearizator
	// with proper FileStreamData cleanup
	return boost::shared_ptr<Delinearizator> (instance, 
			FileStreamDataDeleter<Delinearizator>(*streamData));
}

int Delinearizator::delinearize(const char * fileName)
{
	lastObj=0;
	return writeDocument(fileName);
}

int Delinearizator::delinearize(FILE * file)
{
	lastObj=0;
	return writeDocument(file);
}

int Delinearizator::fillObjectList(IPdfWriter::ObjectList &objectList, int maxObjectCount)
{
  	// collects (fetches) all objects from XRef::entries array, which are not 
  	// free, to the objectList. Skips also Linearized dictionary
	utilsPrintDbg(debug::DBG_DBG, "Collecting objects starting from "<<lastObj);
	objectList.clear();
	for(; lastObj < ::XRef::size; )
  	{
		// stop if we reach the maximum objects
		if(maxObjectCount>0 && (int)objectList.size()>=maxObjectCount)
			break;

  		// gets object and generation number
		int num=lastObj++;
		::XRefEntry entry=::XRef::entries[num];
  		if(entry.type==xrefEntryFree)
  			continue;

		// if entry is compressed (from object stream, gen is allways 0 but xpdf
		// uses this number for object order in stream)
		int gen=entry.gen;
		if(entry.type==xrefEntryCompressed)
			gen=0;
  		
  		// if entry is compressed (from object stream, gen is allways 0 but xpdf
  		// uses this number for object order in stream)
  		if(num==linearizedRef.num && gen==linearizedRef.gen)
  			continue;
  
  		::Object * obj=XPdfObjectFactory::getInstance();
  		::Ref ref={num, gen};
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
} //namespace utils
} //namespace pdfobjects
