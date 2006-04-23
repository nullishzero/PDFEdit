// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80

/*
 * $RCSfile$
 *
 * $Log$
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
#include"streamwriter.h"

/** Returns minimum of given values.
 * @param a Value to compare.
 * @param b Value to compare.
 *
 */
#define min(a, b)\
	((a)<(b))?(a):(b)

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
	
	size_t pos=getPos();
	int writen=fprintf(f, "%s\n", line);
	fflush(f);
	setPos(pos+writen);
}

size_t FileStreamWriter::clone(FILE * file, size_t start, size_t length)
{
	if(!file)
		return 0;

	// if length is negative value, content until file end is duplicated
	bool wholeContent=false;
	if(length=0)
		wholeContent=true;
		
	
	setPos(start);
	char buffer[BUFSIZ];
	size_t read=0;
	size_t writen=0, totalWriten=0;

	// copies content until there is something to read or length is fulfilled.
	while((read=fread(buffer, sizeof(char), min(BUFSIZ, length), f))>0)
	{
		// TODO be sure all bytes were writen
		if((writen=fwrite(buffer, sizeof(char), read, file))<=0)
			break;
		totalWriten+=writen;

		// if we don't copy whole content, we will decreases total length to be
		// written
		if(!wholeContent)
			length-=writen;
	}

	return totalWriten;
}
