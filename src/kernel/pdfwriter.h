// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80

/*
 * $RCSfile$
 *
 * $Log$
 * Revision 1.7  2006/06/05 22:28:29  hockm0bm
 * * IProgressBar interface added
 * * ProgressObserver implemented
 *
 * Revision 1.6  2006/05/24 19:30:31  hockm0bm
 * OldStylePdfWriter::maxObjNum is initialized in constructor (it wasn't)
 *
 * Revision 1.5  2006/05/23 19:04:03  hockm0bm
 * OldStylePdfWriter::writeTrailer
 *         - signature changed returns position after stored xref section
 *         - trims averything behind stored data
 *
 * Revision 1.4  2006/05/17 19:42:00  hockm0bm
 * EOFMARKER is extern now
 *
 * Revision 1.3  2006/05/16 17:57:28  hockm0bm
 * * infrastructure for obserevers usable for IPdfWriter
 *         - OperationStep, OperationScope structures
 *         - PdfWriterObserver and PdfWriterObserverContext types
 * * OldStylePdfWriter uses observers to provide progress information
 * * writeTrailer method changed
 *         - lastXref parameter replaced by PrevSecInfo structure
 *         - Trailer Prev field removed if PrevSecInfo::xrefPos==0
 *         - Trailer Size field is set to proper value
 *
 * Revision 1.2  2006/05/09 20:10:55  hockm0bm
 * * doc update
 * * writeContent some checking added
 *         - duplicated entries are ignored
 *         - NULL entries are ignored
 *
 * Revision 1.1  2006/05/08 20:12:19  hockm0bm
 * * abstract IPdfWriter class for pdf content writers
 * * OldStylePdfWriter implementation of IPdfWriter
 * * pdf key words defined here now
 *
 *
 */
#ifndef _PDF_WRITER_H_
#define _PDF_WRITER_H_

#include<vector>
#include<map>
#include "streamwriter.h"
#include "utils/objectstorage.h"
#include "utils/observer.h"

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

/** Type for pdf writer observer contenxt.
 *
 * This context hold OperationScope structure for change scope information. User
 * of this context should get scope information by getScope method and its total
 * field holds total number of operations that has to be perfomed. Observer gets
 * also current operation number which is enough for progress information.
 */
typedef observer::ScopedChangeContext<OperationStep, OperationScope> PdfWriterObserverContext;

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
	virtual ~IProgressBar(){};
	
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
 * parameter and registered to IObserverHandler which supports OperationStep
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
	 * progress is displayed. started field is initialized to falser.;
	 */
	ProgressObserver(IProgressBar * pB):progressBar(pB), started(false){}

	/** Virtual destructor.
	 * 
	 * If started is true and progressBar is non NULL (progress is running now 
	 * and it is displayed) calls progressBar->finish() and them deallocates 
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
	 * This methos is called by observed object when progress has changed. If
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

/** Interface for pdf content writer.
 *
 * Implementator knows how to put data to the file to create correct pdf
 * content.
 * <br>
 * Pdf content is written in 2 phases:
 * <ul>
 * <li>objects writing - when writeContent method is called (one or more times).
 * This data forms revision.
 * <li>cross reference & trailer writing - when all collected data are used for
 * cross reference and trailer to finish PDF revision.
 * </ul>
 *
 * So each sequence of writeContent [, writeContent]* , writeTrailer forms new 
 * document revision. writeTrailer resets internal data and so it can be
 * repeated. 
 * <br>
 * Offset parameter can change default stream position. Default parameter value
 * is 0, current position is used.
 * <p>
 * Class implements IObserverHandler with OperationStep value keeper and so
 * Observers can be registered on each instance. Observer notification is in
 * full control of implementator.
 *
 * @see PdfWriterObserverContext
 * @see OperationScope
 * @see OperationStep
 */
class IPdfWriter:public observer::IObserverHandler<OperationStep>
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

		/** Number of objects in previous section.
		 *
		 * If no previous section is available value should be 0.
		 */
		size_t objNum;
	};

	virtual ~IPdfWriter(){};

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
	virtual void writeContent(ObjectList & objectList, StreamWriter & stream, size_t off=0) =0;

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
	virtual size_t writeTrailer(Object & trailer, PrevSecInfo prevSection, StreamWriter & stream, size_t off=0)=0;

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
	typedef std::map< ::Ref, size_t, RefComparator> OffsetTab;

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
	std::string CONTENT;

	/** String for context task in writeTrailer.
	 * This value is used in ScopedChangeContext's task field in writeTrailer
	 * method. User of the instance can change this value directly.
	 */
	std::string TRAILER;
		
	/** Initialize constructor.
	 *
	 * Initializes CONTENT and TRAILER fields to default values.
	 */
	OldStylePdfWriter():maxObjNum(0), CONTENT("Content phase"), TRAILER("XREF/TRAILER phase"){};

	/** Writes given objects.
	 * @param objectList List of objects to write.
	 * @param stream Stream writer where to write.
	 * @param off Stream offset where to start writing (if 0, uses current
	 * position).
	 *
	 * Uses xpdfObjToString method to get string representation with
	 * createIndirectObjectStringFromString decorator function to get proper PDF
	 * representation of indirect object and writes this string using
	 * stream::putLine method.
	 * <br>
	 * First object is stored at specified position and all others are placed
	 * immediately after.
	 * <br>
	 * Notifies all observers immediatelly after object has been written to the
	 * stream. newValue parameter is number of written objects until now and
	 * context (typed as ScopedChangeContext with OperationScope typed scope)
	 * contains total number of object which should be written by this call.
	 * Context task field contains CONTENT string.
	 */
	virtual void writeContent(ObjectList & objectList, StreamWriter & stream, size_t off=0);

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
	 * In first step offTable is used for cross reference table creation. This
	 * contain set of cross reference subsections.
	 * Then xref keyword is written to the stream followed be cross reference
	 * table constructed from subsections.
	 * Then checks given prevSection (context for previous section) structures
	 * and if xrefPos is not zero sets Prev trailer field to given value. Finaly
	 * calculates trailer's Size field as result of
	 * <pre>
	 * Size = max { prevSection.objNum, (maxObjNum + 1)}
	 * </pre>
	 * When all trailer information is ready, writes it also to the stream 
	 * followed by lastxref key word with position of this cross reference 
	 * section start. EOFMARKER is written immediately after.
	 * <p>
	 * <b>PDF specification notes:</b>
	 * <br>
	 * Old style cross reference section is set of cross reference subsections.
	 * Each subsection is preceeded by object number and size values. This is
	 * followed by size number of lines. Each line represents object with
	 * (number+line#-1) object number. Line format is as follows:
	 * <pre>
	 * nnnnnnnnnn ggggg n \n
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
	virtual size_t writeTrailer(Object & trailer, PrevSecInfo prevSection, StreamWriter & stream, size_t off=0);

	/** Resets all collected data.
	 *
	 * Clears offTable field and so this instance can used for another revision.
	 */
	virtual void reset();
};

} // namespace utils

} // namespace pdfobjects

#endif

