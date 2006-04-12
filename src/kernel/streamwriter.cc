// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80

/*
 * $RCSfile$
 *
 * $Log$
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
#include"streamwriter.h"

void FileStreamWriter::putChar(int ch)
{
	fputc(ch, f);
	fflush(f);
}

void FileStreamWriter::putLine(const char * line)
{
	fprintf(FileStream::f, "%s\n", line);
	fflush(f);
}
