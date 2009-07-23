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
#include <kernel/xrefwriter.h>
#include <kernel/pdfedit-core-dev.h>
#include <iostream>
#include "utils.h"

using namespace boost;
using namespace pdfobjects;
using namespace std;

#define open_and_get_xrefwriter(pdf, xref, filename) \
	do {					\
		pdf = open_file(filename);	\
		xref = dynamic_cast<XRefWriter*>(pdf->getCXref());\
	}while(0)

// change to all available revisions
void bench_changeRevision(XRefWriter * xref, struct result * result)
{
	time_stamp_t start, end;
	for (size_t i=1;i< xref->getRevisionCount(); ++i)
	{
		get_time_stamp(&start);
		xref->changeRevision(i);
		get_time_stamp(&end);
		if(result)
			update_result(time_diff(start, end), *result);
	}
}

void bench_knowsRef(XRefWriter * xref, struct result * result_known, struct result * result_unknown)
{
	// this should not influence performance of knowsRef
	int not_present=0, total=xref->getNumObjects();
	int obj_count = total;
	int objNum = 1;
	time_stamp_t start, end;
	IndiRef ref;
	// normally we should check only total>0, but this can lead to 
	// endless loop (until total overflows), because we are using
	// hardcoded gen=0 and there may be documents which use gen
	// number > 0
	for(;total>0 && (obj_count-not_present)>0;++objNum)
	{
		ref.num = objNum;
		get_time_stamp(&start);
		RefState state = xref->knowsRef(ref);
		get_time_stamp(&end);
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
		get_time_stamp(&start);
		RefState state = xref->knowsRef(ref);
		get_time_stamp(&end);
		assert(state == UNUSED_REF);
		if(result_unknown)
			update_result(time_diff(start, end), *result_unknown);
	}
}

// changes per% number of objects
void bench_changeObject(XRefWriter * xref, struct result *result, int per)
{
	int obj_count = xref->getNumObjects();
	int number = obj_count * per / 100;
	int not_present = 0;
	IndiRef ref;
	int objNum = 1;
	time_stamp_t start, end;
	// normally we should check only number>0, but this can lead to 
	// endless loop (until number overflows), because we are using
	// hardcoded gen=0 and there may be documents which use gen
	// number > 0
	for(;number>0 && (obj_count-not_present)>0;++objNum)
	{
		ref.num = objNum;
		RefState refState = xref->knowsRef(ref);
		if(refState != INITIALIZED_REF) 
		{
			++not_present;
			continue;
		}
		--number;
		Object orig_obj;
		xref->fetch(ref.num, ref.gen, &orig_obj);
		Object * changed_obj = orig_obj.clone();
		orig_obj.free();
		if(!changed_obj)
			continue;

		// use cloned object - this should be enough
		get_time_stamp(&start);
		xref->changeObject(ref.num, ref.gen, changed_obj);
		get_time_stamp(&end);
		if(result)
			update_result(time_diff(start, end), *result);
		xpdf::freeXpdfObject(changed_obj);
	}
}

void bench_fetch(XRefWriter *xref, struct result * result_known, struct result * result_unknown)
{
	// this should not influence performance of knowsRef
	int not_present=0, total=xref->getNumObjects();
	int obj_count = total;
	int objNum = 1;
	time_stamp_t start, end;
	IndiRef ref;
	// normally we should check only total>0, but this can lead to 
	// endless loop (until total overflows), because we are using
	// hardcoded gen=0 and there may be documents which use gen
	// number > 0
	for(;total>0 && (obj_count-not_present)>0;++objNum)
	{
		ref.num = objNum;
		RefState state = xref->knowsRef(ref);
		Object obj;
		get_time_stamp(&start);
		xref->fetch(ref.num, ref.gen, &obj);
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
		RefState state = xref->knowsRef(ref);
		Object obj;
		get_time_stamp(&start);
		xref->fetch(ref.num, ref.gen, &obj);
		get_time_stamp(&end);
		obj.free();
		assert(state == UNUSED_REF);
		if(result_unknown)
			update_result(time_diff(start, end), *result_unknown);
	}

}

int main(int argc, char ** argv)
{
	int ret;

	if((ret = init_bench(argc, argv)))
		return ret;

	time_stamp_t start, end;
	shared_ptr<CPdf> pdf;
	XRefWriter * xref;

	// changeRevision test
	open_and_get_xrefwriter(pdf, xref, file_name);
	DEFINE_RESULTS(changeRevisionResults1, "changeRevision_no_changed");
	bench_changeRevision(xref, &changeRevisionResults1);

	// knowsRef test without any changed objects
	open_and_get_xrefwriter(pdf, xref, file_name);
	DEFINE_RESULTS(knowsRef_known1, "knowsRef_known_no_changed");
	DEFINE_RESULTS(knowsRef_unknown1, "knowsRef_unknown_no_changed");
	bench_knowsRef(xref, &knowsRef_known1, &knowsRef_unknown1);

	// knowsRef test with all indirect object changed
	open_and_get_xrefwriter(pdf, xref, file_name);
	DEFINE_RESULTS(knowsRef_known2, "knowsRef_known_all_changed");
	DEFINE_RESULTS(knowsRef_unknown2, "knowsRef_unknown_all_changed");
	DEFINE_RESULTS(changeObject_all, "changeObject_all");
	// change all objects bench
	if(pdf->getMode() != CPdf::ReadOnly)
	{
		bench_changeObject(xref, &changeObject_all, 100);
		bench_knowsRef(xref, &knowsRef_known2, &knowsRef_unknown2);
	}
	
	// fetch without changed objects
	open_and_get_xrefwriter(pdf, xref, file_name);
	DEFINE_RESULTS(fetch_known1, "fetch_known_no_changed");
	DEFINE_RESULTS(fetch_unknown1, "fetch_unknown_no_changed");
	bench_fetch(xref, &fetch_known1, &fetch_unknown1);
	
	// fetch with all objects changed
	open_and_get_xrefwriter(pdf, xref, file_name);
	DEFINE_RESULTS(fetch_known2, "fetch_known_all_changed");
	DEFINE_RESULTS(fetch_unknown2, "fetch_unknown_all_changed");
	if(pdf->getMode() != CPdf::ReadOnly)
	{
		bench_changeObject(xref, NULL, 100);
		bench_fetch(xref, &fetch_known2, &fetch_unknown2);
	}

	// clone (???)
	// reserveRef (RESERVED_NUMBER)
	struct result *all_results [] = {
		&changeRevisionResults1, 
		&knowsRef_known1, &knowsRef_unknown1,
		&knowsRef_known2, &knowsRef_unknown2,
		&changeObject_all,
		&fetch_known1, &fetch_unknown1,
		&fetch_known2, &fetch_unknown2,
		NULL
	};

	print_results(stdout, all_results);

	// finally prints xpdf memory debug information if available (DEBUG_MEM
	// macro is defined during compilation)
	// last pdf must be reseted because we need to drop the last reference
	// to deallocate CPdf
	pdf.reset();
	fprintf(stdout, "\n---\n");
	gMemReport(stdout);
	return 0;
}
