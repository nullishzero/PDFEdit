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
#include <kernel/pdfedit-core-dev.h>
#include <kernel/cpdf.h>
#include <kernel/cpage.h>
#include <kernel/delinearizator.h>
#include <boost/program_options.hpp>
#include <vector>

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
	typedef vector<double> Position;
	typedef vector<size_t> Pages;

	struct _pdf_lib {
		bool _ok;
		_pdf_lib (int argc, char ** argv) {_ok = (0 == pdfedit_core_dev_init(&argc, &argv));}
		~_pdf_lib () {pdfedit_core_dev_destroy();}
	};

	struct _add {
		void operator () (shared_ptr<CPage> page, 
						  const string& what, 
						  const Position& where, 
						  std::string font_id)
		{
				if (where.size() != 2)
					throw std::exception ();
			libs::Point point (where[0], where[1]);
			static CPageFonts::SystemFontList fonts (CPageFonts::getSystemFonts ());
			if (fonts.end() != std::find (fonts.begin(), fonts.end(), font_id))
			{
				font_id = page->addSystemType1Font (font_id);
			}
			page->addText (what, point, font_id);
		}
	};
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
		("what", po::value<string>(), "string to add")
		("where", po::value<Pages>(), "which page(s) to add")
		("font", po::value<string>()->default_value(string("Helvetica")), "which system font to use (default Helvetica")
		("showfonts","show system fonts")
		("p", po::value<Position>(), "position(e.g. --p 1 --p 1)")
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

		if (vm.count("showfonts"))
		{
			CPageFonts::SystemFontList fonts (CPageFonts::getSystemFonts());
			std::copy(fonts.begin(), fonts.end(), 
						std::ostream_iterator<string>(std::cout, "\n"));
			return 0;
		}
		if (!vm.count("file") || !vm.count("what") || !vm.count("p")) 
		{
			cout << desc << endl;
			return 1;
		}
	string file = vm["file"].as<string>(); 
	string what = vm["what"].as<string>();
	string font_id = vm["font"].as<string>();
	Pages pages = vm["where"].as<Pages>();
	Position pos = vm["p"].as<Position>();
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


		for (Pages::const_iterator it = pages.begin(); it != pages.end(); ++it)
		{
			// sane values
			#ifdef WIN32
			DWORD time = ::GetTickCount ();
			#endif

				if (*it > pdf->getPageCount())
				{
					cout << "Invalid page number! " << endl << desc << endl;
					continue;
				}

			shared_ptr<CPage> page = pdf->getPage(*it);
			_add()(page, what, pos, font_id);

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
