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

void print_pages(const char *fname, RefContainer &refs)
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

	std::cerr << "Building page list: "; 
	typedef std::vector<boost::shared_ptr<CPage> > PageList;
	PageList pageList;
	for(int i=1; i<= pdf->getPageCount(); ++i)
		pageList.push_back(pdf->getPage(i));
	std::cerr << "done" << std::endl;

	RefContainer::const_iterator i;
	for (i=refs.begin(); i!=refs.end(); ++i)
	{
		IndiRef ref = *i;
		boost::shared_ptr<IProperty> prop = pdf->getIndirectProperty(ref);
		std::cout << "[" << ref << "]: ";
		
		for(PageList::iterator pageIter = pageList.begin(); 
				pageIter != pageList.end();
				++pageIter)
		{
			boost::shared_ptr<CDict> pageDict = (*pageIter)->getDictionary();
			if(pageDict->getIndiRef() == ref)
			{
				std::cout << "page #" << pdf->getPagePosition(*pageIter) << std::endl;
				return;
			}
		}
		std::cout << "not a page" << std::endl;
	}
}
int main(int argc, char ** argv)
{
	if(pdfedit_core_dev_init())
	{
		std::cerr << "Unable to initialize pdfedit-dev" << std::endl;
		return 1;
	}

	int opt;
	RefContainer refs;
	while ((opt = getopt(argc, argv, "r:")) != -1 )
	{
		switch (opt)
		{
			case 'r':
				if(add_ref(refs, optarg))
				{
					std::cerr << optarg 
						<< " is not a valid reference" 
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
		const char * fname = argv[i];
		print_pages(fname, refs);
	}
	pdfedit_core_dev_destroy();
	return 0;
}
