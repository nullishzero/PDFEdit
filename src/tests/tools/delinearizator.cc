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
