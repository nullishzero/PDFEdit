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

#define PNG_DEBUG 3
#include <png.h>


using namespace pdfobjects;
using namespace std;
using namespace boost;
using namespace utils;
namespace po = program_options;

namespace {

	// transformation matrix
	typedef vector<double> Position;
	typedef vector<size_t> Pages;
	typedef vector<char> buffer;

	struct _pdf_lib {
		bool _ok;
		_pdf_lib (int argc, char ** argv) {_ok = (0 == pdfedit_core_dev_init(&argc, &argv));}
		~_pdf_lib () {pdfedit_core_dev_destroy();}
	};

	struct png {
		libs::Point size;
		png_byte color_type;
		png_byte bit_depth;
		buffer buf;
		bool ok;

		png (const std::string& file) : ok (false)
		{
			png_structp png_ptr;
			png_infop info_ptr;
			int number_of_passes;
			unsigned char header[8];	// 8 is the maximum size that can be checked

			// quick encaps. fix: original c code taken from  http://zarb.org/~gc/html/libpng.html
			struct _1 {
				FILE* fp;
				_1 (const std::string& file) : fp(NULL) { fp = fopen(file.c_str(), "rb"); }
				~_1 () { fclose (fp); }
				operator FILE* () { return fp; }
			};

			/* open file and test for it being a png */
			_1 fp (file);
				if (!fp)
					return;
				fread(header, 1, 8, fp);
				if (png_sig_cmp(header, 0, 8))
					return;

			/* initialize stuff */
			png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
				if (!png_ptr)
					return;

			info_ptr = png_create_info_struct(png_ptr);
				if (!info_ptr)
					return;

			if (setjmp(png_jmpbuf(png_ptr)))
				return;

			png_init_io(png_ptr, fp);
			png_set_sig_bytes(png_ptr, 8);

			png_read_info(png_ptr, info_ptr);

			size.x = info_ptr->width;
			size.y = info_ptr->height;
			color_type = info_ptr->color_type;
			bit_depth = info_ptr->bit_depth;

			number_of_passes = png_set_interlace_handling(png_ptr);
			png_read_update_info(png_ptr, info_ptr);

			/* read file */
			if (setjmp(png_jmpbuf(png_ptr)))
				return;

			scoped_array<png_byte>  raw_buf (new png_byte [info_ptr->rowbytes]);
			for (size_t y = 0; y < info_ptr->height; y++)
			{
				png_bytep bufp = raw_buf.get();
				png_bytepp bufpp = &bufp;
				png_read_rows (png_ptr, bufpp, NULL, 1);
				std::copy (&raw_buf[0], &raw_buf[info_ptr->rowbytes], std::back_inserter (buf));
			}

	        fclose(fp);
			ok = true;
		}

	};

	struct _add {
		void operator () (shared_ptr<CPage> page, 
						  const Position& where, 
						  Point image_dim, 
						  size_t bit_depth, 
						  buffer what)
		{
				if (where.size() != 2)
					throw std::exception ();
			libs::Point point (where[0], where[1]);

			page->addInlineImage (what, image_dim, point);
		}
	};
}

int 
main(int argc, char ** argv)
{
	// 
	// parameter parsing
	//
	po::options_description desc("Allowed options\nExample options: --file=test.pdf --where=1 --png=test.png --p=100 --p=100");
	desc.add_options()
		("help", "produce help message")
		("file", po::value<string>(), "file")
		("png", po::value<string>(), "")
		("where", po::value<Pages>(), "which page(s) to add")
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

		if (!vm.count("file") || !vm.count("where") || !vm.count("p") || !vm.count("png")) 
		{
			cout << desc << endl;
			return 1;
		}
	string file = vm["file"].as<string>(); 
	string image = vm["png"].as<string>();
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

		// read image
		png _png (image);
		if (!_png.ok)
		{
			cout << "Problems with parsing png file" << endl;
			return -1;
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
			_add()(page, pos, _png.size, _png.bit_depth, _png.buf);

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
