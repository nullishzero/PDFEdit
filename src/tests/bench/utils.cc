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
#include <stdlib.h>
#include <kernel/pdfedit-core-dev.h>
#include "utils.h"
const char *file_name;
int parse_cmd_line(int argc, char **argv)
{
	if(argc<2)
	{
		std::cerr << "Bad usage. Filename parameter expected" << std::endl;
		exit(EXIT_FAILURE);
	}

	// TODO use getopt instead and support several files
	file_name = argv[1];
	return 0;
}

int init_bench(int argc, char **argv)
{
  	if (pdfedit_core_dev_init(&argc, &argv))
  	{
  		// not able to initialize pdfedit-core-dev
  		return 1;
  	}
	return parse_cmd_line(argc, argv);
}

// result is in miliseconds
double time_diff(time_stamp_t &start, time_stamp_t &end)
{
	time_stamp_t diff;
	diff.tv_sec = end.tv_sec - start.tv_sec;
	diff.tv_usec = end.tv_usec - start.tv_usec;
	if(start.tv_usec > end.tv_usec)
	{
		--diff.tv_sec;
		diff.tv_usec += 1000000;
	}

	return diff.tv_sec*1000 + (double)diff.tv_usec/(double)1000;
}

void update_result(double time, struct result & result)
{
	if(time >= result.max_time)
		result.max_time = time;
	if (time <= result.min_time)
		result.min_time = time;
	result.sum_time += time;
	++(result.count);
	result.valid = true;
}

void print_results(FILE * out, struct result ** results)
{
using namespace std;
	for(struct result **iter=results; *iter; ++iter)
	{
		struct result * curr = *iter;
		fprintf(out, "%s", curr->name);
		if(!curr->valid)
		{
			fprintf(out, ":NO_RESULTS\n");
			continue;
		}
		double avg = (double)curr->sum_time/(double)curr->count;
		fprintf(out, ":max=%g:min=%g:avg=%g:count=%u\n", 
				curr->max_time, curr->min_time, avg, curr->count);
	}
}

int getFontId(boost::shared_ptr<pdfobjects::CPage> page, const std::string &fontName, std::string &fontId)
{
	pdfobjects::CPage::FontList fonts;
	page->getFontIdsAndNames(fonts);
	for(pdfobjects::CPage::FontList::const_iterator i = fonts.begin(); i != fonts.end(); ++i)
		if(i->second == fontName)
		{
			fontId = i->first;
			return 0;
		}
	return -1;
}
