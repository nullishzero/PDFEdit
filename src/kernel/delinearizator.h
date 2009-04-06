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
#ifndef _DELINEARIZATOR_H_
#define _DELINEARIZATOR_H_

#include "kernel/static.h"
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

	virtual const char * what()const throw()
	{
		return "Content is not linerized";
	}
};

class FileStreamWriter;

namespace pdfobjects
{

namespace utils
{
class IPdfWriter;

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
 * boost::shared_ptr<Delinearizator> delinearizator=Delinearizator::getInstance(fileName, contentWriter);
 *
 * // check for encryption and set credentials if necessary
 * if (delinearizator->isEncrypted())
 * 	delinearizator->setCredentials(ownerPasswd, userPasswd);
 *
 * // delinearize file content to file specified by name
 * delinearizator->delinearize(outputFile);
 * 
 * ...
 * 
 * // instance is wrapped by the smart pointer so you don't
 * // have bother with deallocation
 * </pre>
 */
class Delinearizator: public PdfDocumentWriter
{
	/** Reference of linearized dictionary.
	 */
	::Ref linearizedRef;

 	/** Marker of the last object returned by fillObjectList.
	 * Zeroed in writeDocument method.
	 */
 	int lastObj;

	/** Initialization constructor.
	 * @param streamData Outpu stream data.
	 * @param writer Pdf content writer.
	 *
	 * Delegates all the work to the PdfDocumentWriter constructor and 
	 * additionaly checks whether document is linearized.
	 * 
	 * @throw MalformedFormatExeption if file content is not valid pdf document.
	 * @throw NotLinearizedException if file content is not linearized.
	 */
	Delinearizator(FileStreamData &streamData, IPdfWriter * writer);

	/** Destructor.
	 *
	 * Deallocates pdfWriter and delegates the rest to ~CXref.
	 */
	virtual ~Delinearizator() {}

	friend class FileStreamDataDeleter<Delinearizator>;


	/** Provides all objects for delinearized document.
	 * @param objectList Container for objects.
	 * @param maxObjectCount Maximum objects count to be filled 
	 * 	into the objectList.
	 *
	 * Provides a deep copy of all objects availble in XRef::etries 
	 * array except for Linearization dictionary. Consequential calls
	 * will continue from the last seen provided object.
  	 *
	 * @return number of objects filled to the objectList.
  	 */
	virtual int fillObjectList(IPdfWriter::ObjectList &objectList, int maxObjectCount);
public:
	
	/** Factory method for instance creation.
	 * @param fileName Name of the pdf file.
	 * @param pdfWriter  Pdf content writer.
	 *
	 * Creates FileStream from given fileName (file is open with `r' mode) and 
	 * creates Delinearizator instance. Finally checks whether file is 
	 * linearized and if not, prints error message and returns with NULL.
	 * <br>
	 * Smart pointer returned by this method contains a proper deleter which
	 * closes file handle created from given fileName.
	 *
	 * @throw MalformedFormatExeption if file content is not valid pdf document.
	 * @return Delinearizator instance ready to be used or NULL, if given file
	 * is not linearized.
	 */
	static boost::shared_ptr<Delinearizator> getInstance(const char * fileName, 
			IPdfWriter * pdfWriter);

	/** Delinearizes pdf content to the given file.
	 * @param fileName Output file name.
	 *
	 * Delegates to PdfDocumentWriter::writeDocument(const char*).
	 *
	 * @return 0 on success, errno otherwise.
	 * @throw NotImplementedException if document is encrypted.
	 * @throw MalformedFormatExeption if the input file is currupted.
	 */
	int delinearize(const char * fileName);
	
	/** Delinearizes pdf content to the given file.
	 * @param file File handle where to put data.
	 *
	 * Delegates to PdfDocumentWriter::writeDocument(FILE*).
	 * @throw NotImplementedException if document is encrypted.
	 * @throw MalformedFormatExeption if the input file is currupted.
	 */
	int delinearize(FILE * file);
};

} // end of namespace utils
} // end of pdfobjects namespace
#endif
