/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006, 2007, 2008  PDFedit team: Michal Hocko,
 *                                              Miroslav Jahoda,
 *                                              Jozef Misutka,
 *                                              Martin Petricek
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
#include <sstream>
#include <kernel/pdfedit-core-dev.h>
#include <kernel/cpdf.h>
#include <kernel/cpage.h>
#include <kernel/ccontentstream.h>

using namespace boost;
using namespace pdfobjects;
using namespace std;

namespace {
	template<typename T, typename U>
	T _to(const U& val)
	{
		T ret = T();
		istringstream iss (val);
		iss >> ret;
		return ret;
	}
}

int main(int argc, char ** argv)
{
	if (pdfedit_core_dev_init(&argc, &argv))
		return 1;

	std::vector<string> args (argv+1, argv+argc);

	if (args.size() < 3)
		return 1;

	string filename = args[0];
	string frompage = args[1];
	string topage = args[2];

	//
	shared_ptr<CPdf> pdf = CPdf::getInstance (filename.c_str(), CPdf::ReadWrite);

	//
	cout << "# Pages: " << pdf->getPageCount () << endl;
	
	// speciality
	for (int i = _to<int,string>(frompage); i < std::min (_to<unsigned int,string>(topage), pdf->getPageCount()); ++i)
	{
		shared_ptr<CPage> page = pdf->getPage(i);
		typedef vector<shared_ptr<CContentStream> > CCs;
		CCs ccs;
		page->getContentStreams (ccs);
		std::cout << "PAGE NUMBER " << i << endl;
		std::cout << "=====================" << endl;
		for (CCs::const_iterator it = ccs.begin(); it != ccs.end(); ++it)
		{
			string str;
			(*it)->getStringRepresentation (str);
			std::cout << str << flush;
		}
	}
	
	gMemReport(stdout);
    pdfedit_core_dev_destroy();
	return 0;
}
