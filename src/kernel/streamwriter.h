// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80

/*
 * $RCSfile$
 *
 * $Log$
 * Revision 1.6  2006/05/14 12:35:25  hockm0bm
 * * StreamWriter
 *         - putLine() with size parameter method added
 * * FileStreamWriter
 *         - putLine(char *, size_t) implemented
 *         - putLine(char *) uses one with size
 *
 * Revision 1.5  2006/05/08 14:23:02  hockm0bm
 * * StreamWriter new flush method added
 * * clone method corrected
 *         - checking for whole content copy
 *
 * Revision 1.4  2006/04/23 13:11:38  hockm0bm
 * clone method added to StreamWriter
 *
 * Revision 1.3  2006/04/20 20:22:40  hockm0bm
 * charachter writing moves position - uses setPos() in both putChar and putLine
 *
 * Revision 1.2  2006/04/12 20:49:03  hockm0bm
 * FileStreamWriter methods flushes immediately after writing
 *
 * Revision 1.1  2006/04/12 17:39:06  hockm0bm
 * StreamWriter base class wor all stream writers
 * FileStreamWriter writer for FileStreams
 *
 *
 */

#include "xpdf.h"

/**
 * @file streamwriter.h
 *
 * Stream writers classes.
 *
 * This header file provides extended classes for manipulation with xpdf Stream
 * based classed. They additionaly provides writing functionality. Each class
 * has same name as original one with Writer suffix.
 */
#ifndef _STREAM_WRITER_
#define _STREAM_WRITER_

/** Abstract class for all BaseStream writers.
 *
 * Declares base interface for all writers to Base stream. All real writers
 * should implement this abstract class and stream type which is written.
 */
class StreamWriter: virtual public BaseStream
{
public:
	/** Constructor with dictionary object.
	 * @param dictA Object where to store stream dictionary.
	 */
	StreamWriter(Object * dictA):BaseStream(dictA){};
	
	/** Puts character at current position.
	 * @param ch Character to put to the stream.
	 *
	 * If current position is not at the end of file, overwrites previous content
	 * and moves position. Otherwise appends stream.
	 */
	virtual void putChar(int ch)=0;

	/** Puts line from current position.
	 * @param line Line to put to the stream.
	 *
	 * If current position is not at the end of the file, overwrites original
	 * content. Characters which doesn't fit, expands stream.
	 * <br>
	 * Given string is appended by newline character. 
	 */
	virtual void putLine(const char * line)=0;

	/** Puts exactly length number of byte to one line.
	 * @param line Line buffer pointer.
	 * @param length Number of bytes to be printed.
	 *
	 * Doesn't check end of the string '\0' and prints exactly given number of
	 * bytes. This should be used when binary data are stored in line and so
	 * they may contain '\0' bytes.
	 * <br>
	 * Caller should guarantee that line is allocated at least for length size.
	 * Otherwise result is unpredictable.
	 */
	virtual void putLine(const char * line, size_t length)=0;
	
	/** Forces stream flush.
	 *
	 * Cached data in stream are forced to be writen to the target.
	 */
	virtual void flush()const =0;
	
	/** Duplicates content to given file.
	 * @param file File where to put duplicated content.
	 * @param start Position where to start duplication.
	 * @param length Number of bytes to be duplicated.
	 *
	 * Copies up to length bytes from start postion from stream to given file.
	 * If length is 0, copies content until end of stream.
	 *
	 * @return number of bytes writen to given file.
	 */ 
	virtual size_t clone(FILE * file, size_t start, size_t length) =0;
};

/** FileStream writer.
 *
 * Implements BaseStreamWriter and FileStream to enable writing to the file
 * stream.
 */
class FileStreamWriter:  virtual public StreamWriter, public FileStream
{
public:
	FileStreamWriter(FILE *fA, Guint startA, GBool limitedA, Guint lengthA, Object * dictA)
		: BaseStream(dictA),
		  StreamWriter(dictA),
		  FileStream(fA, startA, limitedA, lengthA, dictA) 
		  {};

	virtual ~FileStreamWriter(){};
	
	/** Puts character to the file.
	 * Additionally flushes all changes to the file and position is moved after
	 * inserted character.
	 * @see BaseStreamWriter::putChar
	 */
	virtual void putChar(int ch);

	/** Puts line to the file.
	 * Additionally flushes all changes to the file. and position is moved after
	 * inserted string.
	 * @see BaseStreamWriter::putLine
	 */
	virtual void putLine(const char * line);

	/** Puts exactly length number of byte to one line.
	 * @param line Line buffer pointer.
	 * @param length Number of bytes to be printed.
	 *
	 * Prints exactly length number of bytes starting from given line.
	 * Additionally flushes all changes to the file and position is moved after
	 * inserted buffer.
	 *
	 * @see StreamWriter::putLine(const char *, size_t)
	 */
	virtual void putLine(const char * line, size_t length);
	
	/** Forces file flush.
	 *
	 * Calls fflush on the file handle.
	 */
	virtual void flush()const
	{
		fflush(f);
	}

	/** Duplicates content ro given file.
	 * @param file File where to put duplicated content.
	 * @param start Position where to start duplication.
	 * @param length Number of bytes to be duplicated.
	 *
	 * Copies up to length bytes from start postion from stream to given file.
	 * If length is 0, copies content until end of stream.
	 *
	 * @return number of bytes writen to given file.
	 */ 
	virtual size_t clone(FILE * file, size_t start, size_t length);
};

#endif
