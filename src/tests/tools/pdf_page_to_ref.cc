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
#include "common.h"
#include <kernel/cpage.h>

using namespace pdfobjects;
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

	int opt;
	PagePosList pagePosList;
	while ((opt = getopt(argc, argv, "p:")) != -1 )
	{
		switch (opt)
		{
			case 'p':
				if(add_page_range(pagePosList, optarg))
				{
					std::cerr << optarg 
						<< " is not a valid page" 
						<< std::endl;
					exit(EXIT_FAILURE);
				}
				break;
			default:
				std::cerr << "Bad parameter" << std::endl; 
		}
	}
	if (optind >= argc)
	{
		std::cerr << "Filename expected" << std::endl;
		exit(EXIT_FAILURE);
	}

	for(int i=optind;i<argc; ++i)
	{
		const char *fname = argv[i];
		print_pages(fname, pagePosList);
	}

	pdfedit_core_dev_destroy();
	return 0;
}
