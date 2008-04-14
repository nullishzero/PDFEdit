/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006, 2007, 2008  PDFedit team: Michal Hocko,
 *                                              Miroslav Jahoda,
 *                                              Jozef Misutka,
 *                                              Martin Petricek
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
#include "utils.h"

using namespace boost;
using namespace pdfobjects;
using namespace std;

// measures getIndirectProperty time same way as in XRefWriter::fetch method
void bench_getIndirectProperty(shared_ptr<CPdf> pdf,
		struct result *result_known, struct result *result_unknown)
{
	XRefWriter *helperXref = dynamic_cast<XRefWriter *>(pdf->getCXref());
	int not_present=0, total=helperXref->getNumObjects();
	int obj_count = total;
	int objNum = 1;
	time_stamp_t start, end;
	IndiRef ref;
	for(;total>0;++objNum)
	{
		ref.num = objNum;
		RefState state = helperXref->knowsRef(ref);
		Object obj;
		get_time_stamp(&start);
		shared_ptr<IProperty> prop = pdf->getIndirectProperty(ref);
		get_time_stamp(&end);

		obj.free();
		if(state == UNUSED_REF)
		{
			if(result_unknown)
				update_result(time_diff(start, end), *result_unknown);
			++not_present;
		}
		else
		{
			// RESERVED_REF should never happen here because
			// we no half added objects (reserved, but not added) should
			// be present
			assert(state == INITIALIZED_REF);
			if(result_known)
				update_result(time_diff(start, end), *result_known);
			--total;
		}
	}

	// everything higher is not reserved
	for (;(obj_count-not_present)>0; ++not_present)
	{
		ref.num = objNum;
		RefState state = helperXref->knowsRef(ref);
		Object obj;
		get_time_stamp(&start);
		shared_ptr<IProperty> prop = pdf->getIndirectProperty(ref);
		get_time_stamp(&end);
		obj.free();
		assert(state == UNUSED_REF);
		if(result_unknown)
			update_result(time_diff(start, end), *result_unknown);
	}
}

void bench_changeIndirectObject(shared_ptr<CPdf> pdf, struct result *result, int per)
{
	XRefWriter * xref = dynamic_cast<XRefWriter*>(pdf->getCXref());
	int number = xref->getNumObjects() * per / 100;
	IndiRef ref;
	int objNum = 1;
	time_stamp_t start, end;
	for (;number>0; ++objNum)
	{
		ref.num = objNum;
		RefState refState = xref->knowsRef(ref);
		if(refState != INITIALIZED_REF)
			continue;
		--number;
		shared_ptr<IProperty> orig_obj = pdf->getIndirectProperty(ref);
		shared_ptr<IProperty> changed_obj = orig_obj->clone();
		if(!changed_obj)
			continue;
		changed_obj->setPdf(orig_obj->getPdf());
		changed_obj->setIndiRef(orig_obj->getIndiRef());

		// use cloned object - this should be enough
		get_time_stamp(&start);
		pdf->changeIndirectProperty(changed_obj);
		get_time_stamp(&end);
		if(result)
			update_result(time_diff(start, end), *result);
	}
}
struct PagePosition
{
	enum PosType {FRONT, CURR, END};
	PosType type;
	PagePosition(PosType t): type(t) {}
	size_t operator ()(shared_ptr<CPdf> pdf, shared_ptr<CPage> page)
	{
		switch(type)
		{
			case FRONT:
				return 1;
			case CURR:
				return pdf->getPagePosition(page);
			case END:
				return pdf->getPageCount();
		}
		return -1UL;
	}
};

// inserts per% of pages from original_pdf to the pdf at position specified
// by storePosition
void bench_insertPage(shared_ptr<CPdf> pdf, shared_ptr<CPdf> original_pdf, 
		struct result *result, struct PagePosition storePosition, 
		int per)
{
	int number = original_pdf->getPageCount() * per / 100;
	int page_num = 1;
	time_stamp_t start, end;
	for (;number>0; ++page_num, --number)
	{
		shared_ptr<CPage> page = original_pdf->getPage(page_num);
		// count also already stored pages
		size_t pos = storePosition(original_pdf, page) + page_num - 1;
		get_time_stamp(&start);
		pdf->insertPage(page, pos);
		get_time_stamp(&end);
		if(result)
			update_result(time_diff(start, end), *result);
	}
}

// removes per% of pages from the position specified by removePosition
// copy_pdf is used to determine page position to remove (should be same
// document in different CPdf instance) so that we don't influence results
// by fetching page tree indirect objects.
void bench_removePage(shared_ptr<CPdf> pdf, shared_ptr<CPdf> copy_pdf,
		struct result *result, struct PagePosition removePosition, 
		int per)
{
	int number = copy_pdf->getPageCount() * per / 100;
	int page_num = 1;
	time_stamp_t start, end;
	for (;number>0; ++page_num, --number)
	{
		shared_ptr<CPage> page = copy_pdf->getPage(page_num);
		// count also already removed pages if we are removing
		// from the end or current position
		size_t pos = removePosition(copy_pdf, page);
		pos -= (removePosition.type != PagePosition::FRONT)?(page_num - 1):0;
		get_time_stamp(&start);
		pdf->removePage(pos);
		get_time_stamp(&end);
		if(result)
			update_result(time_diff(start, end), *result);
	}
}

// measures page forward iteration (hasNextPage && getNextPage starting
// from getFirstPage)
void bench_fwd_iter(shared_ptr<CPdf> pdf, struct result * result)
{
	timeval start, end;
	shared_ptr<CPage> page;
	get_time_stamp(&start);
	page = pdf->getFirstPage();
	get_time_stamp(&end);
	if(result)
		update_result(time_diff(start, end), *result);

	get_time_stamp(&start);
	while(pdf->hasNextPage(page))
	{
		page = pdf->getNextPage(page);
		get_time_stamp(&end);
		if(result)
			update_result(time_diff(start, end), *result);
		get_time_stamp(&start);
	}
	// we don't use last unseccessfull hasNextPage
}

// measures page forward iteration (hasPrevPage && gePrevPage starting
// from getLastPage)
void bench_bwd_iter(shared_ptr<CPdf> pdf, struct result * result)
{
	timeval start, end;
	shared_ptr<CPage> page;
	get_time_stamp(&start);
	page = pdf->getLastPage();
	get_time_stamp(&end);
	if(result)
		update_result(time_diff(start, end), *result);

	get_time_stamp(&start);
	while(pdf->hasPrevPage(page))
	{
		page = pdf->getPrevPage(page);
		get_time_stamp(&end);
		if(result)
			update_result(time_diff(start, end), *result);
		get_time_stamp(&start);
	}

	// we don't use last unseccessfull hasPrevPage
}

int main(int argc, char **argv)
{
	int ret;
	if((ret = parse_cmd_line(argc, argv)))
		return ret;
	timeval start, end;
	shared_ptr<CPdf> pdf;

	DEFINE_RESULTS(getInstance, "getInstance");
	get_time_stamp(&start);
	pdf = open_file(file_name);
	get_time_stamp(&end);
	update_result(time_diff(start, end), getInstance);

	// get all indirect properties and check also those which 
	// are not present
	DEFINE_RESULTS(getIndirectProperty_known_no_changes1,"getIndirectProperty_known_no_changed_first");
	DEFINE_RESULTS(getIndirectProperty_unknown_no_changes1,"getIndirectProperty_unknown_no_changed_first");
	shared_ptr<CPdf> helperPdf = open_file(file_name);
	bench_getIndirectProperty(pdf,
			&getIndirectProperty_known_no_changes1, 
			&getIndirectProperty_unknown_no_changes1);

	// repeat again on the same instance - this test can
	// show big internal caching performance grow, because all
	// indirect properties are cached so that repeated getIndirectProperty
	// with the same indirect number has to return the same object
	// (if a reference to it still exists)
	DEFINE_RESULTS(getIndirectProperty_known_no_changes2,"getIndirectProperty_known_no_changed_again");
	DEFINE_RESULTS(getIndirectProperty_unknown_no_changes2,"getIndirectProperty_unknown_no_changed_again");
	bench_getIndirectProperty(pdf,
			&getIndirectProperty_known_no_changes2, 
			&getIndirectProperty_unknown_no_changes2);

	// changeIndirectProperty to all properties - we simply create
	// deep copy and call changeIndirectProperty
	pdf = open_file(file_name);
	DEFINE_RESULTS(changeIndirectProperty_all1,"changeIndirectProperty_all_first");
	bench_changeIndirectObject(pdf, &changeIndirectProperty_all1, 100);
	
	// get all indirect objects after they have been changed
	DEFINE_RESULTS(getIndirectProperty_known_all_changes,"getIndirectProperty_known_all_changed");
	DEFINE_RESULTS(getIndirectProperty_unknown_all_changes,"getIndirectProperty_unknown_all_changed");
	bench_getIndirectProperty(pdf,
			&getIndirectProperty_known_all_changes, 
			&getIndirectProperty_unknown_all_changes);

	// we already maintain all changed object without xpdf code so
	// we should measure only our performance here
	DEFINE_RESULTS(changeIndirectProperty_all2,"changeIndirectProperty_all_again");
	bench_changeIndirectObject(pdf, &changeIndirectProperty_all2, 100);
	
	// TODO addIndirectProperty - something reasonable big with many indirect
	// entries - e.g. page tree root
	
	// getPageCount
	DEFINE_RESULTS(getPageCount, "getPageCount");
	pdf = open_file(file_name);
	get_time_stamp(&start);
	pdf->getPageCount();
	get_time_stamp(&end);
	update_result(time_diff(start, end), getPageCount);

	// page iteration - forward + hasNextPage
	// 		  - backward + hasPrevPage
	// 		  - first/last
	DEFINE_RESULTS(page_fwd_iteration, "page_forward_iteration");
	DEFINE_RESULTS(page_bwd_iteration, "page_backward_iteration");
	pdf = open_file(file_name);
	bench_fwd_iter(pdf, &page_fwd_iteration);
	pdf = open_file(file_name);
	bench_bwd_iter(pdf, &page_bwd_iteration);

	// TODO getPagePosition
	
	// insertPage - same document opened in different CPdf all pages
	// are inserted to the back and front
	pdf = open_file(file_name);
	shared_ptr<CPdf> copy_pdf = open_file(file_name);
	DEFINE_RESULTS(insertPage_all_end, "insertPage_all_end");
	bench_insertPage(pdf, copy_pdf, &insertPage_all_end, PagePosition(PagePosition::END), 100);

	pdf = open_file(file_name);
	copy_pdf = open_file(file_name);
	DEFINE_RESULTS(insertPage_all_front, "insertPage_all_front");
	bench_insertPage(pdf, copy_pdf, &insertPage_all_front, PagePosition(PagePosition::FRONT), 100);

	// removePage from back, front
	pdf = open_file(file_name);
	DEFINE_RESULTS(removePage_all_end, "removePage_all_end");
	bench_removePage(pdf, copy_pdf, 
			&removePage_all_end, PagePosition(PagePosition::END), 100);

	pdf = open_file(file_name);
	DEFINE_RESULTS(removePage_all_front, "removePage_all_front");
	bench_removePage(pdf, copy_pdf, 
			&removePage_all_front, PagePosition(PagePosition::FRONT), 100);

	// TODO changeRevision

	struct result *all_results [] = {
		&getInstance,
		&getIndirectProperty_known_no_changes1, 
		&getIndirectProperty_unknown_no_changes1,
		&getIndirectProperty_known_no_changes2, 
		&getIndirectProperty_unknown_no_changes2,
		&changeIndirectProperty_all1,
		&getIndirectProperty_known_all_changes,
		&getIndirectProperty_unknown_all_changes,
		&changeIndirectProperty_all2,
		&getPageCount,
		&page_fwd_iteration,
		&page_bwd_iteration,
		&insertPage_all_end,
		&insertPage_all_front,
		&removePage_all_end,
		&removePage_all_front,
		NULL
	};
	print_results(all_results);
	return 0;
}
