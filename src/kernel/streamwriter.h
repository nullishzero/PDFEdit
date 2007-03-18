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

/*
 * $RCSfile$
 *
 * $Log$
 * Revision 1.12  2007/03/18 22:14:29  mstsxfx
 * typo
 *
 * Revision 1.11  2007/02/04 20:17:02  mstsxfx
 * Common Licence comment for all cc and h files available in doc/licence_header
 * file and its content to all cc and h files in src/{gui,kernel,utils}
 * directories.
 * Xpdf code, QSA and QOutputDevice code are not touched
 *
 * Revision 1.10  2006/09/04 13:12:50  hockm0bm
 * FileStreamWriter::clone
 *         - if length parameter is 0, nothing is copied rather than to the end
 *           of document content (feature wasn't implemented completly and this
 *           feature is not used in project)
 *
 * Revision 1.9  2006/06/25 16:25:09  hockm0bm
 * doc update - doxygen warnings removed (if they are real problem)
 *
 * Revision 1.8  2006/05/23 19:04:50  hockm0bm
 * * new StreamWriter::trim method
 * * FileStreamWriter::trim implemented
 *
 * Revision 1.7  2006/05/16 18:02:06  hockm0bm
 * destructor documentation for FileStreamWriter
 *
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
	
	/** Removes all data behind given position.
	 * @param pos Stream offset from where to trim.
	 *
	 * Removes all bytes behind (inclusive) given position from the stream.
	 *
	 * @return true if stream was trimed, false otherwise.
	 */
	virtual bool trim(size_t pos)=0;
	
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
	/** Costructor.
	 * @param fA File handle for stream.
	 * @param startA Start offset in the file.
	 * @param limitedA Limited flag for stream (true if stream has limited
	 * size).
	 * @param lengthA Length of the stream (ignored if limitedA is false).
	 * @param dictA Dictionary for the stream (should be initialized as NULL
	 * object).
	 *
	 * Calls BaseStream and StreamWriter constructors with given dictA parameter
	 * and initializes FileStream super type with fA, startA, limitedA and dictA
	 * parameters.
	 */
	FileStreamWriter(FILE *fA, Guint startA, GBool limitedA, Guint lengthA, Object * dictA)
		: BaseStream(dictA),
		  StreamWriter(dictA),
		  FileStream(fA, startA, limitedA, lengthA, dictA) 
		  {};

	/** Destructor for FileStreamWriter.
	 *
	 * NOTE: doesn't close given file handle. Instance creator is responsible to
	 * that.
	 */
	virtual ~FileStreamWriter(){};
	
	/** Puts character to the file.
	 * @param ch Character to write.
	 *
	 * Additionally flushes all changes to the file and position is moved after
	 * inserted character.
	 * @see BaseStreamWriter::putChar
	 */
	virtual void putChar(int ch);

	/** Puts line to the file.
	 * @param line String to write.
	 *
	 * Additionally flushes all changes to the file. and position is moved after
	 * inserted string. 
	 * <br>
	 * Note that given is writen untils first `\0' character.
	 *
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
	
	/** Removes all data behind given file offset position.
	 * @param pos Stream offset where to start removing.
	 *
	 * Calls ftruncate function to truncate underlaying FILE stream to the start+pos 
	 * size (pos is relative position to stream start NOT absolut file start).
	 * FILE stream is flushed before and after truncation.
	 * <br>
	 * This method can be used when there is some pending garbage behind usuful
	 * data. File size is changed as a side effect. Note that if pos is greater
	 * than current file size, file will be appended with NUL characters (binary
	 * 0). If current stream position is in truncated area, it is moved to the 
	 * stream end.
	 * <br>
	 * Note that this operation may fail, if stream is limited and given pos is
	 * out of border:
	 * <pre>
	 * pos>length
	 * </pre>
	 *
	 * @return true if stream was trimed, false otherwise.
	 */
	virtual bool trim(size_t pos);
		
	/** Forces file flush.
	 *
	 * Calls fflush on the file handle.
	 */
	virtual void flush()const
	{
		fflush(f);
	}

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
	virtual size_t clone(FILE * file, size_t start, size_t length);
};

#endif
