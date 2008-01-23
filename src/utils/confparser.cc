/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko,
 *                                              Miroslav Jahoda,
 *                                              Jozef Misutka,
 *                                              Martin Petricek
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit
 */
// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80
#include "kernel/static.h"
#include "utils/confparser.h"

/* TODO - tune for performance - to many string object creation - replace by
 * referencies
 */
namespace configuration
{

namespace utils
{
	
size_t tokenizer(const std::string & text, const std::string & delimiters, std::vector<std::string> & tokens)
{
using namespace std;

	size_t tokenStart=0;
	size_t tokenEnd=0;
	size_t tokenCount=0;

	while((tokenEnd=text.find_first_of(delimiters, tokenStart))!=string::npos)
	{
		// token found, makes copy of [tokenStart, tokenEnd) range from text and
		// moves tokenStart behind delimiter position (tokenEnd)
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
