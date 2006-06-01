// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80
/*
 * $RCSfile$
 *
 * $Log$
 * Revision 1.3  2006/06/01 14:05:05  hockm0bm
 * * trim method added
 * * tokenizer method signature changed
 *         - text and deliminers parameters are const & now
 *
 * Revision 1.2  2006/06/01 09:12:30  hockm0bm
 * tokenizer function
 *         - moved to configuration::utils namespace
 *         - empty strings are not parsed
 *         - bug fix last token is also considered (when text is not
 *           finished by deliminer)
 *
 * Revision 1.1  2006/06/01 08:49:12  hockm0bm
 * * confparser.cc module added
 * * tokenizer function added
 *
 *
 */

#include "confparser.h"

namespace configuration
{

namespace utils
{
	
size_t tokenizer(const std::string & text, const std::string & deliminers, std::vector<std::string> & tokens)
{
using namespace std;

	size_t tokenStart=0;
	size_t tokenEnd=0;
	size_t tokenCount=0;

	while((tokenEnd=text.find_first_of(deliminers, tokenStart))!=string::npos)
	{
		// token found, makes copy of [tokenStart, tokenEnd) range from text and
		// moves tokenStart behind deliminer position (tokenEnd)
		tokenCount++;
		string token;
		token.assign(text, tokenStart, tokenEnd-tokenStart);
		tokens.push_back(token);
		tokenStart=tokenEnd+1;
	}

	// inserts last token - if tokenStart is not at the end (or behind)
	if(tokenStart<text.length())
	{
		tokenCount++;
		string token;
		token.assign(text, tokenStart, text.length()-tokenStart);
		tokens.push_back(token);
	}

	return tokenCount;
}

std::string & trim(std::string & str, const std::string & blankSet)
{
using namespace std;

	// removes everything behind last non blank
	string::size_type pos = str.find_last_not_of(blankSet);
	if(pos != string::npos) 
	{
		// erases everything behind last non blank
		str.erase(pos + 1);

		// gets first non blank from the begining
		// and removes everything before if found
		pos = str.find_first_not_of(' ');
		if(pos != string::npos) 
			str.erase(0, pos);
	}
	else
		// given string is full of blanks
		str.erase(str.begin(), str.end());

	return str;
}

} // namespace configuration::utils
} // namespace configuration
