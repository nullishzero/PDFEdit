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
#include <stdio.h>
#include "kernel/pdfedit-core-dev.h"
#include "kernel/delinearizator.h"
#include "kernel/pdfwriter.h"
#include "kernel/streamwriter.h"
#include "kernel/cxref.h"

using namespace pdfobjects;
using namespace pdfobjects::utils;

int delinearize(const char *input, const char *output)
{
	FILE *file=fopen(input, "r");
	Object dict;
	dict.initNull();
	boost::shared_ptr<Delinearizator> del = 
		Delinearizator::getInstance(input, new OldStylePdfWriter());
	if (!del) 
		return 1;
	int ret = del->delinearize(output);
	return ret;
}

int main(int argc, char ** argv)
{
	int ret;
	if(pdfedit_core_dev_init())
	{
		std::cerr << "Unable to initialize pdfedit-dev" << std::endl;
		return 1;
	}

	int opt;
	const char *input_file=NULL, *output_file=NULL;
	while ((opt = getopt(argc, argv, "i:o:")) != -1 )
	{
		switch(opt)
		{
			case 'i':
				input_file = optarg;
				break;
			case 'o':
				output_file = optarg;
				break;
			default:
				std::cerr << "Bad parameter" << std::endl;
		}
	}

	if(!input_file)
	{
		std::cerr << "Input file not specified"<< std::endl;
		return 1;
	}
	if(!output_file)
	{
		std::cerr << "Output file not specified"<< std::endl;
		return 1;
	}
	ret = delinearize(input_file, output_file);

	pdfedit_core_dev_destroy();
	return ret;
}
