// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80
/*
 * $RCSfile$
 *
 * $Log$
 * Revision 1.2  2006/05/24 19:28:12  hockm0bm
 * * Delinearizator::getInstance mem leak
 *         - if Delinearizator constructor fails deallocates inputStream and
 *           closes FILE stream
 * * Delinearizator::delinearize mem leak
 *         - Object allocated also for free entries (which are skipped) - fixed
 * * destructor closes FILE stream
 *
 * Revision 1.1  2006/05/16 17:42:21  hockm0bm
 * * Delinearizator class implementation
 * * NotLinearizedException added
 *
 *
 */

#include <errno.h>
#include "delinearizator.h"
#include "debug.h"
#include "kernel/streamwriter.h"
#include "kernel/factories.h"


using namespace pdfobjects;
using namespace utils;

Delinearizator::Delinearizator(FileStreamWriter * stream, IPdfWriter * writer)
	:XRef(stream), 
	pdfWriter(writer)
{
	if(getErrorCode() !=errNone)
	{
		// xref is corrupted
		throw MalformedFormatExeption("XRef parsing problem errorCode="+getErrorCode());
	}

	if(!checkLinearized(*stream, this, &linearizedRef))
		throw NotLinearizedException();
}
Delinearizator * Delinearizator::getInstance(const char * fileName, IPdfWriter * pdfWriter)
{
using namespace debug;
	
	utilsPrintDbg(DBG_DBG, "fileName="<<fileName);

	// opens file handle and creates FileStreamWriter instance
	FILE * file=fopen(fileName, "r");
	if(!file)
	{
		int err=errno;
		utilsPrintDbg(DBG_ERR, "Unable to open file. Error message="<<strerror(err));
		return NULL;
	}
	Object dict;
	FileStreamWriter * inputStream=new FileStreamWriter(file, 0, false, 0, &dict);

	// creates instance
	Delinearizator * instance;
	try
	{
		instance=new Delinearizator(inputStream, pdfWriter);
	}catch(std::exception & e)
	{
		utilsPrintDbg(DBG_ERR, "Unable to create Delinearizator instance. Error message="<<e.what());
		delete inputStream;
		fclose(file);
		return NULL;
	}

	return instance;

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
		// FIXME to constant
		return -1;
	}
	if(!pdfWriter)
	{
		utilsPrintDbg(DBG_ERR, "No pdfWriter specified. Aborting");
		// FIXME to constant
		return -1;
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
			return -1;
		}
	}while(!strstr(buffer, PDFHEADER));

	// prints header to the file
	outputStream->putLine(buffer, strlen(buffer));

	// PDF specification suggests that header line should be followed by comment
	// line with some binary (with codes bigger than 128) - so application
	// transfering such files will copy them as binary data not as ASCII files
	buffer[0]='%';
	buffer[1]=129;
	buffer[2]=130;
	buffer[3]=253;
	buffer[4]=254;
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
		int num=i, 
			gen=entry.gen;
		
		// if entry is compressed (from object stream, gen is allways 0 but xpdf
		// uses this number for object order in stream)
		if(entry.type==xrefEntryCompressed)
			gen=0;

		// linearized dictionary is skipped and all other streams used for such
		// purposes are not in xref table
		if(num==linearizedRef.num && gen==linearizedRef.gen)
			continue;

		::Object * obj=XPdfObjectFactory::getInstance();
		XRef::fetch(num, gen, obj);
		::Ref ref={num, gen};
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
		freeXpdfObject(i->second);
		i->second=NULL;
	}

	outputStream->flush();
	// deallocates outputStream
	delete outputStream;

	return 0;
}

