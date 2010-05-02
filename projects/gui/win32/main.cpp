#include <windows.h>

#include "utils.h"
#include "logger.h"
#include "params.h"
#include "widgets.h"
#include "memory.h"

// xpdf, pdfedit
#define NO_CMAP
#include "xpdf/GlobalParams.h"
#undef NO_CMAP
#include "kernel/pdfedit-core-dev.h"
#include "kernel/cpdf.h"	
#include "kernel/cpage.h"	
#include "splash/Splash.h"	
#include "splash/SplashBitmap.h"	
#include "xpdf/SplashOutputDev.h"	


namespace {
	static const int YES = 1;
	static const int NO = 0;

	// pages
	typedef std::vector<size_t> Pages;
	// library wrapper
	struct _pdf_lib {
		bool _ok;
		_pdf_lib (int argc, char ** argv) {
			std::string cwd (".");
			struct pdfedit_core_dev_init init = {0};
			init.fontDir = cwd.c_str(); // ehm, but valid
			_ok = (0 == pdfedit_core_dev_init(&argc, &argv, &init));
		}
		~_pdf_lib () {pdfedit_core_dev_destroy();}
	};


	struct _display
	{
		boost::shared_ptr<pdfobjects::CPdf> pdf;

		_display (boost::shared_ptr<pdfobjects::CPdf> p) : pdf(p){}
	
		void operator() (size_t page_num, HDC hdc, RECT)
		{
			boost::shared_ptr<pdfobjects::CPage> page (pdf->getPage(page_num));

			// init splash bitmap
			SplashColor paperColor;
			paperColor[0] = paperColor[1] = paperColor[2] = 0xff;
			SplashOutputDev splash  (splashModeBGR8, 4, gFalse, paperColor);
			splash.startDoc(pdf->getCXref());

			// alter display params
			pdfobjects::DisplayParams displayparams;
			displayparams.hDpi = utils::params::instance().value("hdpi",25);
			displayparams.vDpi = utils::params::instance().value("vdpi",25);

			// display it = create internal splash bitmap
			page->displayPage (splash, displayparams);
			splash.clearModRegion();

			//
			// WIN GDI stuff
			BITMAPINFO bmi;
			::ZeroMemory(&bmi,sizeof(bmi));
			bmi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
			bmi.bmiHeader.biWidth=splash.getBitmap()->getWidth();//+(m_splashOut->getBitmap()->getWidth()%2);
			bmi.bmiHeader.biHeight=splash.getBitmap()->getHeight();
			bmi.bmiHeader.biPlanes=1;
			bmi.bmiHeader.biBitCount=24;
			bmi.bmiHeader.biCompression=BI_RGB;

			bmi.bmiColors[0].rgbBlue = 0;
			bmi.bmiColors[0].rgbGreen = 0;
			bmi.bmiColors[0].rgbRed = 0;
			bmi.bmiColors[0].rgbReserved = 0;

			::StretchDIBits(hdc,
							0,0,bmi.bmiHeader.biWidth,bmi.bmiHeader.biHeight,
							0,
							bmi.bmiHeader.biHeight,bmi.bmiHeader.biWidth,-bmi.bmiHeader.biHeight,
							(void *)splash.getBitmap()->getDataPtr(),
							&bmi,
							DIB_RGB_COLORS,SRCCOPY);
			
			//********END DIB
		}
	};



}


int APIENTRY wWinMain(HINSTANCE hInstance,
                       HINSTANCE hPrevInstance,
                       LPTSTR    lpCmdLine,
                       int       nCmdShow)
{
	// load config
	utils::params::instance().load (win_utils::cwd<std::string>()+std::string("config")); 
	

	// get logger
	static const char* LOG_FILE = "log.txt";
	utils::logger logger (LOG_FILE);
	logger << "Starting" << std::endl;


	try
	{
		// pdf lib init & work
		_pdf_lib _lib(0, NULL);
			if (!_lib._ok)
				return 1;


		GlobalParams::initGlobalParams(NULL)->setEnableT1lib("no");
		GlobalParams::initGlobalParams(NULL)->setEnableFreeType("yes");
		GlobalParams::initGlobalParams(NULL)->setErrQuiet(gFalse);
		GlobalParams::initGlobalParams(NULL)->setAntialias("yes");
		GlobalParams::initGlobalParams(NULL)->setupBaseFonts(win_utils::cwd<std::string>().c_str());

		// file
		std::wstring file (utils::convert(utils::params::instance().value("file",std::string("g:\\test.pdf"))));

		// create ui
		ui::test_widget<_display> ui (
				hInstance,
				ui::position(
					utils::params::instance().value("x",0),
					utils::params::instance().value("y",0),
					utils::params::instance().value("width",900),
					utils::params::instance().value("height",530)),
					_display (pdfobjects::CPdf::getInstance (utils::convert(file).c_str(), pdfobjects::CPdf::ReadOnly)),
				file
			);
		ui.show();

		BOOL ret = 0;
		MSG msg = {0};

		while ((ret=::GetMessage(&msg,NULL,0,0)) != 0)
		{ 
				if (-1 == ret)
					break;
			::TranslateMessage(&msg); 
			switch (msg.message)
			{
				case WM_CHAR:
				{
					ui.page (static_cast<char>(msg.wParam)-'0');
					::InvalidateRect (ui, NULL, true);
					::UpdateWindow (ui);
					break;
				}
				case WM_CANCELMODE:
				case WM_QUIT:
					::PostQuitMessage (0);
					break;
			}
			::DispatchMessage(&msg); 
		}

		logger << "Ending..." << std::endl;

	}catch (std::exception& e)
	{
		logger << "Exception: " << e.what() << std::endl;
	}

	return 0;
}