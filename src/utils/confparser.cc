// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80
/*
 * $RCSfile$
 *
 * $Log$
 * Revision 1.1  2006/06/01 08:49:12  hockm0bm
 * * confparser.cc module added
 * * tokenizer function added
 *
 *
 */

#include "confparser.h"

using namespace configuration;

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

	// if no token found - no deliminer in text - we will return only one token
	// containing whole text
	if(!tokenCount)
	{
		tokenCount++;
		tokens.push_back(text);
	}

	return tokenCount;
}
