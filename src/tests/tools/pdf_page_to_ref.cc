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
