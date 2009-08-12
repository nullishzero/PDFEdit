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
#include <kernel/cpdf.h>
#include <kernel/xrefwriter.h>
#include "utils.h"

using namespace boost;
using namespace pdfobjects;
using namespace std;

void print_info(shared_ptr<CPdf> pdf, FILE * out)
{
	XRefWriter * xref;
	xref = dynamic_cast<XRefWriter*>(pdf->getCXref());

	// linearized information
	fprintf(out, "\tlinearized: %s\n", (pdf->isLinearized())?"true":"false");

	// encryption
	bool encrypted = utils::isEncrypted(pdf);
	fprintf(out, "\tencrypted: %s\n", (encrypted)?"true":"false");
	if(encrypted)
		return;

	// revisions
	size_t revisionCount = pdf->getRevisionsCount();
	fprintf(out, "\tNumber of revisions: %d\n", revisionCount);
	size_t rev = 0;
	do
	{
		fprintf(out, "\t\trevision %u size: %lu\n", rev, 
				(unsigned long)pdf->getRevisionSize(rev));
	}while(++rev < revisionCount);

	// page information
	fprintf(out, "\tPage count: %lu\n", (unsigned long)pdf->getPageCount());

	// TODO detailed info about pages - number, size, type of content streams

	// indirect objects information
	fprintf(out, "\tIndirect objects: %d\n", xref->getNumObjects());

	// TODO number of direct objects - transitive from document catalog
}

int main(int argc, char ** argv)
{
	int ret;
	FILE * out = stdout;
	if((ret = init_bench(argc, argv)))
		return ret;

	ret = 0;
	while(--argc > 0)
	{
		argv++;
		fprintf(out, "Document information: \"%s\"\n", *argv);
		try 
		{
			print_info(open_file(*argv), out);
		}catch(...)
		{
			fprintf(stderr, "Unable to process file\n");
			ret++;
		}
	}
	return ret;
}
