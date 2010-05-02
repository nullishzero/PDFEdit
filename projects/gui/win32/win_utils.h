// simple utility functions in utils namespace

#pragma once

#include <string>
#include <vector>
#include <windows.h>
#include <tchar.h>

#include "utils.h"

namespace win_utils {

	// returns current working directory in type you want (string,wstring) + \\
	// win specific
	template<typename T>
	inline T cwd () 
	{
		TCHAR path [MAX_PATH+1];
		DWORD len = ::GetModuleFileName(::GetModuleHandle(NULL), path, MAX_PATH);
		if(len)
		{
			while(len && path[len] != L'\\')
				len--;
			if(len)
				path[len+1] = L'\000';
		}
		return len ? utils::convert<T>(&path[0],&path[len+1]) : T();
	}
	inline std::string cwd ()
		{ return cwd<std::string> (); }

	template<typename T>
	inline T relative_to_cwd (T name)
	{
		return cwd<T>() + T("\\") + name;
	}

	inline std::string 
	relative_to_cwd (const char* name)
	{
		return cwd() + std::string("\\") + name;
	}

	// execute ftor for all hwnds in the system
	template<typename T>
	inline void for_all_hwnd (T& ftor)
	{
		struct _1 {
			static BOOL CALLBACK clb (HWND hwnd, LPARAM lParam) 
			{ 
				T* f = (T*)lParam;
				return (*f)(hwnd);
			}
		};
		::EnumWindows (_1::clb, (LPARAM)&ftor);
	}

	// return list of HWNDS
	template<typename T>
	inline std::vector<HWND> filter_all_hwnd (T& filter)
	{
		std::vector<HWND> filtered;
		struct _1 {
			std::vector<HWND> filtered;
			const T& filter;
			_1 (T& filter_) : filter (filter_) {}
			void operator() (HWND hwnd) 
			{ 
				if (_filter(hwnd))
					filtered.push_back (hwnd);
				return true;
			}
		};
		_1 anonym;
		for_all_hwnd (anonym);
		return anonym.filtered;
	}


	// remove file
	template<typename T>
	inline void remove (const T& file) 
	{
		::DeleteFile (utils::convert<std::wstring>(file).c_str());
	}

	// list all files in a directory
	template<typename T>
	inline std::vector<T> listdir(const T& dir)
	{
		std::vector<T> files;
		
		WIN32_FIND_DATA find_data;
		HANDLE find_handle;
		std::wstring dirlist = utils::convert<std::wstring>(dir) + std::wstring(L"\\*");
		find_handle = ::FindFirstFile(dirlist.c_str(), &find_data);
		if (INVALID_HANDLE_VALUE!= find_handle)
		{
			do  {
				wchar_t fontfile[MAX_PATH+20];
				int len = ::wsprintf(fontfile, L"%s%s", dir.c_str(), find_data.cFileName);
				files.push_back(utils::convert<T>(&fontfile[0], &fontfile[len]));
			}while (::FindNextFile(find_handle, &find_data));

			if (INVALID_HANDLE_VALUE != find_handle)
				::FindClose(find_handle);
		}
		return files;
	}

} // namespace utils
