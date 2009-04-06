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
#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <iostream>
#include <sstream>
#include <string>

//============================================
namespace _JM_NAMESPACE {
//============================================

/**
 * Default formatter.
 */
struct SimpleFormatter
{
	template<typename T>
	inline void init (T& out,
					 const std::string& header, 
					 const std::string& prefix,
					 const std::string& file,
					 const std::string& function,
					 int line)
	{
		out << header <<":"<<prefix<<":" << file << ":" << function <<":"<< line << ": ";
	}

	template<typename T, typename U>
	inline void msg (T& out, const U& msg) 
	{
		out << msg;
	}

	inline char get () const
	{
		return '\n';
	}


};

/**
 * Logger class providing explicit formatting, runtime output level and 
 * output destination change.
 *<code>
 *	typedef _JM_NAMESPACE::Logger<> KernelLogger;
 *	KernelLogger kernelog ("KERNEL");
 *	#define LOG(logger,msg)	\
 * 		logger.start_log ("level", __FILE__, __FUNCTION__, __LINE__);	\
 * 		logger << msg;	\
 * 		logger.end_log();
 * </code>
 *
 * Advanced usage:
 */
template<typename Formatter = SimpleFormatter>
class Logger
{
private:
	Formatter _ft;
	std::ostream& _out;
	const std::string _header;

public:
	Logger (std::ostream& out, const std::string& header) : _out(out), _header(header) {}
	Logger (const std::string& header) : _out(std::cout),  _header(header) {}
	~Logger () {}

public:
	void output (std::ostream& out)
		{ _out = out; }

public:
	inline void
	start_log (	const std::string& prefix,
				const std::string& file,
				const std::string& function,
				int line) 
	{
		_ft.init (_out, _header, prefix, file, function, line);
	}

	template<typename T>
	inline Logger&
	operator<< (const T& msg)
	{
		_ft.msg (_out, msg);
		return *this;
	}

	inline void
	end_log ()
	{
		_out << _ft.get () << std::flush;
	}

};

//============================================
} // namespace _JM_NAMESPACE
//============================================

// Here is the example for Api logger implementation
//==================================================================
// Api Logger
//==================================================================

/** Api logger formatter. */
// struct ApiFormatter
// {
// 	std::string _msg;
//
// 	template<typename T>
// 	inline void init (T& out,
// 					 const std::string& header, 
// 					 const std::string& prefix,
// 					 const std::string& file,
// 					 const std::string& function,
// 					 int line) 
// 	{
// 		_msg.clear ();
// 		_msg = header + std::string(":") + function + std::string("(");
// 	}
//	
// 	template<typename T, typename U>
// 	inline void msg (T& out, const U& msg)
//	{
//		std::ostringstream oss;
// 		oss << msg;
//		_msg += oss.str();
// 	}
//
//	inline std::string get () const
// 		{ return _msg+std::string(");\n"); }
//};
//
//
///**
// * Api Logger.
// * Static initialization has to be done in object file:
// * ApiLogger::Logger ApiLogger::logger (std::cout, "API");
// */
//struct ApiLogger
//{
//	typedef _JM_NAMESPACE::Logger<ApiFormatter> Logger;
//	static Logger logger;
//};
//
///**
// * Print API debug information.
// * Can be used to create testcases.
// *
// * Every API function exported to GUI should begin with call to this function.
// */
//#define _APIDbg(msg)						 \
//{											 \
//	if ( debug::debugLevel >= debug::DBG_API)\
//	{										 \
//		debug::ApiLogger::logger.start_log ("", __FILE__, __FUNCTION__, __LINE__); \
//		debug::ApiLogger::logger << msg; 	\
//		debug::ApiLogger::logger.end_log(); \
//	}										\
//}
//
//#define APIDbg1(p1)			_APIDbg((p1));
//#define APIDbg2(p1,p2)		_APIDbg((p1) << ", " << (p2));
//#define APIDbg3(p1,p2,p3)	_APIDbg((p1) << ", " << (p2) << ", " << (p3));
//
//

#endif // _LOGGER_H_

