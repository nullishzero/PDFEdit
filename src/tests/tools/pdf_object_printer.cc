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

#include <boost/program_options.hpp>
#include <boost/shared_ptr.hpp>
#include <kernel/pdfedit-core-dev.h>
#include <kernel/cpdf.h>
#include <string>

using namespace pdfobjects;
using namespace std;
using namespace boost;
namespace po = program_options;

bool decodeStream = false;
typedef std::vector<pdfobjects::IndiRef> RefContainer;

void getStringRepresentation(boost::shared_ptr<IProperty> &prop, std::string &str)
{
	if(!decodeStream || !isStream(prop))
	{
		prop->getStringRepresentation(str);
		return;
	}

	// we want to have decoded stream
	boost::shared_ptr<CStream> streamProp = IProperty::getSmartCObjectPtr<CStream>(prop);
	streamProp->getDecodedStringRepresentation(str);
}


void print_objects(const char *fname, RefContainer &refs)
{
	boost::shared_ptr<CPdf> pdf = pdfobjects::CPdf::getInstance(fname, CPdf::ReadOnly);

	std::cout << "Document: \"" << fname << "\"" << std::endl;
	RefContainer::const_iterator i;
	for (i=refs.begin(); i!=refs.end(); ++i)
	{
		IndiRef ref = *i;
		boost::shared_ptr<IProperty> prop = pdf->getIndirectProperty(ref);
		std::string str;
		getStringRepresentation(prop, str);
		std::cout << "[" << ref << "]:" << std::endl << str << std::endl;
	}
}
int main(int argc, char ** argv)
{
	if(pdfedit_core_dev_init())
	{
		std::cerr << "Unable to initialize pdfedit-dev" << std::endl;
		return 1;
	}

	typedef vector<string> RefsRepr;
	
	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce help message")
		("file", po::value<string>(), "Input pdf file")
		("ref", po::value<vector<string> >(), "Reference to object which should be printed e.g. \"1 0\".")
		("decode", po::value<bool>()->default_value(false), "True if streams should be decoded too.")
	;
	
	po::variables_map vm;
	try {
		po::store(po::parse_command_line(argc, argv, desc), vm);
		po::notify(vm);    
	}catch(std::exception& e)
	{
		std::cout << "exception - " << e.what() << ". Please, check your parameters." << endl;
		return 1;
	}    

		if (vm.count("help") || !vm.count("ref") || !vm.count("file") )
		{
			cout << desc << "\n";
			return 1;
		}

	string input_file = vm["file"].as<string>(); 
	decodeStream = vm["decode"].as<bool>(); ;

	RefContainer refs;
	RefsRepr refs_repr = vm["ref"].as<RefsRepr>(); 

	try {
		for (RefsRepr::const_iterator it = refs_repr.begin();
				it != refs_repr.end();
				++it)
		{
			IndiRef ref;
			utils::simpleValueFromString(*it, ref);
			if (isRefValid(&ref))
				refs.push_back(ref);
		}
	
		print_objects(input_file.c_str(), refs);

	}catch (std::exception& e)
	{
		cout << e.what() << "\n";
		cout << desc << "\n";
		return 1;
	}

	pdfedit_core_dev_destroy();
	return 0;
}
