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
#include <boost/program_options.hpp>
#include "kernel/pdfedit-core-dev.h"
#include "kernel/delinearizator.h"
#include "kernel/pdfwriter.h"
#include "kernel/streamwriter.h"
#include "kernel/cxref.h"

using namespace std;
using namespace pdfobjects;
using namespace pdfobjects::utils;
using namespace boost;
namespace po = program_options;

int delinearize(const char *input, const char *output)
{
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

	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce help message")
		("file", po::value<string>(), "Input pdf file")
		("output", po::value<string>(), "Output pdf file")
	;
	
	po::variables_map vm;
	try {
		po::store(po::parse_command_line(argc, argv, desc), vm);
		po::notify(vm);    
	}catch(std::exception& e)
	{
		std::cout << "exception - " << e.what() << ". Please, check your parameters." << endl;
		return 1;
	}   

		if (vm.count("help") || !vm.count("file") || !vm.count("output"))
		{
			cout << desc << "\n";
			return 1;
		}

	string input_file = vm["file"].as<string>(); 
	string output_file = vm["output"].as<string>();

	ret = delinearize(input_file.c_str(), output_file.c_str());

	pdfedit_core_dev_destroy();
	return ret;
}
