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
#include <splash/Splash.h>
#include <splash/SplashBitmap.h>	
#include <xpdf/SplashOutputDev.h>

#include <boost/program_options.hpp>
#include <vector>

#ifdef WIN32
#include <windows.h>
#endif

using namespace pdfobjects;
using namespace std;
using namespace boost;
namespace po = program_options;

#ifndef WIN32
#warning Sorry, only for WIN32!

int main()
{
	std::cerr << "This tool is not implemented for non Windows environment :(" << std::endl;
	return 1;
}

#else

namespace {


	// to bmp
	static void save_bmp(const std::string& file, void* pbuf, BITMAPINFO& bmpInfo)
	{	// uff, taken from web
		FILE*      fp = NULL;
		BITMAPFILEHEADER  bmpFileHeader = {0};
		do{
			if((fp = fopen(file.c_str(),"wb"))==NULL)
				return;
			bmpFileHeader.bfReserved1=0;
			bmpFileHeader.bfReserved2=0;
			bmpFileHeader.bfSize=sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+bmpInfo.bmiHeader.biSizeImage;
			bmpFileHeader.bfType='MB';

			bmpFileHeader.bfOffBits=sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER);
			fwrite(&bmpFileHeader,sizeof(BITMAPFILEHEADER),1,fp);
			fwrite(&bmpInfo.bmiHeader,sizeof(BITMAPINFOHEADER),1,fp);
			fwrite(pbuf,bmpInfo.bmiHeader.biSizeImage,1,fp);
		}while(false);

		if(fp)      
			fclose(fp);
	}

	struct _time {
		DWORD _tick;
		_time () : _tick (::GetTickCount()) {}
		std::string passed () const 
		{
			std::ostringstream oss;
			oss << (::GetTickCount() - _tick);
			return oss.str();
		}
	};


	// pages
	typedef vector<size_t> Pages;
	// library wrapper
	struct _pdf_lib {
		bool _ok;
		_pdf_lib (int argc, char ** argv) {_ok = (0 == pdfedit_core_dev_init(&argc, &argv));}
		~_pdf_lib () {pdfedit_core_dev_destroy();}
	};
	// what to do with a page
	struct _bmpify {
		string operator () (shared_ptr<CPdf> pdf, 
							shared_ptr<CPage> page, 
							const std::string& file, 
							size_t hdpi, size_t vdpi)
		{
			_time time;
			SplashColor paperColor;
			paperColor[0] = paperColor[1] = paperColor[2] = 0xff;
			SplashOutputDev splash  (splashModeBGR8, 4, gFalse, paperColor);
			splash.startDoc(pdf->getCXref());

			// alter display params
			pdfobjects::DisplayParams displayparams;
			displayparams.hDpi = hdpi;
			displayparams.vDpi = vdpi;

			// display it = create internal splash bitmap
			page->displayPage (splash, displayparams);
			splash.clearModRegion();

			//
			// WIN GDI stuff
			BITMAPINFO source_bmi = {0};
			source_bmi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
			size_t w = splash.getBitmap()->getWidth();
			size_t h = splash.getBitmap()->getHeight();
			source_bmi.bmiHeader.biWidth=w;
			source_bmi.bmiHeader.biHeight=h;
			source_bmi.bmiHeader.biPlanes=1;
			source_bmi.bmiHeader.biBitCount=24;
			source_bmi.bmiHeader.biCompression=BI_RGB;
			source_bmi.bmiColors[0].rgbBlue = 0;
			source_bmi.bmiColors[0].rgbGreen = 0;
			source_bmi.bmiColors[0].rgbRed = 0;
			source_bmi.bmiColors[0].rgbReserved = 0;
			source_bmi.bmiHeader.biSizeImage = (w*h*source_bmi.bmiHeader.biBitCount)/8;

			BITMAPINFO capture_bmi = source_bmi;
			capture_bmi.bmiHeader.biBitCount=32;
			capture_bmi.bmiHeader.biSizeImage = (w*h*capture_bmi.bmiHeader.biBitCount)/8;

			HDC screen_dc = ::GetDC(0);
			HDC capture_dc = ::CreateCompatibleDC(screen_dc);

			void* buf = NULL;
			HBITMAP capture_bmp = ::CreateDIBSection (screen_dc, &capture_bmi, BI_RGB, &buf, NULL, 0);
			::SelectObject(capture_dc, capture_bmp);
			
			if (0 == ::StretchDIBits(capture_dc,
							0,0,capture_bmi.bmiHeader.biWidth,capture_bmi.bmiHeader.biHeight,
							0,
							source_bmi.bmiHeader.biHeight,source_bmi.bmiHeader.biWidth,-source_bmi.bmiHeader.biHeight,
							(void *)splash.getBitmap()->getDataPtr(),
							&source_bmi,
							DIB_RGB_COLORS,SRCCOPY))
			{
				std::cout << "GDI problem" << std::endl;
			}

			std::string result = std::string (" [paint:") + time.passed() + std::string ("]");

			save_bmp(file, buf, capture_bmi);

			::DeleteDC(screen_dc);
			::DeleteDC(capture_dc);
			::DeleteObject(capture_bmp);

			return result;
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
		("what", po::value<Pages>(), "page to convert")
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

	size_t hdpi = vm["hdpi"].as<size_t>();
	size_t vdpi = vm["vdpi"].as<size_t>();

	try
	{
		// pdf lib init & work
		_pdf_lib _lib(argc, argv);
			if (!_lib._ok)
				return 1;

		GlobalParams::initGlobalParams(NULL)->setEnableT1lib("no");
		GlobalParams::initGlobalParams(NULL)->setEnableFreeType("yes");
		GlobalParams::initGlobalParams(NULL)->setErrQuiet(gTrue);
		GlobalParams::initGlobalParams(NULL)->setAntialias("yes");
		GlobalParams::initGlobalParams(NULL)->setupBaseFonts(".");

		// open pdf
		shared_ptr<CPdf> pdf = CPdf::getInstance (file.c_str(), CPdf::ReadWrite);


		if (pages.empty())
		{
			for (size_t i = 1; i <= pdf->getPageCount(); ++i)
			{
				ostringstream oss;
				oss << i << ".bmp";
				_time time;
				shared_ptr<CPage> page = pdf->getPage(i);
				std::cout << "\nPage " << i << _bmpify()(pdf, page, oss.str(), hdpi, vdpi);
				std::cout << " [all:" << time.passed() << "]";
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

			ostringstream oss;
			oss << *it << ".bmp";
			_time time;
			shared_ptr<CPage> page = pdf->getPage(*it);
			std::cout << "\nPage " << *it << _bmpify()(pdf, page, oss.str(), hdpi, vdpi);
			std::cout << " [all:" << time.passed() << "]";
		}

	}catch (std::exception& e)
	{
		std::cout << "exception - " << e.what();
	}

	return 0;
}
#endif
