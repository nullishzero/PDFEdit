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
#ifndef _BENCH_UTILS_H_
#define _BENCH_UTILS_H_

#include <kernel/static.h>
#include <kernel/cpdf.h>
#include <kernel/cpage.h>
#include <sys/time.h>
#include <time.h>
#include <boost/shared_ptr.hpp>
#include <limits.h>

extern const char * file_name;

int init_bench(int argc, char **argv);

// redefine if something different than gettimeofday should
// be used for time measuring
typedef struct timeval time_stamp_t;
double time_diff(time_stamp_t &start, time_stamp_t &end);
// gets current time stamp - pointer to timeval struct
#define get_time_stamp(val)	gettimeofday(val, NULL)

struct result
{
	double max_time;
	double min_time;
	double sum_time;
	unsigned count;
	const char * name;
	bool valid;
};

#define DEFINE_RESULTS(var, name) struct result var = {0,LONG_MAX,0,0,name,false}

void update_result(double time, struct result & result);
void print_results(FILE * out, struct result ** results);


static inline boost::shared_ptr<pdfobjects::CPdf> open_file(
		const char * name, 
		pdfobjects::CPdf::OpenMode mode = pdfobjects::CPdf::Advanced)
{
	return pdfobjects::CPdf::getInstance(name, mode);
}

// TODO something like this should be part of standard API
int getFontId(boost::shared_ptr<pdfobjects::CPage> page, const std::string &fontName, std::string &fontId);

#endif

