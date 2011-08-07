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

using namespace pdfobjects;
using namespace std;
using namespace boost;
namespace po = program_options;

namespace {

	// default values
	const string DEFAULT_ENCODING( "UTF-8" );
	const bool DEFAULT_OUTPUT_PAGES = false;
	const string DEFAULT_FONT_DIR( "." );

	// pages
	typedef vector<size_t> Pages;
	// library wrapper
	struct _pdf_lib {
		bool _ok;
		_pdf_lib (int argc, char ** argv, const string& font_dir) {
			struct pdfedit_core_dev_init init = {0};
			init.fontDir = font_dir.c_str();
			_ok = (0 == pdfedit_core_dev_init(&argc, &argv, &init));
		}
		~_pdf_lib () {pdfedit_core_dev_destroy();}
	};
	// what to do with a page
	struct _textify {
		string operator () (shared_ptr<CPage> page, const string& encoding)
		{
			// Update display params to use media box not default page rect (DEFAULT_PAGE_RX, DEFAULT_PAGE_RY)
			// TODO upsidedown? get/set
			DisplayParams dp;
			dp.useMediaBox = gTrue;
			dp.crop = gFalse;
			dp.rotate = page->getRotation ();
			page->setDisplayParams (dp);

			string text;
			page->getText( text, &encoding );
			return text;
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
		("file", po::value<string>(), "input file")
		("what", po::value<Pages>(), "pages to convert")
		("output-pages", po::value<bool>()->default_value(DEFAULT_OUTPUT_PAGES), "output page number before each page")
		("encoding", po::value<string>()->default_value(DEFAULT_ENCODING), "encoding to use")
		("font-dir", po::value<string>()->default_value(DEFAULT_FONT_DIR), "(xpdf) font directory with font definitions(e.g. N019003L.PFB)")
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

		if (!vm.count("file")) 
		{
			cout << desc << endl;
			return 1;
		}
	string file = vm["file"].as<string>(); 
	bool output_pages = vm["output-pages"].as<bool>(); 
	string encoding = vm["encoding"].as<string>(); 
	string font_dir = vm["font-dir"].as<string>(); 
	
	Pages pages;
	if (vm.count("what"))
		pages = vm["what"].as<Pages>();

	try
	{
		// pdf lib init & work
		_pdf_lib _lib(argc, argv, font_dir);
			if (!_lib._ok)
				return 1;

		// open pdf
		shared_ptr<CPdf> pdf = CPdf::getInstance (file.c_str(), CPdf::ReadWrite);

		if (pages.empty())
		{
			for (size_t i = 1; i <= pdf->getPageCount(); ++i)
			{
				shared_ptr<CPage> page = pdf->getPage(i);
				if (output_pages)
					std::cout << "\nPage " << i << ":\n";
				std::cout << _textify()(page, encoding);
			}
		}
		
		// do it for selected pages
		for (Pages::const_iterator it = pages.begin(); it != pages.end(); ++it)
		{
				if (*it > pdf->getPageCount())
				{
					cout << "Invalid page number! " << endl << desc << endl;
					continue;
				}

			shared_ptr<CPage> page = pdf->getPage(*it);
			if (output_pages)
				std::cout << "\nPage " << *it << ":\n";
			std::cout << _textify()(page, encoding);
		}

	}catch (std::exception& e)
	{
		std::cout << "exception - " << e.what();
		return -1;
	}

	return 0;
}
