// simple logger in utils namespace, outputting to file

#pragma once

#include <iostream>
#include <fstream>
#include "win_utils.h"
#include "utils.h"

namespace utils {

	// use cwd and create log.txt inside
	// opens the file stream at the beginning, close at destroy
	class logger
	{
			std::ofstream _off;

		public:
			logger (const std::string& log_file)
			{ 
				std::string path_to_logfile (win_utils::cwd<std::string>()+log_file);
				_off.open (path_to_logfile.c_str()); 
			}
			~logger()
			{
				_off.close();
			}

			// synctactic sugar
			logger& operator<<(std::ostream& (*_fn)(std::ostream&))
			{
				(*_fn)(_off);
				return *this;
			}
			template<typename T> friend logger& operator<<(logger&, const T&);
	};

	template<typename T>
	inline logger& operator << (logger& off, const T& txt)
	{
		off._off << txt << std::flush;
		return off;
	}

} // namespace utils

