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
#include <kernel/cpage.h>
#include <kernel/factories.h>
#include "utils.h"

using namespace boost;
using namespace pdfobjects;
using namespace std;

void bench_get_ccstreams(shared_ptr<CPdf> pdf, struct result *results, int startPage, int pageCount)
{
	for(int p=startPage; p < startPage+pageCount; ++p)
	{
		shared_ptr<CPage> page = pdf->getPage(p);
		vector<shared_ptr<CContentStream> > cs;
		time_stamp_t start,  end;
		get_time_stamp(&start);
		page->getContentStreams(cs);
		get_time_stamp(&end);
		if (results)
			update_result(time_diff(start, end), *results);
	}
}

void addText(shared_ptr<CPage> page, double x, double y, std::string &fontName, std::string &text)
{
	// copy of operatorAddTextLine script function with
	// font: PDFEDIT_F1
	// color: #000000
	double fontSize = 10;
	shared_ptr<UnknownCompositePdfOperator> q(new UnknownCompositePdfOperator("q", "Q"));
	shared_ptr<UnknownCompositePdfOperator> BT(new UnknownCompositePdfOperator("BT", "ET"));
	PdfOperator::Operands fontOperands;
	fontOperands.push_back(shared_ptr<IProperty>(CNameFactory::getInstance(fontName)) );
	fontOperands.push_back(shared_ptr<IProperty>(CRealFactory::getInstance(fontSize)));
	q->push_back(BT,q);
	BT->push_back(createOperator("Tf", fontOperands), getLastOperator(BT));
	PdfOperator::Operands posOperands;
	posOperands.push_back(shared_ptr<IProperty>(CRealFactory::getInstance(x)));
	posOperands.push_back(shared_ptr<IProperty>(CRealFactory::getInstance(y)));
	BT->push_back(createOperator("Td", posOperands), getLastOperator(BT));
	PdfOperator::Operands textOperands;
	textOperands.push_back(shared_ptr<IProperty>(CStringFactory::getInstance(text)));
	BT->push_back(createOperator("Tj", textOperands), getLastOperator(BT));
	PdfOperator::Operands emptyOperands;
	BT->push_back(createOperator("ET", emptyOperands), getLastOperator(BT));
	q->push_back(createOperator("Q", emptyOperands), getLastOperator(q));
	std::deque<shared_ptr<PdfOperator> > stack;
	stack.push_back(q);
	page->addContentStreamToFront(stack);
}

void bench_addTextToStream(shared_ptr<CPdf> pdf, const std::string &fontName, struct result *results, int p, int numberOfAdditions)
{
	shared_ptr<CPage> page = pdf->getPage(p);
	std::string fontId; 
	if(getFontId(page, fontName, fontId))
		fontId = page->addSystemType1Font(fontName);
	time_stamp_t start,  end;
	std::string text="Foooo";
	// don't include time for parsing existing content streams on the page
	bench_get_ccstreams(pdf, NULL, p, 1);
	for(int iter = 0; iter < numberOfAdditions; ++iter)
	{
		get_time_stamp(&start);
		addText(page, 10, 10, fontId, text);
		get_time_stamp(&end);
		if (results)
			update_result(time_diff(start, end), *results);
	}
}

int main(int argc, char ** argv)
{
	int ret;
	std::string fontName = "Times-Roman";

	if((ret = init_bench(argc, argv)))
		return ret;

	time_stamp_t start, end;
	shared_ptr<CPdf> pdf;

	pdf = open_file(file_name);

	DEFINE_RESULTS(getCStreams_first, "getCStreams_first");
	int pageCount = pdf->getPageCount();
	bench_get_ccstreams(pdf, &getCStreams_first, 1, pageCount);
	DEFINE_RESULTS(getCStreams_again, "getCStreams_again");
	bench_get_ccstreams(pdf, &getCStreams_again, 1, pageCount);

	// add text on the clean pdf
	pdf = open_file(file_name);
	DEFINE_RESULTS(addTextToStream1, "addToStream1");
	bench_addTextToStream(pdf, fontName, &addTextToStream1, 1, 1);

	pdf = open_file(file_name);
	DEFINE_RESULTS(addTextToStream10, "addToStream10");
	bench_addTextToStream(pdf, fontName, &addTextToStream10, 1, 10);

	pdf = open_file(file_name);
	DEFINE_RESULTS(addTextToStream100, "addToStream100");
	bench_addTextToStream(pdf, fontName, &addTextToStream100, 1, 100);

	pdf = open_file(file_name);
	DEFINE_RESULTS(addTextToStream1000, "addToStream1000");
	bench_addTextToStream(pdf, fontName, &addTextToStream1000, 1, 1000);

	// make changes cumulative
	pdf = open_file(file_name);
	DEFINE_RESULTS(addTextToStream1cumulative, "addToStream1cumulative");
	bench_addTextToStream(pdf, fontName, &addTextToStream1cumulative, 1, 1);

	DEFINE_RESULTS(addTextToStream10cumulative, "addToStream10cumulative");
	bench_addTextToStream(pdf, fontName, &addTextToStream10cumulative, 1, 10);

	DEFINE_RESULTS(addTextToStream100cumulative, "addToStream100cumulative");
	bench_addTextToStream(pdf, fontName, &addTextToStream100cumulative, 1, 100);

	DEFINE_RESULTS(addTextToStream1000cumulative, "addToStream1000cumulative");
	bench_addTextToStream(pdf, fontName, &addTextToStream1000cumulative, 1, 1000);

	pdf.reset();
	struct result *all_results [] = {
		&getCStreams_first,
		&getCStreams_again,
		&addTextToStream1,
		&addTextToStream10,
		&addTextToStream100,
		&addTextToStream1000,
		&addTextToStream1cumulative,
		&addTextToStream10cumulative,
		&addTextToStream100cumulative,
		&addTextToStream1000cumulative,
		NULL
	};

	print_results(stdout, all_results);
	fprintf(stdout, "\n---\n");
	gMemReport(stdout);
	return 0;
}
