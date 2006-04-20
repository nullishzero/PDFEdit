// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80

/*
 * $RCSfile$
 *
 * $Log$
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
