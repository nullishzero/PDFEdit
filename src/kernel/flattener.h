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
#ifndef _FLATTENER_H_
#define _FLATTENER_H_

#include "kernel/xpdf.h"
#include "kernel/exceptions.h"
#include "kernel/pdfwriter.h"

namespace pdfobjects 
{
namespace utils
{

/** Flattener class.
 * Provides functionality to write the new PDF document with the 
 * all reachable objects from the original document in the single 
 * revision.
 * <br>
 * This can be usefull for several reasons:
 * <ul>
 * <li>To get rid of older revisions
 * <li>To get rid of objects which cannot affect document displaying (because
 * they are not reachable) and so make the result file smaller.
 * <li>To hide changes made during editing (e.g. they are confidential)
 * </ul>
 * <p>
 * <b>Usage</b>
 * Use static factory method for instance creation:
 * <pre>
 * // we will use OldStylePdfWriter IPdfWriter implementator
 * IPdfWriter * contentWriter=new OldStylePdfWriter();
 * boost::shared_ptr<Flattener> flattener = Flattener::getInstance(fileName, contentWriter);
 *
 * // check for encryption and set credentials if necessary
 * if (flattener->isEncrypted())
 * 	flattener->setCredentials(ownerPasswd, userPasswd);
 *
 * // flatten file content to the file specified by name
 * flattener->delinearize(outputFile);
 *
 * ...
 *
 * // instance is wrapped by the smart pointer so you don't
 * // have bother with deallocation
 * </pre>
 */
class Flattener: public PdfDocumentWriter
{
public:
	// TODO maybe some more lookup efficient structure
	typedef std::vector<Ref> RefList;

	/** List of all reachable indirect objects.
	 * Initialized in initReachableObjects.
	 */
	RefList reachAbleRefs;

private:
	/** Index of the last in the reachAbleRefs returned object by fillObjectList.
	 * Zeroed in flatten methods.
	 */
	size_t lastIndex;

	virtual ~Flattener() {};

	// deallocator for this class
	friend class FileStreamDataDeleter<Flattener>;

protected:
	Flattener(FileStreamData &streamData, IPdfWriter * writer);

	/** Initializes all reachable objects.
	 *
	 * Starts with the Trailer and recursively travels all reachable
	 * indirect objects which are stored in reachAbleRefs container.
	 */
	void initReachableObjects();

	/** Fills up to given maxObjectCount into the given list.
	 * @param objectList Container for objects.
	 * @param maxObjectCount Maximum objects count to be filled.
	 *
	 * Note that given list is cleared at the beggining.
	 * @return number of objects filled into the container.
	 */
	virtual int fillObjectList(IPdfWriter::ObjectList &objectList, int maxObjectCount);
public:
	/** Factory method.
	 * @param fileName Input PDF document.
	 * @param pdfWriter PDF Writer to be used for content writing.
	 * @throw MalformedFormatExeption if file content is not valid pdf document.
	 * @return Instance ready to be used.
	 */
	static boost::shared_ptr<Flattener> getInstance(const char * fileName, IPdfWriter * pdfWriter);

	/** Flattens this document and puts the result into the given file.
	 * @param fileName Output file name.
	 *
	 * Delegates to PdfDocumentWriter::writeDocument(const char*).
	 *
	 * @return 0 on success, errno otherwise.
	 * @throw NotImplementedException if document is encrypted.
	 * @throw MalformedFormatExeption if the input file is currupted.
	 */
	int flatten(const char *fileName);

	/** Flattens this document and puts the result into the given file.
	 * @param file File handle where to put data.
	 *
	 * Delegates to PdfDocumentWriter::writeDocument(FILE*).
	 * @throw NotImplementedException if document is encrypted.
	 * @throw MalformedFormatExeption if the input file is currupted.
	 */
	int flatten(FILE * file);

};

} // namespace utils
} // namespace pdfobjects

#endif

