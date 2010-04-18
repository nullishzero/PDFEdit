// simple utility functions in utils namespace

#pragma once

#include <string>
#include <vector>
#include <windows.h>
#include <tchar.h>

// add mips specific ...
#include <Tlhelp32.h>
#if ARM
	#if !defined _WIN32_WCE || _WIN32_WCE < 0x600
	#pragma comment(lib, "..\\libs\\wince5\\armv4i\\Toolhelp.lib")
	#else
	#pragma comment(lib, "Toolhelp.lib")	
	#endif
#elif MIPS
	#if !defined _WIN32_WCE || _WIN32_WCE < 0x600
	#pragma comment(lib, "..\\libs\\wince5\\mipsii\\Toolhelp.lib")
	#else
	#pragma comment(lib, "Toolhelp.lib")	
	#endif
#elif defined (UNDER_CE)
	#pragma comment(lib, "Toolhelp.lib")	
#else
#endif

# define USE_TOOLHELP
#pragma message("If you get linking errors, either copy toolhelp.lib to libs under predefined directories or undefine USE_TOOLHELP because toolhelp is probably missing...")

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

#ifdef USE_TOOLHELP

	// traverse through processes and get their name/pid
	// return value is error
	template<typename T>
	inline bool for_all_processes (T& ftor)
	{
		//#define TH32CS_SNAPNOHEAPS 0x40000000
		HANDLE snap = ::CreateToolhelp32Snapshot (0x40000000|TH32CS_SNAPPROCESS, 0);
		if (INVALID_HANDLE_VALUE == snap)
			return false;

		PROCESSENTRY32 pe = {0};
		pe.dwSize = sizeof (PROCESSENTRY32);
		::Process32First (snap, &pe);
		do {
				std::wstring name (pe.szExeFile);
				if (!ftor (utils::convert<std::string>(name), pe.th32ProcessID, pe))
					break;
		}while (::Process32Next(snap, &pe));

	#if defined(UNDER_CE)
		if (INVALID_HANDLE_VALUE != snap)
			::CloseToolhelp32Snapshot(snap);
	#else
		::CloseHandle (snap);
	#endif
		return true;
	}

	struct info
	{
		std::string process;
		DWORD pid;
		DWORD start_address;
	};

	inline info process_info (const std::string& process)
	{
		// create info with used ftors
		struct strange_info
		{
			std::string process;
			DWORD pid;
			DWORD start_address;
			
			// ctor
			strange_info (const std::string& process_) : pid(-1), start_address(0), process(process_) {}
			
			// for hwnds
			virtual bool operator() (HWND hwnd) 
			{ 
				_TCHAR buf [MAX_PATH] = {0};
				int len = ::GetWindowText(hwnd, buf, MAX_PATH);
					if (!len)
						return true;
				std::string name = utils::convert<std::string>(&buf[0], &buf[len]);
				if (name.find (process) != std::string::npos)
				{
					DWORD pid_;
					::GetWindowThreadProcessId (hwnd, &pid_); 
					pid = pid_;
					return false; 
				} 
				return true; 
			}
			// for processes
			virtual bool operator () (const std::string& name_, DWORD pid_, const PROCESSENTRY32& pe_)
			{
				if (pid == pid_ || std::string::npos != name_.find (process))
				{
					// just to be sure
					pid = pid_;
					process = name_;
			#ifdef UNDER_CE
					start_address = pe_.th32MemoryBase;
			#endif
					return false;
				}
				return true;
			}

		};
		// create the smart class
		strange_info oddinfo (process);

		//1. DO IT EITHER BY ENUMARATING HWNDs
		//2. or by processes
		// not all work on all devices ...

		// 1.
		win_utils::for_all_hwnd (oddinfo);

		// 2. 
		if (!win_utils::for_all_processes (oddinfo))
		{
			std::cerr << "Problems with toolhelp [" << ::GetLastError () << "]!";
		}

		// create return info
		info ret;
		ret.process = oddinfo.process;
		ret.pid = oddinfo.pid;
		ret.start_address = oddinfo.start_address;
		return ret;
	}

#endif

	void minimise (const std::string& process)
	{
		struct myinfo : public info {
			myinfo (info& i)
			{
				 pid = i.pid;
				 process = i.process;
				 start_address = i.start_address;
			}
			bool operator () (HWND hwnd)
			{ 
				DWORD _pid;
				::GetWindowThreadProcessId (hwnd, &_pid);
				if (pid == _pid)
				{
					::ShowWindow(hwnd, SW_HIDE);
					::ShowWindow(hwnd, SW_MINIMIZE);
					::PostMessage (hwnd, WM_MOVE, 0, MAKELPARAM(10,10));
					std::cout << "Sent.\n";

				} 
				return true; 
			}
		};

		myinfo info (process_info (process));
		for_all_hwnd (info);
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
