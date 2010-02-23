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
using namespace pdfobjects::utils;
using namespace std;
using namespace boost;
namespace po = program_options;

typedef std::vector<pdfobjects::IndiRef> RefContainer;

std::string appendRefsToContext(const std::string &prefix, const IndiRef &r1, const IndiRef &r2)
{
	std::ostringstream oss(prefix);
	oss << "::" << r1 << "_" << r2;
	return oss.str();
}

std::string appendNameToContext(const std::string &prefix, const std::string &name)
{
	std::string out = prefix + "::[" + name + "]";
	return out;
}

std::string appendIndexToContext(const std::string &prefix, int index)
{
	std::ostringstream oss(prefix);
	oss<< "::[" << index << "]";
	return oss.str();
}

std::ostream& operator<< (std::ostream& out, const std::pair<int, int> &pair)
{
	out << "["<<pair.first <<","<<pair.second<<"]";
	return out;
}

/* prints all elements from given container and if context is non NULL
 * adds @context suffix
 */
template<typename Container>
void printContainer(const Container &container, std::string *context = NULL)
{
	typename Container::const_iterator i;
	for(i=container.begin(); i!=container.end(); ++i)
	{
		std::cout << " " << (*i);
		if(context)
			std::cout << "@"<<*context;
	}
}

template<typename Type, typename Container>
bool containsElem(const Type &elem, Container &container, bool remove)
{
	typename Container::iterator i;
	for(i=container.begin(); i!=container.end(); ++i)
	{
		Type e = *i;
		if (elem == e)
		{
			if (remove)
				container.erase(i);
			return true;
		}
	}
	return false;
}

/* Comparer for simple types
 */
template<typename ItemType>
int typeCmp(const boost::shared_ptr<IProperty> &p1, const boost::shared_ptr<IProperty> &p2, 
		const std::string &context)
{
	typename ItemType::Value p1Value = getValueFromSimple<ItemType>(p1), 
		p2Value = getValueFromSimple<ItemType>(p2);
	if(p1Value != p2Value)
	{
		std::cout<< context << ":"<< p1Value << " != "<<p2Value <<std::endl;
		return 1;
	}
	return 0;
}	

// fwd declarations
int arrayCmp(const boost::shared_ptr<IProperty> &p1, const boost::shared_ptr<IProperty> &p2, const std::string &context);
int dictCmp(const boost::shared_ptr<IProperty> &p1, const boost::shared_ptr<IProperty> &p2, const std::string &context);
int streamCmp(const boost::shared_ptr<IProperty> &p1, const boost::shared_ptr<IProperty> &p2, const std::string &context);

/* Compares generic property and uses given context for
 * printing.
 */
bool propertyCmp(const boost::shared_ptr<IProperty> &p1, const boost::shared_ptr<IProperty> &p2, const std::string &context)
{
	PropertyType p1Type = p1->getType(),
		     p2Type = p2->getType();

	if( p1Type != p2Type )
	{
		std::cout<<context<<"::TypeMismatch:"<< p1Type <<" != "<<p2Type <<std::endl;
		return 1;
	}
	switch(p1Type){
		case pBool:
			return typeCmp<CBool>(p1, p2, context);
		case pInt:
			return typeCmp<CInt>(p1, p2, context);
		case pReal:
			return typeCmp<CReal>(p1, p2, context);
		case pString:
			return typeCmp<CString>(p1, p2, context);
		case pName:
			return typeCmp<CName>(p1, p2, context);
		case pRef:
		{
			IndiRef p1Ref = getValueFromSimple<CRef>(p1),
				p2Ref = getValueFromSimple<CRef>(p2);

			if (! (p1Ref == p2Ref))
			{
				std::cout<<context<<":"<< 
					p1Ref << " != " << p2Ref << std::endl;
				return 1;
			}
			/* TODO - needs the tracking of already seen indirect objects
			 * TODO - enable only by parameter --deep
			boost::shared_ptr<CPdf> pdf1 = p1->getPdf().lock(),
				pdf2 = p2->getPdf().lock();
			boost::shared_ptr<IProperty> p1Target = pdf1->getIndirectProperty(p1Ref),
				p2Target = pdf2->getIndirectProperty(p2Ref);
			
			std::string newContext = appendRefsToContext(context, p1Ref, p2Ref);
			return propertyCmp(p1Target, p2Target, newContext);
			*/
			break;
		}
		case pArray:
			return arrayCmp(p1, p2, context);
		case pDict:
			return dictCmp(p1, p2, context);
		case pStream:
			return streamCmp(p1, p2, context);
		default:
			// fall through for other types
			break;
	}
	return 0;
}

/* compares given array properties and uses given context for
 * printing.
 */
int arrayCmp(const boost::shared_ptr<IProperty> &p1, const boost::shared_ptr<IProperty> &p2, const std::string &context)
{
	int ret = 0;

	boost::shared_ptr<CArray> p1Array = IProperty::getSmartCObjectPtr<CArray>(p1),
		p2Array = IProperty::getSmartCObjectPtr<CArray>(p2);

	size_t p1ArrayCount = p1Array->getPropertyCount(),
		p2ArrayCount = p2Array->getPropertyCount();

	if (p1ArrayCount != p2ArrayCount)
	{
		ret++;
		std::cout << context << ":ArrayCountMismatch " 
			<< p1ArrayCount 
			<< " != " 
			<< p2ArrayCount 
			<< std::endl;
	}
	for(size_t i=0; i < std::min(p1ArrayCount, p2ArrayCount); ++i)
	{
		std::string newContext = appendIndexToContext(context, i);
		boost::shared_ptr<IProperty> p1Child = p1Array->getProperty(i),
			p2Child = p2Array->getProperty(i);
		if (propertyCmp(p1Child, p2Child, newContext))
			ret++;
	}
	return ret;
}

/* Common comparision for CDict and CStream which both export
 * the same dictionary interface but CStream doesn't export CDict
 */
template<typename Type>
int nameListCmp(const Type & prop1, const Type & prop2, const std::string &context)
{
	int ret = 0;
	std::vector<std::string> p1Names, p2Names;
	prop1->getAllPropertyNames(p1Names);
	prop2->getAllPropertyNames(p2Names);

	std::vector<std::string> common;
	std::vector<std::string>::iterator i;
	// removes all names which are in both lists - those which
	// stay are incompatible
	for(i=p1Names.begin(); i!= p1Names.end();)
	{
		if (containsElem(*i, p2Names, true))
		{
			common.push_back(*i);
			i = p1Names.erase(i);
			continue;
		}
		++i;
	}
	if (p1Names.size() || p2Names.size())
	{
		std::cout<<context <<":DictNamesMismatch:";
		ret+=p1Names.size()+p2Names.size();
		std::string c = "1";
		printContainer(p1Names,&c);
		c = "2";
		printContainer(p2Names,&c);
		std::cout<<std::endl;
	}
	for (i=common.begin(); i!=common.end(); ++i)
	{
		std::string name = *i;
		std::string newContext = appendNameToContext(context, name);
		boost::shared_ptr<IProperty> p1Child = prop1->getProperty(name),
			p2Child = prop2->getProperty(name);
		if (propertyCmp(p1Child, p2Child, newContext))
			ret++;

	}
	return ret;
}

/* compares given dict properties and uses given context for
 * printing.
 */
int dictCmp(const boost::shared_ptr<IProperty> &p1, const boost::shared_ptr<IProperty> &p2, const std::string &context)
{
	boost::shared_ptr<CDict> p1Dict = IProperty::getSmartCObjectPtr<CDict>(p1),
		p2Dict = IProperty::getSmartCObjectPtr<CDict>(p2);
	return nameListCmp(p1Dict, p2Dict, context);
}

/* compares given stream properties and uses given context for
 * printing.
 */
int streamCmp(const boost::shared_ptr<IProperty> &p1, const boost::shared_ptr<IProperty> &p2, 
		const std::string&context)
{
	boost::shared_ptr<CStream> p1Stream = IProperty::getSmartCObjectPtr<CStream>(p1),
		p2Stream = IProperty::getSmartCObjectPtr<CStream>(p2);

	// checks the dictionary first
	std::string newContext = context + "::stream";
	int ret = nameListCmp(p1Stream, p2Stream, newContext);

	CStream::Buffer buf1 = p1Stream->getBuffer(),
		buf2 = p2Stream->getBuffer();

	if (buf1.size() != buf2.size())
	{
		std::cout<<newContext<<":StreamDataSizeMismatch:"<<
			buf1.size() << " != " << buf2.size()<<std::endl;
	}
	// collection of different hunks (pairs of offset and length)
	std::vector<std::pair<int, int> > diffs;
	std::vector<std::pair<int, int> >::iterator hunk = diffs.begin();
	size_t diffLen = 0;
	for(size_t i=0; i<std::min(buf1.size(), buf2.size()); ++i)
	{
		CStream::StreamChar ch1 = buf1[i], ch2=buf2[i];
		if(ch1!=ch2)
		{
			// new  hunk
			if (hunk == diffs.end())
				hunk = diffs.insert(hunk, std::pair<int,int>(i,0));
			hunk->second++;
		}else 
			// finish hunk if we have a match
			if(hunk!=diffs.end())
			{
				diffLen += hunk->second;
				hunk = diffs.end();
			}
	}
	// print all hunks
	if(diffs.size())
	{	
		std::cout<<newContext<<":MismatchingData:";
		printContainer(diffs);
		std::cout<< ":" <<
			(double)diffLen*100/std::min(buf1.size(), buf2.size()) 
			<<"% of mismatches" << std::endl;
		ret += diffs.size();
	}
	return ret;
}

/* simple startup. Resolves given properties and delegates the 
 * rest to propertyCmp.
 */
int compare_object(boost::shared_ptr<CPdf> &pdf1, boost::shared_ptr<CPdf> &pdf2, IndiRef &r1, IndiRef &r2)
{
	std::string context = appendRefsToContext("", r1, r2);
	boost::shared_ptr<IProperty> pdf1Prop = pdf1->getIndirectProperty(r1);
	boost::shared_ptr<IProperty> pdf2Prop = pdf2->getIndirectProperty(r2);

	return propertyCmp(pdf1Prop, pdf2Prop, context);
}


// compares pairs of references for both files. If there is no pair
// fo the reference then the same one is used for both files
int compare_objects(const char*f1, const char*f2, RefContainer& refs)
{
	boost::shared_ptr<CPdf> pdf1 = pdfobjects::CPdf::getInstance(f1, CPdf::ReadOnly),
		pdf2 = pdfobjects::CPdf::getInstance(f2, CPdf::ReadOnly);

	RefContainer::iterator i;
	std::cout<<"Comparing \""<<f1<<"\" and \""<<f2<<"\""<<std::endl;
	int ret = 0;
	for(i=refs.begin(); i!=refs.end();)
	{
		IndiRef ref1(*i);
		++i;
		IndiRef ref2 = (i==refs.end())?ref1:*i++;
		if (compare_object(pdf1, pdf2, ref1, ref2))
			ret++;
	}
	return ret;
}

int main(int argc, char ** argv)
{
	if(pdfedit_core_dev_init())
	{
		std::cerr << "Unable to initialize pdfedit-dev" << std::endl;
		return 1;
	}

	typedef vector<string> RefsRepr;

	po::options_description desc("pdf_object_comparer [-h] [-d] (-r ref)+ file1 file2\n\n"
		"where\n"
		"\t-h - prints this help\n"
		"\t(-r ref)+ - references to be used for comparing. All pairs are\n"
		"\t\tsplit among file1 and file2. If there is odd number of references,\n"
		"\t\tthe last one is used for both files.\n"
		"Program will print all mismatching objects (defined by refs) from given documents\n"
		"and returns the number of mismatches.");

	desc.add_options()
		("help", "produce help message")
		("file1", po::value<string>(), "First input pdf file")
		("file2", po::value<string>(), "Second input pdf file")
		("ref", po::value<vector<string> >(), "Reference to object which should be printed e.g. \"1 0\".")
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

		if (vm.count("help") || !vm.count("ref") || !vm.count("file1") || !vm.count("file2"))
		{
			cout << desc << "\n";
			return 1;
		}

	string file1 = vm["file1"].as<string>(); 
	string file2 = vm["file2"].as<string>(); 
	RefsRepr refs_repr = vm["ref"].as<RefsRepr>(); 

	int ret = 0;
	RefContainer refs;

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
	
		ret = compare_objects(file1.c_str(), file2.c_str(), refs);

	}catch (std::exception& e)
	{
		cout << e.what() << "\n";
		cout << desc << "\n";
		return 1;
	}

	pdfedit_core_dev_destroy();
	return ret;
}

