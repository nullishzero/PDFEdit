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
#ifndef _DELINEARIZATOR_H_
#define _DELINEARIZATOR_H_

#include "kernel/xpdf.h"
#include "kernel/exceptions.h"
#include "kernel/pdfwriter.h"

/** Exception for not linearized pdf documents.
 * This exception is thrown if Delinearizator is used for non linearized
 * documents.
 *
 */
class NotLinearizedException:public PdfException
{
public:
	virtual ~NotLinearizedException()throw()
	{}

	virtual const char * what()throw()
	{
		return "Content is not linerized";
	}
};

namespace pdfobjects
{

namespace utils
{

/** Delinearizator class.
 *
 * Provides functionality enabling correct changing linearized pdf documents to
 * their unlinearized form. This may be very usefull especially if changes has to
 * be done to document, because linearized document has rather strict rules and
 * so can be easily broken by incremental changes.
 * <p>
 * Class reuses XRef class for low level pdf reading and parsing purpouses
 * and adds logic related to linearized documents.
 * <p>
 * Output file will contain all objects - except those needed by linearized
 * structure - in same format (e. g. filters in content streams), object and
 * generation numbers. 
 * <br>
 * Linearized documents are not prepared for multiversion documents very well 
 * (as mentioned before) and so output file will contain just one trailer and 
 * xref (so one revision). Format of this final section fully depends on given
 * IPdfWriter implementator (set in constructor).
 * <p>
 * <b>Usage</b>
 * <br>
 * Use static factory method for instance creation:
 * <pre>
 * // we will use OldStylePdfWriter IPdfWriter implementator
 * IPdfWriter * contentWriter=new OldStylePdfWriter();
 * Delinearizator * delinearizator=Delinearizator::getInstance(fileName, contentWriter);
 *
 * // delinearize file content to file specified by name
 * delinearizator->delinearize(outputFile);
 * 
 * ...
 * 
 * // destroys delinearizator with conentWriter
 * delete delinearizator;
 * </pre>
 */
class Delinearizator: protected ::XRef
{
	/** Pdf content writer implementator.
	 *
	 * All writing of pdf content is delegated to this object.
	 */
	IPdfWriter * pdfWriter;

	/** Reference of linearized dictionary.
	 */
	::Ref linearizedRef;

	/** File handle for the file stream.
	 * It is initialized in constructor and closed in destructor. This has to be
	 * done, because stream used for XRef doesn't allow to access its file
	 * handle from outside and also doesn't provide proper close functionality.
	 */
	FILE * file;
protected:
	/** Initialization constructor.
	 * @param f File handle for stream.
	 * @param stream FileStreamWriter instance.
	 * @param writer Pdf content writer.
	 *
	 * Uses XRef(BaseStream *) constructor and initializes file handle and 
	 * pdfWriter with given one (pdfWriter has to be allocated by new operator,
	 * because it is deallocated by delete in destructor - if NULL is provided,
	 * delinearize methods do nothing). 
	 * 
	 * @throw MalformedFormatExeption if file content is not valid pdf document.
	 * @throw NotLinearizedException if file content is not linearized.
	 */
	Delinearizator(FILE * f, FileStreamWriter * stream, IPdfWriter * writer);
public:
	/** Destructor.
	 *
	 * Destroys XRefWriter internal data, deallocates pdfWriter and closes file
	 * handle opened used for FileStream.
	 */
	~Delinearizator()
	{
		if(pdfWriter)
			delete pdfWriter;
		// FILE stream has to be closed - FileStream::close method doesn't do that!
		fclose(file);
	}
	
	/** Factory method for instance creation.
	 * @param fileName Name of the pdf file.
	 * @param pdfWriter  Pdf content writer.
	 *
	 * Creates FileStream from given fileName (file is open with `r' mode) and 
	 * creates Delinearizator instance. Finally checks whether file is 
	 * linearized and if not, prints error message and returns with NULL.
	 *
	 * @throw MalformedFormatExeption if file content is not valid pdf document.
	 * @return Delinearizator instance ready to be used or NULL, if given file
	 * is not linearized.
	 */
	static Delinearizator * getInstance(const char * fileName, IPdfWriter * pdfWriter);

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
	
	/** Delinearizes pdf content.
	 * @param fileName Name of the output file.
	 *
	 * Opens given file (in trucate mode) and delegates the rest to 
	 * delinearize(FILE *) method. If given file doesn't exist, it will be
	 * created. Finally closes file.
	 *
	 * @see delinearize(FILE *)
	 *
	 * @return 0 if everything ok, otherwise value of errno of the error.
	 */
	int delinearize(const char * fileName);
	
	/** Delinearizes pdf content.
	 * @param file File handle where to put data.
	 *
	 * Sets position to the file beginning and writes same pdf header as in
	 * original stream. Then starts copying of all objects - except Linearization
	 * dictionary - which are available in XRef::entries array.
	 * Finally stores xref and trailer section.
	 * <br>
	 * Caller is responsible for file opening and closing.
	 * <br>
	 * NOTE that method doesn't check whether given file is same as one used for
	 * input data. If it refers to same file, result is unpredictable.
	 * <br>
	 * If no pdfWriter is specified (it is NULL), does nothing and immediatelly
	 * returns.
	 *
	 * @return 0 if everything ok, otherwise valie of error of the error.
	 */
	int delinearize(FILE * file);
};

} // end of namespace utils
} // end of pdfobjects namespace
#endif
