// How to add boost
//- download boost - http://www.boost.org/ or http://www.boostpro.com/download
//- add include/libs to VS Tools->Options->Directories

#include <boost/shared_ptr.hpp>
#include <boost/program_options.hpp>
#include <boost/algorithm/string/replace.hpp>

int main(int , char* [])
{
	boost::shared_ptr<int> i;
	return 0;
}

