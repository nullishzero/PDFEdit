#pragma once

#include <windows.h>

#include "utils.h"

namespace ui {


struct position { 
	position (int x_, int y_, int w_, int h_) : x(x_), y(y_), w(w_), h(h_) {}
	int x,y,w,h; 
};


/**
 * Class encapsulating win control as widget.
 */
class winwidget
{
protected:
	static const int HANDLED = 0;
	static const int NOTHANDLED = 1;
	static const int HANDLED_SPECIAL = 2;
	
	WNDPROC _def_proc;
	HWND _hwnd;

public:
	winwidget() : _hwnd(0), _def_proc(0) {}
	virtual ~winwidget () { _hwnd = NULL; }

public:
	inline static LRESULT CALLBACK 
	wnd_proc_callback (HWND hwnd, UINT msg, WPARAM w, LPARAM l)
	{
		winwidget* tthis = reinterpret_cast<winwidget*>(::GetWindowLong(hwnd, GWL_USERDATA));
		if (!tthis)
			return ::DefWindowProc (hwnd, msg, w, l);
		LRESULT ret = tthis->wnd_proc (hwnd, msg, w, l);
			if (NOTHANDLED == ret && tthis->default_proc())
				ret = ::DefWindowProc (hwnd, msg, w, l);
		return ret;
	}

	WNDPROC default_proc () const { return _def_proc; }
	operator HWND () const 
		{ return _hwnd; }
	inline void raise()
		{ ::SetWindowPos(*this, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE); }
	inline void lower()
		{ ::SetWindowPos(*this, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE); }
	inline void show ()
		{ ::ShowWindow(_hwnd, SW_SHOW);	}
	inline void hide ()
		{ ::ShowWindow(_hwnd, SW_HIDE);	}


protected:
	/** Call this function in constructor to receive messages in WndProc(). */
	bool init (HWND hwnd)
	{
			if (!hwnd)
				return false;
		_hwnd = hwnd;
		::SetWindowLong(_hwnd, GWL_USERDATA, reinterpret_cast<LONG>(this));
		_def_proc = reinterpret_cast<WNDPROC>(::SetWindowLong(hwnd, GWL_WNDPROC, reinterpret_cast<LONG>(&winwidget::wnd_proc_callback)));
		return true;
	}

	/** Override to catch window messages. Return HANDLED if message handled. */
	virtual LRESULT wnd_proc(HWND, UINT, WPARAM, LPARAM) = 0; 
};

#define TEST_WINDOW	L"WINDOW"


// status widget
template<typename Ftor>
class test_widget : public winwidget
{
	size_t _page;
	Ftor _display;

	// ctor & dtor
public:
	test_widget (HINSTANCE hInstance, 
				const position& p, 
				Ftor ftor,
				const std::wstring title = std::wstring (L"Pdf viewer (pdfedit)"))
		: _display (ftor), _page (1)
	{
		
		WNDCLASSEX wc;
		HWND hwnd;

		//Step 1: Registering the Window Class
		wc.cbSize        = sizeof(WNDCLASSEX);
		wc.style         = 0;
		wc.lpfnWndProc   = winwidget::wnd_proc_callback;
		wc.cbClsExtra    = 0;
		wc.cbWndExtra    = 0;
		wc.hInstance     = 0;
		wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
		wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
		wc.lpszMenuName  = NULL;
		wc.lpszClassName = TEST_WINDOW;
		wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

		if(!RegisterClassEx(&wc))
		{
			MessageBox(NULL, L"Window Registration Failed!", L"Error!",
				MB_ICONEXCLAMATION | MB_OK);
			return;
		}
		{
			hwnd = ::CreateWindowEx ( WS_EX_CLIENTEDGE, TEST_WINDOW, title.c_str(), 
				WS_OVERLAPPEDWINDOW, p.x, p.y, p.w, p.h, NULL, 0, hInstance, 0);
			init (hwnd);
		}
	}

	void page (size_t page)
		{ _page = page; }

protected:
	virtual LRESULT wnd_proc(HWND hwnd, UINT msg, WPARAM, LPARAM)
	{
		switch (msg)
		{
			case WM_PAINT:
				PAINTSTRUCT pps;
				::BeginPaint (hwnd, &pps);
				_display (_page, pps.hdc, pps.rcPaint);
				::EndPaint (hwnd, &pps);
				break;

			case WM_CANCELMODE:
			case WM_CLOSE:
			case WM_QUIT:
				::PostQuitMessage (0);
				break;
		}

		return NOTHANDLED;
	}

};

} // namespace ui