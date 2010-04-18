// simple param reader in utils namespace
//
// Example file .\config:
// test_info=1
// test_device_capabilities=1
//
// Example usage:
// utils::params::instance().load (win_utils::cwd()+std::string("config")); 
// if (1 == utils::params::instance().value("test_info",1))
//	do_something
//

#pragma once

#include <map>
#include <fstream>
#include <string>
#include <sstream>

namespace utils {

	// singleton params read from a file
	class params
	{
		private:
			params () {}
			params (const params&);
			params& operator=(const params&);

		private:
			typedef std::string value_type;
			std::map<value_type,value_type> _values;
			static const char delim = '=';

		public:
			static params& instance () 
			{
				static params _instance;
				return _instance;
			}
			
			// load pair of strings
			void load (const std::string& file)
			{ 
				std::ifstream iff (file.c_str());
				std::istream_iterator<value_type> it(iff), end;
				while (it != end)
				{// uff ;)
					*this << *it++;
				}
				iff.close();
			}

			// return default_value if not found
			template<typename T>
			T value (const value_type& key, T default_value) const
			{
					if (_values.find(key) == _values.end())
						return default_value;
				std::stringstream off (_values.find(key)->second);
				T tmp;
				off >> tmp;
				return tmp;
			}

			friend params& operator<<(params&, const value_type&);
	};

	inline
	params& operator << (params& p, const params::value_type& str)
	{
		size_t found = str.find (params::delim);
			if (found == std::string::npos)
				return p;
		p._values.insert (std::make_pair (str.substr(0,found), str.substr(found+1)));
		return p;
	}


} // namespace utils

