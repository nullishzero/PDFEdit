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
#include "common.h"
using namespace pdfobjects;

boost::shared_ptr<pdfobjects::CPdf> openDocument(const char *fname, pdfobjects::CPdf::OpenMode mode)
{
	boost::shared_ptr<pdfobjects::CPdf> pdf;
	try
	{
		pdf = pdfobjects::CPdf::getInstance(fname, mode);
	}catch(...)
	{
		std::cerr << fname << " is not a valid pdf document" << std::endl;
		exit(EXIT_FAILURE);
	}
	return pdf;
}
int add_ref(RefContainer &refs, const char *refStr)
{
	std::string str = refStr;
	IndiRef ref;
	try
	{
		utils::simpleValueFromString(str, ref);
	}catch (CObjBadValue &)
	{
		return 1;
	}
	if (!isRefValid(&ref))
		return 1;
	refs.push_back(ref);
	return 0;
}

int parseInt(const char *str, long &value)
{
	char * end;
	if(!(*str))
	{
		value = 0;
		return 0;
	}
	value = strtol(str, &end, 10);
	if(!(*end))
		return 0;
	return -1;
}

void add_to_range(PagePosList &pagePosList, int page)
{
	// make it sorted list
	if(find(pagePosList.begin(), pagePosList.end(), page) == pagePosList.end())
		pagePosList.push_back(page);
}

int add_page_range(PagePosList &pagePos, const char *posStr)
{
	// only simple number or simple range
	const char *rangeStart = strchr(posStr, '-');
	if(!rangeStart)
	{
		long value;
		if(parseInt(posStr, value))
			return -1;
		add_to_range(pagePos, value);
		return 0;
	}

	size_t startLen = rangeStart-posStr;
	size_t endLen = strlen(posStr)-startLen;
	boost::scoped_array<char> start (new char[startLen+1]);
	boost::scoped_array<char> end (new char[endLen+1]);
	strncpy(start.get(), posStr, startLen);
	start[startLen] = '\0';
	strncpy(end.get(), rangeStart+1, endLen);
	end[endLen] = '\0';

	long startValue, endValue;
	if(parseInt(start.get(), startValue))
		return -1;
	if(parseInt(end.get(), endValue))
		return -1;
	for (;startValue<=endValue; ++startValue)
		add_to_range(pagePos, startValue);
	return 0;
}
