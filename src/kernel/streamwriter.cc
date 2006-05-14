// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80

/*
 * $RCSfile$
 *
 * $Log$
 * Revision 1.10  2006/05/14 16:15:07  hockm0bm
 * quick fix
 *         - putLine appends new line character
 *
 * Revision 1.9  2006/05/14 12:35:24  hockm0bm
 * * StreamWriter
 *         - putLine() with size parameter method added
 * * FileStreamWriter
 *         - putLine(char *, size_t) implemented
 *         - putLine(char *) uses one with size
 *
 * Revision 1.8  2006/05/08 14:23:02  hockm0bm
 * * StreamWriter new flush method added
 * * clone method corrected
 *         - checking for whole content copy
 *
 * Revision 1.7  2006/05/06 08:43:14  hockm0bm
 * clone method bug fix assignment instead of comparision
 *
 * Revision 1.6  2006/04/23 13:11:38  hockm0bm
 * clone method added to StreamWriter
 *
 * Revision 1.5  2006/04/20 20:22:40  hockm0bm
 * charachter writing moves position - uses setPos() in both putChar and putLine
 *
 * Revision 1.4  2006/04/17 19:58:41  hockm0bm
 * FileStreamWriter::putLine checks parameter for NULL
 *
 * Revision 1.3  2006/04/13 18:17:43  hockm0bm
 * TODO for filter usage in FileStreamWriter if any
 *
 * Revision 1.2  2006/04/12 20:49:03  hockm0bm
 * FileStreamWriter methods flushes immediately after writing
 *
 * Revision 1.1  2006/04/12 17:39:05  hockm0bm
 * StreamWriter base class wor all stream writers
 * FileStreamWriter writer for FileStreams
 *
 *
 */

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

void FileStreamWriter::putLine(const char * line)
{
	// NULL line is ignored
	if(!line)
		return;

	// uses putLine with number of bytes same as string length
	putLine(line, strlen(line));
	
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

size_t FileStreamWriter::clone(FILE * file, size_t start, size_t length)
{
using namespace debug;

	if(!file)
		return 0;

	kernelPrintDbg(DBG_DBG, "start="<<start<<" length="<<length);

	// if length is negative value, content until file end is duplicated
	bool wholeContent=false;
	if(length==0)
		wholeContent=true;
		
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
