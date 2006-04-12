// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80

/*
 * $RCSfile$
 *
 * $Log$
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
	 * Additionally flushes all changes to the file.
	 * @see BaseStreamWriter::putChar
	 */
	virtual void putChar(int ch);

	/** Puts line to the file.
	 * Additionally flushes all changes to the file.
	 * @see BaseStreamWriter::putLine
	 */
	virtual void putLine(const char * line);
};

#endif
