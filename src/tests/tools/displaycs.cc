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
#include <kernel/static.h>
#include <sstream>
#include <kernel/pdfedit-core-dev.h>
#include <kernel/cpdf.h>
#include <kernel/cpage.h>
#include <kernel/ccontentstream.h>
#include <boost/program_options.hpp>


using namespace pdfobjects;
using namespace std;
using namespace boost;
namespace po = program_options;

namespace {
	template<typename T, typename U>
	T _to(const U& val)
	{
		T ret = T();
		istringstream iss (val);
		iss >> ret;
		return ret;
	}
}

int main(int argc, char ** argv)
{
	if (pdfedit_core_dev_init(&argc, &argv))
		return 1;

	// Declare the supported options.
	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce help message")
		("file", po::value<string>(), "set compression level")
		("from", po::value<size_t>()->default_value(1), "start page (default 0)")
		("to", po::value<size_t>(), "end page (default till the end)")
	;

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);    

		if (vm.count("help") || !vm.count("file"))
		{
			cout << desc << "\n";
			return 1;
		}

	string file = vm["file"].as<string>(); 
	size_t from = vm["from"].as<size_t>();

	// open pdf
	shared_ptr<CPdf> pdf = CPdf::getInstance (file.c_str(), CPdf::ReadWrite);

	// sane values
	size_t to = pdf->getPageCount()+1;
	if (vm.count("to")) 
		to = std::min(to, vm["to"].as<size_t>());

	//
	cout << "File: " << file << " #pages: " << pdf->getPageCount ()
			<< " from: " << from << " to: " << to << endl;
	
	// speciality
	for (size_t i = from; i < to; ++i)
	{
		std::cout << "=====================" << endl;
		std::cout << "PAGE NUMBER " << i << endl;
		shared_ptr<CPage> page = pdf->getPage(i);
		typedef vector<shared_ptr<CContentStream> > CCs;
		CCs ccs;
		page->getContentStreams (ccs);
		std::cout << "- parsed" << endl;
		for (CCs::const_iterator it = ccs.begin(); it != ccs.end(); ++it)
		{
			string str;
			(*it)->getStringRepresentation (str);
			std::cout << str << flush;
		}
		std::cout << "- got the repre" << endl;
	}
	
	gMemReport(stdout);
    pdfedit_core_dev_destroy();
	return 0;
}
