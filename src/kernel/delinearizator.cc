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

Delinearizator::Delinearizator(FileStreamWriter * stream, IPdfWriter * writer)
	:CXref(stream),
	pdfWriter(writer)
{
	if(!checkLinearized(*stream, this, &linearizedRef))
		throw NotLinearizedException();
}

/** Class to deallocate Delinearizator and close associated file
 * handle from smart pointer.
 */
class DelinearizatorDeleter
{
	FILE * file;
public:
	DelinearizatorDeleter(FILE * f): file(f) {};

	void operator ()(Delinearizator * delinearizator)
	{
		assert(delinearizator);
		delete delinearizator;

		// Note that file handle cannot be closed in the destructor 
		// because CXref (parent type) will destroy stream (XRef::str) 
		// which is based on FileStream which in turn may access file 
		// handle which could cause memory corruptions
		if(fclose(file))
		{
			int err = errno;
			kernelPrintDbg(debug::DBG_ERR, "Unable to close file handle (cause=\""
					<<strerror(err) << "\"");
		}

		kernelPrintDbg(debug::DBG_DBG, "Instance deleted.");
	}
};

boost::shared_ptr<Delinearizator> Delinearizator::getInstance(
		const char * fileName, IPdfWriter * pdfWriter)
{
	
	utilsPrintDbg(debug::DBG_DBG, "fileName="<<fileName);

	// opens file handle and creates FileStreamWriter instance
	FILE * file=fopen(fileName, "r");
	if(!file)
	{
		int err=errno;
		utilsPrintDbg(debug::DBG_ERR, "Unable to open file. Error message="<<strerror(err));
		return boost::shared_ptr<Delinearizator>();
	}
	Object dict;
	FileStreamWriter * inputStream=new FileStreamWriter(file, 0, false, 0, &dict);

	// creates instance
	Delinearizator * instance;
	try
	{
		instance=new Delinearizator(inputStream, pdfWriter);
	}catch(NotLinearizedException &e)
	{
		utilsPrintDbg(debug::DBG_ERR, "Document is not linearized.");
		// exception from Delinearizator (~CXref deallocates stream
		// and we have to close file handle
		fclose(file);
		return boost::shared_ptr<Delinearizator>();
	}catch(std::exception & e)
	{
		// exception thrown from CXref so we have to do a cleanup
		utilsPrintDbg(debug::DBG_ERR, "Unable to create Delinearizator instance. Error message="<<e.what());
		fclose(file);
		delete inputStream;
		return boost::shared_ptr<Delinearizator>();
	}

	return boost::shared_ptr<Delinearizator> (instance, DelinearizatorDeleter(file));
}

Delinearizator::~Delinearizator()
{
	if(pdfWriter)
		delete pdfWriter;
}

int Delinearizator::delinearize(const char * fileName)
{
using namespace debug;

	utilsPrintDbg(DBG_DBG, "fileName="<<fileName);

	FILE * file=fopen(fileName, "w");
	int err=0;
	if(!file)
	{
		err=errno;
		utilsPrintDbg(DBG_ERR, "Unable to open file. Error message="<<strerror(err));
		return err;
	}
	
	// delegates and closes file
	err=delinearize(file);
	fclose(file);
	return err;
}

int Delinearizator::delinearize(FILE * file)
{
using namespace debug;

	utilsPrintDbg(DBG_DBG, "");
	if(!file)
	{
		utilsPrintDbg(DBG_ERR, "Bad file handle");
		return EINVAL;
	}
	if(!pdfWriter)
	{
		utilsPrintDbg(DBG_ERR, "No pdfWriter specified. Aborting");
		return EINVAL;
	}

	// don't use check_need_credentials here, because we don't want to throw
	// exception in negative case
	if(getNeedCredentials())
	{
		utilsPrintDbg(DBG_ERR, "No credentials available for encrypted document.");
		return EPERM;
	}
	if(isEncrypted())
	{
		utilsPrintDbg(DBG_ERR, "Delinearization of encrypted documents is not implemented");
		throw NotImplementedException("Encrypted document");
	}
	
	// stream from input file
	StreamWriter * inputStream=dynamic_cast<StreamWriter *>(::XRef::str);

	// creates outputStream writer from given file
	Object dict;
	StreamWriter * outputStream=new FileStreamWriter(file, 0, false, 0, &dict);
	
	// buffer for data
	char buffer[1024];
	memset(buffer, '\0', sizeof(buffer));
	
	// resets outputStream and sets position by Stream::getStart method to copies all 
	// data until end of the line (first of CR or LF characters)
	inputStream->reset();
	inputStream->setPos(inputStream->getStart());
	do
	{
		if(!inputStream->getLine(buffer, sizeof(buffer)-1))
		{
			// this should never happen
			utilsPrintDbg(DBG_ERR, "File is corrupted because, doesn't contain proper PDF header.");
			throw MalformedFormatExeption("pdf header missing");
		}
	}while(!strstr(buffer, PDFHEADER));

	// prints header to the file
	outputStream->putLine(buffer, strlen(buffer));

	// PDF specification suggests that header line should be followed by comment
	// line with some binary (with codes bigger than 128) - so application
	// transfering such files will copy them as binary data not as ASCII files
	buffer[0]='%';
	buffer[1]=(char )129;
	buffer[2]=(char )130;
	buffer[3]=(char )253;
	buffer[4]=(char )254;
	buffer[5]='\0';
	outputStream->putLine(buffer, strlen(buffer));

	// collects (fetches) all objects from XRef::entries array, which are not 
	// free, to the objectList. Skips also Linearized dictionary
	utilsPrintDbg(DBG_DBG, "Collecting all objects");
	IPdfWriter::ObjectList objectList;
	for(size_t i=0; i < (size_t)::XRef::size; i++)
	{
		::XRefEntry entry=::XRef::entries[i];
		if(entry.type==xrefEntryFree)
			continue;

		// gets object and generation number
		int num=i, gen=entry.gen;
		
		// if entry is compressed (from object stream, gen is allways 0 but xpdf
		// uses this number for object order in stream)
		if(entry.type==xrefEntryCompressed)
			gen=0;

		// linearized dictionary is skipped and all other streams used for such
		// purposes are not in xref table
		if(num==linearizedRef.num && gen==linearizedRef.gen)
			continue;

		::Object * obj=XPdfObjectFactory::getInstance();
		::Ref ref={num, gen};
		XRef::fetch(num, gen, obj);
		if(!isOk())
		{
			// TODO how to handle cleanly - with outpustream cleanup
			kernelPrintDbg(DBG_ERR, ref<<" object fetching failed with code="
					<<errCode);
			xpdf::freeXpdfObject(obj);
			delete outputStream;
			throw MalformedFormatExeption("bad data stream");
		}
		objectList.push_back(IPdfWriter::ObjectElement(ref, obj));
	}

	// writes collected objects and xref & trailer section
	utilsPrintDbg(DBG_INFO, "Writing "<<objectList.size()<<" objects to the output outputStream.");
	pdfWriter->writeContent(objectList, *outputStream);
	utilsPrintDbg(DBG_INFO, "Writing xref and trailer section");
	// no previous section information and all objects are going to be written
	IPdfWriter::PrevSecInfo prevInfo={0, 0};
	pdfWriter->writeTrailer(*getTrailerDict(), prevInfo, *outputStream);

	// clean up
	utilsPrintDbg(DBG_DBG, "Cleaning up all writen objects("<<objectList.size()<<").");
	for(IPdfWriter::ObjectList::iterator i=objectList.begin(); i!=objectList.end(); i++)
	{
		xpdf::freeXpdfObject(i->second);
		i->second=NULL;
	}

	outputStream->flush();
	// deallocates outputStream
	delete outputStream;

	return 0;
}
} //namespace utils
} //namespace pdfobjects
