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
#include "kernel/pdfedit-core-dev.h"
#include "kernel/flattener.h"
#include "kernel/pdfwriter.h"
#include "utils/debug.h"

using namespace pdfobjects;
#define suffix ".flatten"
int flatten_file(const char *fname)
{
using namespace utils;
	boost::shared_ptr<utils::Flattener> flattener = 
		Flattener::getInstance(fname, new OldStylePdfWriter()); 
	if(!flattener) {
		std::cerr << "Unable to open "<<fname<<" file"<<std::endl;
		return 1;
	}
	std::string outputFile(fname);
	outputFile+=suffix;
	std::cout << "Writing output to "<<outputFile<<std::endl;
	return flattener->flatten(outputFile.c_str());
}

int main(int argc, char** argv)
{
	if(pdfedit_core_dev_init())
	{
		std::cerr << "Unable to initialize pdfedit-dev" << std::endl;
		return 1;
	}
	//debug::changeDebugLevel(debug::utilsDebugTarget, debug::DBG_DBG);
	int ret = 0;
	for(int i=1; i<argc; ++i)
	{
		const char *fname= argv[i];
		try
		{
			ret = flatten_file(fname);
		}catch(...)
		{
			std::cerr << fname << " is not a valid pdf document - ignoring"<<std::endl;
			ret = 1;
		}
	}
	pdfedit_core_dev_destroy();
	return ret;
}
