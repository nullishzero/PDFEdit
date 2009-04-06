/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006-2009  PDFedit team: Michal Hocko,
 *                                        Jozef Misutka,
 *                                        Martin Petricek
 *                   Former team members: Miroslav Jahoda
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


#include "kernel/static.h"

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
	StreamWriter(Object * dictA):BaseStream(dictA){}
	
	/** Puts character at current position.
	 * @param ch Character to put to the stream.
	 *
	 * If current position is not at the end of file, overwrites previous content
	 * and moves position. Otherwise appends stream.
	 */
	virtual void putChar(int ch)=0;

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
	virtual size_t cloneToFile(FILE * file, size_t start, size_t length) =0;

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
		  {}

	/** Destructor for FileStreamWriter.
	 *
	 * NOTE: doesn't close given file handle. Instance creator is responsible to
	 * that. The reason for this is based on xpdf usage of streams in general.
	 * Xpdf Parser/Lexer classes use higly Stream::makeSubStream method to get
	 * only subset (limited range) of the stream data and when they are finieshed,
	 * this substream is deallocated.
	 * FileStream, in turn, creates such a substream that shares file handle with
	 * the original one. Therefore FileStream and also all descendants *MUST NOT*
	 * close file handle in destructor. This is the case also for this class.
	 * Otherwise we would have invalid file handle in the original stream after
	 * substream is not needed (and deallocated).
	 */
	virtual ~FileStreamWriter(){}
	
	/** Puts character to the file.
	 * @param ch Character to write.
	 *
	 * Additionally flushes all changes to the file and position is moved after
	 * inserted character.
	 * @see BaseStreamWriter::putChar
	 */
	virtual void putChar(int ch);

	/** Puts exactly length number of byte to one line.
	 * @param line Line buffer pointer.
	 * @param length Number of bytes to be printed.
	 *
	 * Prints exactly length number of bytes starting from given line.
	 * Additionally flushes all changes to the file and position is moved after
	 * inserted buffer.
	 * Appends LF after given string.
	 *
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
	virtual size_t cloneToFile(FILE * file, size_t start, size_t length);
};

#endif
