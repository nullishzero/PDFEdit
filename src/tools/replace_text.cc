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
#include <kernel/delinearizator.h>
#include <kernel/pdfwriter.h>
#include <kernel/ccontentstream.h>
#include <kernel/pdfoperatorsiter.h>
#include <boost/program_options.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <limits>

#ifdef WIN32
#include <Windows.h>
#undef max
#undef min
#endif


using namespace pdfobjects;
using namespace std;
using namespace boost;
using namespace utils;
namespace po = program_options;

namespace {

	// transformation matrix
	typedef vector<double> P;

	struct _pdf_lib {
		bool _ok;
		_pdf_lib (int argc, char ** argv) {_ok = (0 == pdfedit_core_dev_init(&argc, &argv));}
		~_pdf_lib () {pdfedit_core_dev_destroy();}
	};

	struct _replace {
		static const string name;
		void operator () (shared_ptr<CPage> page, const string& what, const string& with)
		{
			page->replaceText (what, with);
		}
	};
	const string _replace::name ("replace");
}

int 
main(int argc, char ** argv)
{
	// 
	// parameter parsing
	//
	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce help message")
		("file", po::value<string>(), "file")
		("from", po::value<size_t>()->default_value(1), "start page (default 0)")
		("to", po::value<size_t>(), "end page (default till the end of file)")
		("what", po::value<vector<string> >(), "what to replace")
		("with", po::value<vector<string> >(), "with what")
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

		if (!vm.count("file") || !vm.count("what")|| !vm.count("with")) 
		{
			cout << desc << endl;
			return 1;
		}
	string file = vm["file"].as<string>(); 
	size_t from = vm["from"].as<size_t>();
	vector<string> whats = vm["what"].as<vector<string> >();
	vector<string> withs = vm["with"].as<vector<string> >();
		if (whats.size() != withs.size())
		{
			cout << desc << endl;
			return 1;
		}
	size_t to = numeric_limits<size_t>::max();
	if (vm.count("to")) 
		to = vm["to"].as<size_t>();

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

		if (pdf->isLinearized())
		{
			pdf.reset ();
			string out (file+"-delinearised.pdf");
			{
				shared_ptr<Delinearizator> del (Delinearizator::getInstance(file.c_str(), new OldStylePdfWriter));
					if (!del) return -1;
				del->delinearize(out.c_str());
			}
			pdf = CPdf::getInstance (out.c_str(), CPdf::ReadWrite);
		}


		// sane values
		to = std::min(static_cast<unsigned int>(to), pdf->getPageCount()+1);

		// now the hard stuff comes - do this crazy loops intentionally
		for (size_t things_to_replace = 0; things_to_replace < withs.size(); ++things_to_replace)
		{
			#ifdef WIN32
			DWORD time = ::GetTickCount ();
			#endif

			string what = whats[things_to_replace];
			string with = withs[things_to_replace];
			for (size_t i = from; i < to; ++i)
			{
				shared_ptr<CPage> page = pdf->getPage(i);
				_replace()(page, what, with);
			}
			#ifdef WIN32
			cout << "time passed:" << ::GetTickCount()-time << endl;
			#endif
		}

		pdf->save ();
	
	}catch (std::exception& e)
	{
		std::cout << "exception - " << e.what();
	}

	return 0;
}
