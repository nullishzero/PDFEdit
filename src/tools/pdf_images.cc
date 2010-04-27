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
#include <xpdf-aconf.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "xpdf/ImageOutputDev.h"

using namespace pdfobjects;
using namespace std;
using namespace boost;
namespace po = program_options;

namespace {

	// pages
	typedef vector<size_t> Pages;
	// library wrapper
	struct _pdf_lib {
		bool _ok;
		_pdf_lib (int argc, char ** argv) {_ok = (0 == pdfedit_core_dev_init(&argc, &argv));}
		~_pdf_lib () {pdfedit_core_dev_destroy();}
	};
	// what to do with a page
	struct _extract_images {
		void operator () (shared_ptr<CPage> page, ImageOutputDev& img_out, pdfobjects::DisplayParams& displayparams)
		{
		    page->displayPage (img_out, displayparams);
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
		("dir", po::value<string>()->default_value(string(".")), "directory")
		("what", po::value<string>(), "pages to convert")
		("hdpi", po::value<size_t>()->default_value(72), "horizontal dpi")
		("vdpi", po::value<size_t>()->default_value(72), "vertical dpi")
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
	
	Pages pages;
	if (vm.count("what"))
		pages = vm["what"].as<Pages>();

	string dir (vm["dir"].as<string>());
	size_t hdpi = vm["hdpi"].as<size_t>();
	size_t vdpi = vm["vdpi"].as<size_t>();

	try
	{
		// pdf lib init & work
		_pdf_lib _lib(argc, argv);
			if (!_lib._ok)
				return 1;

		// open pdf
		shared_ptr<CPdf> pdf = CPdf::getInstance (file.c_str(), CPdf::ReadWrite);
		ImageOutputDev img_out (const_cast<char*> (dir.c_str()), gTrue);

		// alter display params
		pdfobjects::DisplayParams displayparams;
		displayparams.hDpi = hdpi;
		displayparams.vDpi = vdpi;

		if (pages.empty())
		{
			for (size_t i = 1; i <= pdf->getPageCount(); ++i)
			{
				shared_ptr<CPage> page = pdf->getPage(i);
				std::cout << "\nPage " << i << ":";
				_extract_images()(page, img_out, displayparams);
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
			std::cout << "\nPage " << *it << ":";
			_extract_images()(page, img_out, displayparams);
		}

	}catch (std::exception& e)
	{
		std::cout << "exception - " << e.what();
	}

	Object::memCheck(stderr);
	gMemReport(stderr);
	return 0;
}
