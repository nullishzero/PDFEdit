// simple utility functions in utils namespace

#pragma once

#include <string>
#include <windows.h>

namespace utils {

	// e.g. string<->wstring
	template<typename T, typename U>
	inline T convert (U what)
	{
		return T (what.begin(), what.end());
	}

	// e.g. iterator first,last<->wstring
	template<typename T, typename It>
	inline T convert (It first, It last)
	{
		return T (first, last);
	}
	
	inline std::wstring convert (const std::string& str) 
		{ return convert<std::wstring> (str); }

	inline std::string convert (const std::wstring& str)
		{ return convert<std::string> (str); }	

} // namespace utils
