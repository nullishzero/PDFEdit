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

#include "common.h"
#include <boost/shared_ptr.hpp>
#include <kernel/pdfedit-core-dev.h>
#include <kernel/cpdf.h>
#include <kernel/cpage.h>
#include <string>
#include <boost/program_options.hpp>

using namespace pdfobjects;
using namespace std;
using namespace boost;
namespace po = program_options;

void print_pages(const char *fname, PagePosList &pagePosList)
{
	boost::shared_ptr<CPdf> pdf;
	try
	{
		pdf = CPdf::getInstance(fname, CPdf::ReadOnly);
	}catch(...)
	{
		std::cerr << fname << " is not a valid pdf document" << std::endl;
		exit(EXIT_FAILURE);
	}

	std::cout << "Document: \"" << fname << "\"" << std::endl;
	for(PagePosList::iterator pagePos=pagePosList.begin();
			pagePos != pagePosList.end();
			++pagePos)
	{
		boost::shared_ptr<CPage> page;
		std::cout << "Page #" << *pagePos << ": ";
		try
		{
			page = pdf->getPage(*pagePos);
		}catch (PageNotFoundException &)
		{
			std::cout << "Not found" << std::endl;
			continue;
		}
		boost::shared_ptr<CDict> pageDict = page->getDictionary();
		IndiRef ref = pageDict->getIndiRef();
		std::cout << "[" << ref << "]"
			<< std::endl;
	}
}

int main(int argc, char** argv)
{
	if(pdfedit_core_dev_init())
	{
		std::cerr << "Unable to initialize pdfedit-dev" << std::endl;
		return 1;
	}

	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "Produce help message")
		("file", po::value<string>(), "Input pdf file")
		("range", po::value<string>(), "Page range")
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

		if (!vm.count("file") || !vm.count("range")) 
		{
			cout << desc << endl;
			return 1;
		}
	string file = vm["file"].as<string>(); 
	string range = vm["range"].as<string>(); 
	PagePosList pagePosList;
	if(add_page_range(pagePosList, range.c_str()))
	{
		std::cerr << range 
			<< " is not a valid page" 
			<< std::endl;
		exit(EXIT_FAILURE);
	}

	print_pages(file.c_str(), pagePosList);

	pdfedit_core_dev_destroy();
	return 0;
}
