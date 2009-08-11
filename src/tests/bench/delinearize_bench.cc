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
#include <kernel/delinearizator.h>
#include <kernel/pdfedit-core-dev.h>
#include "utils.h"

using namespace pdfobjects;
using namespace utils;
int main(int argc, char **argv)
{
	int ret;

	if(pdfedit_core_dev_init(&argc, &argv))
		return 1;

	if((ret = init_bench(argc, argv)))
		return ret;

	boost::shared_ptr<Delinearizator> delin = Delinearizator::getInstance(file_name, new OldStylePdfWriter());

	// check for existing file and remove it
	std::string output_file = file_name+std::string("-delinearized.pdf");
	time_stamp_t start, end;
	DEFINE_RESULTS(delinearize, "delinearize");

	get_time_stamp(&start);
	delin->delinearize(output_file.c_str());
	get_time_stamp(&end);
	update_result(time_diff(start, end), delinearize);
	struct result *all_results [] = {
		&delinearize,
		NULL
	};
	print_results(stdout, all_results);

	fprintf(stdout, "\n---\n");
	gMemReport(stdout);
	return 0;


}
