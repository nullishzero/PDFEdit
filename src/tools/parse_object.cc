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
#include <sstream>
#include <kernel/pdfedit-core-dev.h>
#include <kernel/cpdf.h>
#include <kernel/cpage.h>
#include <kernel/factories.h>
#include <kernel/ccontentstream.h>
#include <boost/program_options.hpp>


using namespace pdfobjects;
using namespace std;
using namespace boost;
namespace po = program_options;

namespace {
	inline ostream& 
	operator << (ostream& os, Object& o)
	{
		string str;
		
		if (o.isCmd() || o.isEOF())
		{
			os << o.getCmd() << endl;

		}else
		{
			utils::xpdfObjToString (o,str);
		}

		return os;
	}
}

int 
main(int argc, char ** argv)
{
	if (pdfedit_core_dev_init(&argc, &argv))
		return 1;

	// Declare the supported options.
	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce help message")
		("file", po::value<string>(), "set compression level")
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

	if (!vm.count("file")) 
	{
		cout << "File not specified" << endl;
		return 1;
	}
	string file = vm["file"].as<string>(); 

	// xpdf ufff who deletes the stuff? OS idiom  used :)
	ifstream ffile (file.c_str(), ios::in|ios::binary|ios::ate);
	long size = ffile.tellg();
	ffile.seekg (0, ios::beg);
	char* buf = new char[size];
	ffile.read (buf, size);
	ffile.close();

	boost::shared_ptr< ::Object> obj(XPdfObjectFactory::getInstance(), xpdf::object_deleter());
	::MemStream* stream = new ::MemStream (buf, 0, size, obj.get(), true);

	// this is really NASTY
	stream->reset ();
	::Lexer* lexer = new ::Lexer (NULL, stream);
	::Parser parser (NULL, lexer, gTrue);

	fstream f;
	f.open ("log.dat", fstream::in | fstream::out | fstream::app);
	if (!(f.is_open()))
	{
		cout << "could not open file: ";
		return -1;
	}
	while (parser.getObj (obj.get()) && !obj->isEOF())
	{
		//cout << obj << endl;
		if (obj->isStream())
		{
			boost::shared_ptr< ::Object> _obj(XPdfObjectFactory::getInstance(), xpdf::object_deleter());
			Stream* _stream = obj->getStream();
			::Lexer* _lexer = new ::Lexer (NULL, _stream);
			::Parser _parser (NULL, _lexer, gFalse);
			while (_parser.getObj (_obj.get()) && !_obj->isEOF())
			{
				f << *_obj << " ";
				f.flush();
			}
		}
	}
	f.close();

	gMemReport(stdout);
    pdfedit_core_dev_destroy();
	return 0;
}
