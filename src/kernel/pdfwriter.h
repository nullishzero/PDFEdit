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

#ifndef _PDF_WRITER_H_
#define _PDF_WRITER_H_

#include "kernel/static.h"
#include "kernel/cxref.h"

/** Header of pdf file.
 * This string should be appended by pdf version number.
 */
extern const char * PDFHEADER;

/** Marker of trailer dictionary.
 *
 * Each trailer dictionary begins immediately after line containing this string.
 */
extern const char * TRAILER_KEYWORD;

/** Marker of cross reference table.
 * Each xref section begins immediately after line containing this string.
 */
extern const char * XREF_KEYWORD;

/** Marker of last cross reference starting offset.
 * This key word marks file offset where xref starts. The number is on following
 * line.
 */
extern const char * STARTXREF_KEYWORD;

/** Marker of pdf content end.
 * This key word marks end of pdf document in the file. 
 */
extern const char * EOFMARKER;

class StreamWriter;

namespace pdfobjects {

namespace utils {

/** Type for pdf writer observer value.
 *
 * This structure contains all information about current step. Step may be
 * interpreted as order of currently written object.
 */
struct OperationStep
{
	/** Last accomplished step.
	 */
	size_t currStep;
};

/** Type for pdf writer observers.
 *
 * Uses OperationStep as value keeper.
 */
typedef observer::IObserver<OperationStep> PdfWriterObserver;

/** Type for operation scope.
 * This sctructure contains information about total number of operation which 
 * has to be done for task named in task field.
 */
struct OperationScope
{
	/** Total number of steps for whole task.
	 */
	size_t total;

	/** Name of the task.
	 */
	std::string task;
};

/** Interface for progress visualizator.
 * Implementator should provide visual form of progress.
 * <br>
 * Progress is started by start method, changed by update method and finished by
 * finish method. If progress contains preciselly known number of steps,
 * setMaxStep method determines this number.
 */
class IProgressBar
{
public:
	/** Default virtual destructor.
	 * Empty destructor.
	 */
	virtual ~IProgressBar(){}
	
	/** Starts progress visualization.
	 */
	virtual void start()=0;

	/** Finishes progress visualization.
	 * Progress bar instance can be destroyed immediatelly after this method
	 * finishes.
	 */
	virtual void finish()=0;

	/** New progress state.
	 * @param step Number of current progress step.
	 */
	virtual void update(int step)=0;

	/** Sets maximum progress state.
	 * @param maxStep Maximum step.
	 *
	 * This should be called when percentage progress bar should be visualized.
	 */
	virtual void setMaxStep(int maxStep)=0;
};

/** Prograss bar observer.
 * Simple observer which holds IProgressBar implementator. Observer interface
 * implementation delegates progress visualization to maintained progressBar
 * field which is provided in constructor. notify method also recognizes
 * progress start and end and announces this fact to progressBar by
 * IProgressBar::start and IProgressBar::finish methods.
 * <br>
 * Instance should be created with IProgressBar implementator as constructor
 * parameter and registered to ObserverHandler which supports OperationStep
 * typed observers.
 */
class ProgressObserver: public PdfWriterObserver
{
	/** Implementation of progress bar.
	 * This instance is responsible for progress visualization. It is set in
	 * constructor. Instance is deallocated in destructor.
	 */
	IProgressBar * progressBar;

	/** Flag for started progress.
	 * This flag is set to false by default and it is changed when first
	 * notification comes and it changed back when all operations are performed.
	 */
	mutable bool started;
public:
	/** Initialization constructor.
	 * @param pB ProgressBar visualizator.
	 *
	 * Sets progressBar field with given parameter. It may be NULL, when no
	 * progress is displayed. started field is initialized to false.
	 */
	ProgressObserver(IProgressBar * pB):progressBar(pB), started(false){}

	/** Virtual destructor.
	 * 
	 * If started is true and progressBar is non NULL (progress is running now 
	 * and it is displayed) calls progressBar-&gt;finish() and then deallocates 
	 * progressBar instance.
	 */
	virtual ~ProgressObserver()throw()
	{
		if(progressBar)
		{
			if(started)
				progressBar->finish();
			delete progressBar;
		}
	}

	/** Observer notification method.
	 * @param newValue Current step of progress.
	 * @param context Context of progress.
	 *
	 * This method is called by observed object when progress has changed. If
	 * progressBar implementation is not provided, skips all further steps.
	 * <br>
	 * If no progress has started yet (started is false), calls 
	 * IProgressBar::start and IProgressBar::setMaxStep (with
	 * ScopedChangeContext::getScope()::total value).
	 * <br>
	 * Then updates progress with given newValue (calls
	 * IProgressBar::update).
	 * <br>
	 * Finally checks whether current step is last one and if so, sets started
	 * back to false and calls IProgressBar::finish method.
	 */
	void notify(boost::shared_ptr<OperationStep> newValue, 
			boost::shared_ptr<const observer::IChangeContext<OperationStep> > context)const throw();

	/** Returns observer priority.
	 * @return 0.
	 */
	observer::IObserver<OperationStep>::priority_t getPriority()const throw()
	{
		// TODO constant
		return 0;
	}
};


/** Base class for filter stream writers.
 * Note that this class - unlike StreamWriter classes defined in streamwriter.h 
 * file is not based on xpdf Stream object. Its purpose is to help IPdfWriter
 * class to write stream object into the pdf document.
 * <br>
 * Use factory getInstance method to get proper filter stream writer 
 * implementation for given stream object. Default writer is used if no such 
 * implementation is available.
 */
class FilterStreamWriter
{
public:
	typedef std::vector<boost::shared_ptr<FilterStreamWriter> > WritersList;
protected:
	/** List of the registered filter stream writers.
	 */
	static WritersList writers;

	/** Default filter stream writer used if no registered can be used.
	 */
	static boost::shared_ptr<FilterStreamWriter> defaultWriter;
public:
	/** Finds proper filter stream writer for given obj.
	 * If no registered appropriate writer exists, defaultWriter is tried. 
	 * If neither defaultWriter is ok, NullFilterStreamWriter is used which 
	 * writes stream object as it is without no filters.
	 *
	 * @param objStream Stream object.
	 * @return Appropriate filter stream writer (never NULL).
	 */
	static boost::shared_ptr<FilterStreamWriter> getInstance(const Object& objStream);

	/** Adds new filter writer to the registered.
	 * @param streamwriter Filter stream writer implementation.
	 */
	static void registerFilterStreamWriter(const boost::shared_ptr<FilterStreamWriter> &streamWriter);

	/** Removes already registerd filter writer.
	 * @param streamWriter Filter stream writer to remove.
	 */
	static void unregisterFilterStreamWriter(const boost::shared_ptr<FilterStreamWriter> &streamWriter);

	/** Removes all registered filter writers.
	 */
	static void clearAllFilterStreamWriters();

	/** Sets default stream writer.
	 * @param streamWriter Writer to be set.
	 */
	static void setDefaultStreamWriter(const boost::shared_ptr<FilterStreamWriter> &defaultWriter);

	/** Checks whether this filter writer is able to handle given object.
	 * Implementation is absolutely free on which criteria it decides whether
	 * it is or not able to handle this object. Usual way, however, is to check
	 * Filter entry and check if curren writer is able to write data in given
	 * format. @see compress method for more information
	 * @param obj Object to be written.
	 * @return true if able, false otherwise.
	 */
	virtual bool supportObject(const Object& obj)const =0;

	/** Writes given stream object to the output stream.
	 * Implementation has to follow pdf specification in format of the data
	 * writen in the stream. Nevertheless it is absolutely free in how it does it.
	 * It can modify given object to use those filters (and all associated 
	 * parameters) which are then used when data are written.
	 * @param obj Object to write (must be stream).
	 * @param ref Indirect reference for object (NULL for direct object).
	 * @param outStream Output stream where to put data.
	 */
	virtual void compress(const Object& obj, Ref* ref, StreamWriter& outStream)const =0;
};

/** Stream writer implementation with no filters.
 * This writer will write stream data as is with all original
 * filters and no modifications to original data.
 */
class NullFilterStreamWriter: public FilterStreamWriter
{
	static boost::shared_ptr<NullFilterStreamWriter> instance;
public:
	static boost::shared_ptr<NullFilterStreamWriter> getInstance();

	/** Checks whether given object is supported.
	 * @return allways true as it can write all stream objects.
	 */
	virtual bool supportObject(UNUSED_PARAM const Object& obj)const;

	/** Extracts stream data without any decoding.
	 */
	static unsigned char * null_extractor(const Object&obj, size_t& size);

	/** Writes given stream object to the stream.
	 * @param obj Stream object.
	 * @param ref Indirect reference for object (NULL if direct).
	 * @param outStream Stream where to write data.
	 *
	 * Uses streamToCharBuffer with null_extractor extractor.
	 */
	virtual void compress(const Object& obj, Ref* ref, StreamWriter& outStream)const;
};

/** Implementation of FlateDecode filter stream writer.
 * It is based on zlib implementation of default deflate method.
 */
class ZlibFilterStreamWriter: public FilterStreamWriter
{
	/** Shared writer instance */
	static boost::shared_ptr<ZlibFilterStreamWriter> instance;

	/** Updates given stream object with the applied fiter data.
	 * @param obj Stream object.
	 *
	 * Only for internal use of ZlibFilterStreamWriter class.
	 */
	static void update_dict(const Object& obj);
public:
	static boost::shared_ptr<ZlibFilterStreamWriter> getInstance();

	/** Checks whether given stream object is supported by this writer.
	 * @param obj Stream object.
	 * @return true if no filter FlateDecode are used.
	 */
	virtual bool supportObject(const Object& obj)const;

	/** Compress given buffer with deflate method.
	 * @param in Input buffer.
	 * @param in_size Input buffer size.
	 * @param size Size of the output buffer data.
	 * @return allocated buffer with the size data bytes or NULL on failure.
	 *
	 * Uses zlib interface to deflate given data.
	 */
	static unsigned char* deflate_buffer(unsigned char * in, size_t in_size, size_t& size);

	/** Stream data extractor implementation for streamToCharBuffer function.
	 * @param obj Stream object.
	 * @param size Size of the returned buffer data.
	 * @return allocated buffer with data or NULL on failure.
	 * 
	 * Uses bufferFromStreamData to get raw data without any filters,
	 * compresses returned buffer with the deflate_buffer function and 
	 * updates given stream object's dictionary to contain proper filter 
	 * data.
	 */
	static unsigned char* deflate(const Object& obj, size_t& size);

	virtual void compress(const Object& obj, Ref* ref, StreamWriter& outStream)const;
};

/** Interface for pdf content writer.
 *
 * Implementator knows how to put data to the file to create correct pdf
 * content.
 * <br>
 * Pdf content is written in 2 or 3 phases:
 * <ul>
 * <li>header writing - only if you are writing complete document (writeHeader).
 * <li>objects writing - writes given set of objects (can be called several times, 
 * each time with the batch of objects). All this data form single revision.
 * <li>cross reference & trailer writing - writes cross reference data and trailer 
 * which ends current revision. 
 * </ul>
 *
 * So each sequence of writeContent [, writeContent]* , writeTrailer forms new 
 * document revision. writeTrailer resets internal data and so it can be
 * repeated. 
 * <br>
 * Offset parameter can change default stream position. Default parameter value
 * is 0, current position is used.
 * <p>
 * Class implements ObserverHandler with OperationStep value keeper and so
 * Observers can be registered on each instance. Observer notification is in
 * full control of implementator.
 *
 * @see PdfWriterObserverContext
 * @see OperationScope
 * @see OperationStep
 */
class IPdfWriter:public observer::ObserverHandler<OperationStep>
{
public:
	/** Type for ObjectList element. */
	typedef std::pair<Ref, Object *> ObjectElement;

	/** Type for object list.
	 * Each element is pair of object reference and its value.
	 * <br>
	 * In any case (or future changes) this type supports push_back method for
	 * storing and iterators which points to ObjectElement typed value.
	 */
	typedef std::vector<ObjectElement> ObjectList;

	/** Type for pdf writer observer contenxt.
	 *
	 * This context holds OperationScope structure for change scope information. 
	 * User of this context should get scope information by getScope method and 
	 * its total field holds total number of operations that has to be perfomed. 
	 * Observer gets also current operation number which is enough for progress 
	 * information.
	 */
	typedef observer::ScopedChangeContext<OperationStep, OperationScope> ChangeContext; 

	/** Context for previous section.
	 * Contains information for proper linking with previous xref/trailer
	 * section.
	 */
	struct PrevSecInfo
	{
		/** Position of previous xref section.
		 *
		 * If no previous section is available value should be 0.
		 */
		size_t xrefPos;

		/** Number of entries in previous section.
		 */
		size_t entriesNum;
	};

	virtual ~IPdfWriter()
	{
#ifdef OBSERVER_DEBUG
		std::cerr << this <<":pdfWriter:";
		if(observers.size())
			dumpObservers();
		std::cerr << std::endl;
#endif
	}

	/** Writes PDF header to the given stream.
	 * @param version Version of the PDF standard used for this document.
	 * @param stream Stream writer where to write.
	 *
	 * Moves the current position in the stream at the begginning before
	 * writing.
	 */
	virtual void writeHeader(const char* version, StreamWriter &stream);

	/** Puts all objects to given stream.
	 * @param objectList List of objects to store.
	 * @param stream Stream writer where to write.
	 * @param off Stream offset where to start writing (if 0, uses current
	 * position).
	 * 
	 * Stores objects from the list and collects all information needed for 
	 * writeTrailer method. Objects which should be marked as free should have
	 * objNull type. It is up to implementator how it handles free objects, but
	 * writing them as null object is also correct.
	 * <br>
	 * Doesn't write xref and trailer.
	 */
	virtual void writeContent(const ObjectList & objectList, StreamWriter & stream, size_t off=0) =0;

	/** Writes xref and trailer section.
	 * @param trailer Trailer object.xrefPos.
	 * @param prevSection Context for previous section.
	 * @param stream Stream writer where to write.
	 * @param off Stream offset where to start writing (if 0, uses current
	 * position).
	 *
	 * Finishes pdf content writing by cross reference table and trailer
	 * dictionary. Uses internal data collected by writeContent method.
	 * reset method is called immediately after all data are written.
	 *
	 * @return stream position where it is safe to store data for new revision.
	 */
	virtual size_t writeTrailer(const Object & trailer, const PrevSecInfo &prevSection, StreamWriter & stream, size_t off=0)=0;

	/** Resets internal data collected in writeContent method.
	 *
	 * Everything collected in writeContent method, which is needed by
	 * writeTrailer for cross reference and trailer section generation, is
	 * cleared here.
	 */
	virtual void reset()=0;
};

/** Implementator of old style cross reference table pdf writer.
 *
 * Writes content with old style cross reference table and trailer.
 *
 */
class OldStylePdfWriter: public IPdfWriter
{
	/** Type for offset table.
	 * Mapping from reference to file offset of indirect object.
	 */
	typedef std::map<const ::Ref, size_t, xpdf::RefComparator> OffsetTab;

	/** Offset table.
	 *
	 * Keeps mapping from objects referencies to their position in the stream
	 * (where they were written).
	 *
	 */
	OffsetTab offTable;

	/** Maximum object number written.
	 *
	 * This value is set in writeContent method, used in writeTrailer and
	 * cleared by reset.
	 * <p>
	 * <b>Motivation</b>:
	 * <br>
	 * Each trailer contains Size field which contains total number of objects
	 * in document. This implies that also objects from previous sections are
	 * involved. Total count can't be determined from written objects, because
	 * we don't have any kind of guarantee, that all objects or even new objects
	 * were written.
	 * <br>
	 * This implementation handles this situation following way:<br>
	 * writeContent sets this field as maximum object number ever written from
	 * last reset (so for current section) and writeTrailer consumes parameter
	 * with previous trailer Size value. Result size is max{given, maxObjNum+1}.
	 * This is perfectly clean according PDF specification (see 3.4.4 Trailer
	 * chapter for more information).
	 */
	int maxObjNum;
public:
	/** String for context task in writeContent.
	 * This value is used in ScopedChangeContext's task field in writeContent
	 * method. User of the instance can change this value directly.
	 */
	static const std::string CONTENT;

	/** String for context task in writeTrailer.
	 * This value is used in ScopedChangeContext's task field in writeTrailer
	 * method. User of the instance can change this value directly.
	 */
	static const std::string TRAILER;
		
	/** Initialize constructor.
	 *
	 * Initializes CONTENT and TRAILER fields to default values.
	 */
	OldStylePdfWriter():maxObjNum(0){}

	/** Writes given objects.
	 * @param objectList List of objects to write.
	 * @param stream Stream writer where to write.
	 * @param off Stream offset where to start writing (if 0, uses current
	 * position).
	 *
	 * Sets new position in the stream, if off parameter is non 0 and iterate
	 * through all objects from given list and writes each to the stream (uses 
	 * helper writeIndirectObject function) and stores stream offset to the
	 * offTable mapping. 
	 * <br>
	 * Notifies all observers immediately after object has been written to the
	 * stream. newValue parameter is number of written objects until now and
	 * context (typed as ChangeContext with OperationScope typed scope)
	 * contains total number of objects which should be written by this call.
	 * Context task field contains CONTENT string.
	 */
	virtual void writeContent(const ObjectList & objectList, StreamWriter & stream, size_t off=0);

	/** Writes cross reference table and trailer.
	 * @param trailer Trailer object.
	 * @param prevSection Context for previous section.
	 * @param off Stream offset where to start writing (if 0, uses current
	 * position).
	 * @param stream Stream writer where to write.
	 *
	 * Constructs (old style) cross reference table from internal reference to 
	 * file offset mapping to the given stream according PDF specification.
	 * <br>
	 * In first step offTable is used for cross reference table builing. When
	 * cross reference table is ready, xref keyword is written to the stream 
	 * followed by built cross reference table.
	 * Then checks given prevSection (context for previous section) structures
	 * and if xrefPos is not zero, sets Prev trailer field to given value. 
	 * Finally calculates trailer's Size field as result of
	 * <pre>
	 * Size = max { prevSection.objNum, (maxObjNum + 1)}
	 * </pre>
	 * When all trailer information is ready, writes its dictionary to the 
	 * stream followed by lastxref key word with position of this cross 
	 * reference section start (file offset of xref key word). EOFMARKER 
	 * is written immediately after.
	 * <p>
	 * <b>PDF specification notes:</b>
	 * <br>
	 * Old style cross reference section is set of cross reference subsections.
	 * Each subsection is preceeded by object number and size values. This is
	 * followed by size number of lines. Each line represents object with
	 * (number+line#-1) object number. Line format is as follows:
	 * <pre>
	 * nnnnnnnnnn ggggg n \\n
	 * 
	 * where 
	 *		n* stands for file offset of object (padded by leading 0)
	 *		g* is generation number (padded by leading 0)
	 *		n is literal keyword identifying in-use object
	 * </pre>
	 * One subsection represents continuous sequence of indirect object numbers.
	 * <br>
	 * Cross reference subsections should also mark deleted objects, those which
	 * are not accessible anymore and so can be reused with higher generation
	 * number. This implementation doesn't write such entries.
	 * <p>
	 * Notifies observers immediately after one subsection is written. newValue
	 * parameter contains number of already written subsections and context
	 * (same asin writeContent method) contains total number of subsections and
	 * task field contains TRAILER string.
	 *
	 * @return stream position of pdf end of file %%EOF marker.
	 */
	virtual size_t writeTrailer(const Object & trailer, const PrevSecInfo &prevSection, StreamWriter & stream, size_t off=0);

	/** Resets all collected data.
	 *
	 * Clears offTable field and so this instance can be used for another 
	 * revision.
	 */
	virtual void reset();
};

/** Helper data structure which keeps all file stream related data.
 */
struct FileStreamData 
{
	/** Stream writer.
	 * Provides interface to write data to the stream.
	 */
	StreamWriter *stream;

	/** File handle.
	 *
	 * We have to keep file handle separately from the stream
	 * because the stream used for XRef doesn't allow to access its
	 * file handle from outside and also doesn't provide proper
	 * close functionality.
	 */
	FILE *file;
};

/** Helper class to deallocate a type which holds FileStreamData and
 * needs to destroy it after its holder is destroyed.
 *
 * This object should be used as a destroyer for smart_ptr shared 
 * pointer to accomplish after-destroy cleanup.
 * <br>
 * Currently only the file from FileStreamData structure is
 * considered (closed by functor) because stream is handled by others.
 * <br>
 * Note that you have to make this class as a friend if the destructor
 * is not available.
 */
template<typename T> class FileStreamDataDeleter
{
	FILE * file;
public:
	FileStreamDataDeleter(FileStreamData &data):file(data.file) {};

	/** Deallocates given instance and cleans-up necessary 
	 * FileStreamData fields.
	 * @param instance Instance to be destroyed.
	 */
	void operator()(T *instance)
	{
		assert(instance);
		delete instance;

		if(fclose(file))
		{
			int err = errno;
			kernelPrintDbg(debug::DBG_ERR, "Unable to close file handle (cause=\""
					<<strerror(err) << "\"");
		}
	}
};

/** Generic class to PDF document writing.
 * The main responsibility of this class is to provide unified interface
 * for arbitrary PDF document writing (note that this limits to complete
 * documents - so it doesn't include incremental update document writing).
 * <br>
 * Class is meant to be base type for other specialized document writers
 * which are supposed to implement fillObjectList abstract class. All 
 * other document structure related issues are already implemented in this
 * class.
 * <br>
 * Note that the class doesn't write content of the file direct but it
 * uses IPdfWriter provided in construtor or by setPdfWriter instead.
 */
class PdfDocumentWriter: public pdfobjects::CXref
{
	/** Number of objects that should be written in one batch.
	 * This constant is used as the maxObjectCount parameter to 
	 * fillObjectList method.
	 */
	static const int writeBatchCount = 1000;

protected:
	/** Pdf content writer implementator.
	 *
	 * All writing of pdf content is delegated to this object.
	 */
	IPdfWriter * pdfWriter;

	/** Abstract method to provide objects to be written.
	 * @param objectList List of objects to be filled.
	 * @param maxObjectCount Maximum number of objects to be filled.
	 *
	 * This method is called by writeDocument until it returns no objects.
	 * Implementation may clear the list before it adds new elements but
	 * is must provide up to given maxObjectCount. Use maxObjectCount=0 
	 * for unlimited number of objects.
	 *
	 * @return Number of objects added to given objectList.
	 * @throw MalformedFormatExeption if the document content is not valid.
	 */
	virtual int fillObjectList(IPdfWriter::ObjectList &objectList, int maxObjectCount)=0;
	
	/** Opens output file and writes a new document to it.
	 * @param fileName File to be opened.
	 * 
	 * Opens given file (in trucate mode) and delegates the rest to 
	 * delinearize(FILE *) method. If given file doesn't exist, it will be
	 * created. Finally closes file.
	 * @return 0 on success, errno otherwise.
	 * @throw NotImplementedException if document is encrypted.
	 */
	virtual int writeDocument(const char *fileName);

	/** Writes a new document to the given file.
	 * @param file Opened file handle where to write.
	 *
	 * Sets position to the file beginning and writes the same pdf header as in
	 * the original stream. Then writes all objects provided by fillObjectList 
	 * (calls this method repeatedly unless 0 objects are returned).
	 * Finally stores xref and trailer section.
	 * <br>
	 * Caller is responsible for file handle closing.
	 * <br>
	 * NOTE that method doesn't check whether given file is same as the 
	 * one used for input data. If it refers to same file, result is 
	 * unpredictable.
	 * <br>
	 * Returns with erro (EINVAL) if no pdfWriter is specified (it is NULL).
	 *
	 * @return 0 if everything ok, otherwise value of error of the error.
	 * @throw NotImplementedException if document is encrypted.
	 * @throw MalformedFormatExeption if the input file is currupted.
	 * 
	 * @return 0 on success, errno otherwise.
	 */
	virtual int writeDocument(FILE *file);

	/** Initialization constructor.
	 * @param streamData Input stream data.
	 * @param writer Pdf content writer.
	 *
	 * Uses CXref(BaseStream *) constructor and initializes file handle and 
	 * pdfWriter with given one (pdfWriter has to be allocated by new operator,
	 * because it is deallocated by delete in destructor - if NULL is provided,
	 * writeDocument methods do nothing).
	 *
	 * @throw MalformedFormatExeption if file content is not valid pdf document.
	 */
	PdfDocumentWriter(FileStreamData &data, IPdfWriter *pdfWriter);

public:
	/** Opens file and creates StreamData.
	 * @param fileName Name of the file to open.
	 * 
	 * This method should be preferred way to provide constructor
	 * data parameter.
	 * @return StreamData which contains opened stream and file handle or
	 * NULL if not able to open file for reading.
	 */
	static FileStreamData* getStreamData(const char * fileName);

	/** Destructor.
	 *
	 * Destroys CXref internal data and deallocates pdfWriter.  Note that
	 * file handle provided to the constructor has to be closed by
	 * descendants and it has to be closed after the instance is destroyed.
	 * FileStreamDataDeleter is the preferred way to do that.
	 */
	virtual ~PdfDocumentWriter();

	/** Sets new pdf content writer.
	 * @param pdfWriter IPdfWriter interface implementator.
	 *
	 * If given parameter is not NULL, sets new value of pdfWriter field and
	 * returns an old one. Otherwise just returns current one.
	 * <br>
	 * NOTE that caller is responsible for deallocation if provides new one.
	 *
	 * @return Currently set implementator or old value if parameter is non NULL.
	 */
	IPdfWriter * setPdfWriter(IPdfWriter * pdfWriter)
	{
		IPdfWriter *current=pdfWriter;
		
		if(pdfWriter)
			this->pdfWriter=pdfWriter;

		return current;
	}
	
};

} // namespace utils

} // namespace pdfobjects

#endif

