#include "kernel/pdfedit-core-dev.h"
#include "kernel/flattener.h"
#include "kernel/pdfwriter.h"
#include "utils/debug.h"

using namespace pdfobjects;
#define suffix ".flatten"
int flatten_file(const char *fname)
{
using namespace utils;
	boost::shared_ptr<utils::Flattener> flattener = 
		Flattener::getInstance(fname, new OldStylePdfWriter()); 
	if(!flattener) {
		std::cerr << "Unable to open "<<fname<<" file"<<std::endl;
		return 1;
	}
	std::string outputFile(fname);
	outputFile+=suffix;
	std::cout << "Writing output to "<<outputFile<<std::endl;
	return flattener->flatten(outputFile.c_str());
}

int main(int argc, char** argv)
{
	if(pdfedit_core_dev_init())
	{
		std::cerr << "Unable to initialize pdfedit-dev" << std::endl;
		return 1;
	}
	//debug::changeDebugLevel(debug::utilsDebugTarget, debug::DBG_DBG);
	int ret = 0;
	for(int i=1; i<argc; ++i)
	{
		const char *fname= argv[i];
		try
		{
			ret = flatten_file(fname);
		}catch(...)
		{
			std::cerr << fname << " is not a valid pdf document - ignoring"<<std::endl;
			ret = 1;
		}
	}
	pdfedit_core_dev_destroy();
	return ret;
}
