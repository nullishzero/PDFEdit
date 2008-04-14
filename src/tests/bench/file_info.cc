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
	bool encrypted = utils::isEncrypted(*pdf, NULL);
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
	if((ret = parse_cmd_line(argc, argv)))
		return ret;
	fprintf(out, "Document information: \"%s\"\n", file_name);
	print_info(open_file(file_name), out);
	return 0;
}
