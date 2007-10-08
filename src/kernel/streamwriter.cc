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

#include <stdio.h>
#include <errno.h>
#include "static.h"
#include "utils/debug.h"
#include "streamwriter.h"

//TODO use stream encoding

void FileStreamWriter::putChar(int ch)
{
	size_t pos=getPos();
	fputc(ch, f);
	fflush(f);
	setPos(pos+1);
}

void FileStreamWriter::putLine(const char * line, size_t length)
{
	if(!line)
		return;

	size_t pos=getPos();
	size_t totalWriten=0;
	
	// writes all data
	// FIXME what if not enough place for writing...
	while(totalWriten<length)
	{
		size_t writen=fwrite(line+totalWriten, sizeof(char), length-totalWriten, f);
		totalWriten+=writen;
	}
	fputc('\n', f);
	totalWriten++;
	fflush(f);
	setPos(pos+totalWriten);
}

bool FileStreamWriter::trim(size_t pos)
{
using namespace debug;

	kernelPrintDbg(DBG_DBG, "pos="<<pos);
	
	// checks whether underlaying stream is limited and pos is in that limited
	// part
	if(limited && length<pos)
	{
		kernelPrintDbg(DBG_ERR, "Trimed data are behind limited arae.");
		return false;
	}

	flush();
	size_t currPos=getPos();
	
	kernelPrintDbg(DBG_DBG, "Triming all data behind absolute file offset="<<start+pos);
	
	// truncates to contain first start+pos bytes
	ftruncate(fileno(f), start+pos);
	flush();

	// sets position to original one or at the end if original is after file end
	// (ftruncate doesn't change FILE stream position, but FileStream class does
	// some buffering and so it may be invalidated)
	if(currPos>start+pos)
	{
		setPos(0, -1);
		kernelPrintDbg(DBG_DBG, "Original position in removed area. Removing to file end. Offset="<<getPos());
	}
	else
		setPos(currPos);

	return true;
}

size_t FileStreamWriter::clone(FILE * file, size_t start, size_t length)
{
using namespace debug;

	if(!file)
		return 0;

	kernelPrintDbg(DBG_DBG, "start="<<start<<" length="<<length);

	setPos(start);
	char buffer[BUFSIZ];
	size_t read=0;
	size_t totalWriten=0;

	// copies content until there is something to read or length is fulfilled.
	while((read=fread(buffer, sizeof(char), std::min((size_t)BUFSIZ, length-totalWriten), f))>0)
	{
		size_t chunkWriten=0, writen;
		// writes whole read chunk
		while((writen=fwrite(buffer+chunkWriten, sizeof(char), read-chunkWriten, file))>0)
			chunkWriten+=writen;

		totalWriten+=chunkWriten;
	}
	if(int err=ferror(f))
		kernelPrintDbg(DBG_ERR, "error occured while stream file reading. Error code="<<err);

	kernelPrintDbg(DBG_INFO, totalWriten<<" bytes written to output file");

	return totalWriten;
}
