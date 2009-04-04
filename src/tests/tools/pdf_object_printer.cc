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
#include "common.h"

using namespace pdfobjects;

bool decodeStream = false;

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
	boost::shared_ptr<CPdf> pdf = openDocument(fname, CPdf::ReadOnly);

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

	int opt;
	RefContainer refs;
	while ((opt = getopt(argc, argv, "dr:")) != -1 )
	{
		switch (opt)
		{
			case 'r':
				if(add_ref(refs, optarg))
				{
					std::cerr << optarg 
						<< " is not a valid reference" 
						<< std::endl;
					exit(EXIT_FAILURE);
				}
				break;
			case 'd':
				decodeStream = true;
				break;
			default:
				std::cerr << "Bad parameter" << std::endl; 
		}
	}
	if (optind >= argc)
	{
		std::cerr << "Filename expected" << std::endl;
		exit(EXIT_FAILURE);
	}

	for(int i=optind;i<argc; ++i)
	{
		const char * fname = argv[i];
		print_objects(fname, refs);
	}
	pdfedit_core_dev_destroy();
	return 0;
}
