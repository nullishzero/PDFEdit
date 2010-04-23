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
#include <sstream>
#include <kernel/pdfedit-core-dev.h>
#include <kernel/cpdf.h>
#include <kernel/cpage.h>
#include <kernel/ccontentstream.h>
#include <boost/program_options.hpp>
#include <limits>


using namespace pdfobjects;
using namespace std;
using namespace boost;
namespace po = program_options;

namespace {

	// transformation matrix
	typedef vector<double> P;

	struct _pdf_lib {
		bool _ok;
		_pdf_lib (int argc, char ** argv) {_ok = (0 == pdfedit_core_dev_init(&argc, &argv));}
		~_pdf_lib () {pdfedit_core_dev_destroy();}
	};

	struct stm {
		static const string name;
		void operator () (shared_ptr<CPage> page, P& p)
		{
				if (p.size() < 6) throw std::runtime_error ("too few p params");
			page->setTransformMatrix (&p[0]);
		}
	};
	const string stm::name ("stm");
	
	struct sr {
		static const string name;
		void operator () (shared_ptr<CPage> page, P& p)
		{
				if (p.size() < 1) throw std::runtime_error ("too few p params");
			page->setRotation ((int)(p[0]));
		}
	};
	const string sr::name ("sr");

	struct smb {
		static const string name;
		void operator () (shared_ptr<CPage> page, const P& p)
		{
				if (p.size() < 4) throw std::runtime_error ("too few p params");
			page->setMediabox (libs::Rectangle (p[0], p[1], p[2], p[3]));
		}
	};
	const string smb::name ("smb");
}

int 
main(int argc, char ** argv)
{
	// 
	// parameter parsing
	//
	po::options_description desc("Example:\npagemetrics-tool.exe --file=test.pdf --alg=sr --p=90\n\nAllowed options");
	desc.add_options()
		("help", "produce help message")
		("file", po::value<string>(), "file")
		("alg", po::value<string>()->default_value(stm::name), 
										 "set algorithm ["
										 "stm - transformation matrix (default), "
										 "sr - rotation, "
										 "smb - media box"
										 "]")
		("p", po::value<P>(), "parameters (e.g. for transformation matrix :) --p 1 --p 1 --p 1 --p 1 --p 1 --p 1)")
		("from", po::value<size_t>()->default_value(1), "start page (default 0)")
		("to", po::value<size_t>(), "end page (default till the end of file)")
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

		if (!vm.count("file") || !vm.count("p")) 
		{
			cout << desc << endl;
			return 1;
		}
	string file = vm["file"].as<string>(); 
	unsigned int from = vm["from"].as<unsigned int>();
	string alg = vm["alg"].as<string>();
	P p = vm["p"].as<P>();
	unsigned int to = numeric_limits<unsigned int>::max();
	if (vm.count("to")) 
		to = vm["to"].as<unsigned int>();

	// 
	// pdf lib init & work
	//
	try
	{
		_pdf_lib _lib(argc, argv);
			if (!_lib._ok)
				return 1;

		// open pdf
		shared_ptr<CPdf> pdf = CPdf::getInstance (file.c_str(), CPdf::ReadWrite);

		// sane values
		to = std::min(to, pdf->getPageCount()+1);

		// now the hard stuff comes
		for (unsigned int i = from; i < to; ++i)
		{
			shared_ptr<CPage> page = pdf->getPage(i);
			if (alg == stm::name)
				stm()(page,p);
			else if (alg == sr::name)
				sr()(page,p);
			else if (alg == smb::name)
				smb()(page,p);
		}

		pdf->save ();
	
	}catch (std::exception& e)
	{
		std::cout << "exception - " << e.what();
	}

	return 0;
}
