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
// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80

#include "kernel/static.h"
#include"kernel/cannotation.h"
#include"kernel/cobject.h"
#include"kernel/cobjecthelpers.h"
#include"utils/debug.h"
#include"kernel/factories.h"

using namespace boost;
using namespace std;

namespace pdfobjects {

namespace utils {

/** Returns CAnnotation enumeration type for given name.
 * @param typeName String representation of type (value of Subtype field from
 * annotation dictionary).
 *
 * Maps given string to CAnnotation::AnnotType.
 *
 * @see CAnnotation::AnnotType
 * @return enumeration representation of annotation type or Unknown if can't
 * recongnize given one.
 */
CAnnotation::AnnotType annotTypeMapping(const std::string & typeName)
{
using namespace std;
	
	typedef map<string, CAnnotation::AnnotType> AnnotMapping;
	// mapping
	static AnnotMapping annotMapping;
	
	// lazy initialization of mapping
	if(annotMapping.empty())
	{
		// These types are mentioned in pdf specification version 1.6
		annotMapping.insert(AnnotMapping::value_type("Text", CAnnotation::Text));
		annotMapping.insert(AnnotMapping::value_type("Link", CAnnotation::Link));
		annotMapping.insert(AnnotMapping::value_type("FreeText", CAnnotation::FreeText));
		annotMapping.insert(AnnotMapping::value_type("Line", CAnnotation::Line));
		annotMapping.insert(AnnotMapping::value_type("Square", CAnnotation::Square));
		annotMapping.insert(AnnotMapping::value_type("Circle", CAnnotation::Circle));
		annotMapping.insert(AnnotMapping::value_type("Polygon", CAnnotation::Polygon));
		annotMapping.insert(AnnotMapping::value_type("PolyLine", CAnnotation::PolyLine));
		annotMapping.insert(AnnotMapping::value_type("Highlight", CAnnotation::Highlight));
		annotMapping.insert(AnnotMapping::value_type("Underline", CAnnotation::Underline));
		annotMapping.insert(AnnotMapping::value_type("Squiggly", CAnnotation::Squiggly));
		annotMapping.insert(AnnotMapping::value_type("StrikeOut", CAnnotation::StrikeOut));
		annotMapping.insert(AnnotMapping::value_type("Stamp", CAnnotation::Stamp));
		annotMapping.insert(AnnotMapping::value_type("Caret", CAnnotation::Caret));
		annotMapping.insert(AnnotMapping::value_type("Ink", CAnnotation::Ink));
		annotMapping.insert(AnnotMapping::value_type("Popup", CAnnotation::Popup));
		annotMapping.insert(AnnotMapping::value_type("Fileattachement", CAnnotation::Fileattachement));
		annotMapping.insert(AnnotMapping::value_type("Sound", CAnnotation::Sound));
		annotMapping.insert(AnnotMapping::value_type("Movie", CAnnotation::Movie));
		annotMapping.insert(AnnotMapping::value_type("Widget", CAnnotation::Widget));
		annotMapping.insert(AnnotMapping::value_type("Screen", CAnnotation::Screen));
		annotMapping.insert(AnnotMapping::value_type("Printermark", CAnnotation::Printermark));
		annotMapping.insert(AnnotMapping::value_type("Trapnet", CAnnotation::Trapnet));
		annotMapping.insert(AnnotMapping::value_type("Watermark", CAnnotation::Watermark));
		annotMapping.insert(AnnotMapping::value_type("3D", CAnnotation::_3D));
	}
	
	AnnotMapping::iterator map=annotMapping.find(typeName);
	if(map!=annotMapping.end())
		// mapping is recognized
		return map->second;

	// unknown annotation type
	return CAnnotation::Unknown;
}

UniversalAnnotInitializer::SupportedList UniversalAnnotInitializer::getSupportedList()const
{
	// returns list with names from implList
	SupportedList list;
	for(AnnotList::const_iterator i=implList.begin(); i!=implList.end(); ++i)
	{
		string annotType=i->first;
		list.push_back(annotType);
	}

	return list;
}

bool UniversalAnnotInitializer::operator()(boost::shared_ptr<CDict> & annotDict, std::string annotType)
{
	// checks if given annotType is supported
	AnnotList::const_iterator implElem;
	if((implElem=implList.find(annotType))==implList.end())
		return false;
	boost::shared_ptr<IAnnotInitializator> impl=implElem->second;

	// use registerd implementator
	return (*impl)(annotDict, annotType);
}

bool UniversalAnnotInitializer::registerInitializer(std::string annotType, boost::shared_ptr<IAnnotInitializator> impl, bool forceNew)
{
	AnnotList::iterator elem=implList.find(annotType);
	if(elem!=implList.end())
	{
		if(!forceNew)
			// keep existing mapping
			return false;

		// this annotation type is already registered but we are forced to
		// rewrite implementation
		elem->second=impl;
		return true;
	}
	
	// this mapping is not registered yet
	implList.insert(AnnotList::value_type(annotType, impl));
	return true;
}


IAnnotInitializator::SupportedList TextAnnotInitializer::getSupportedList()const
{
	SupportedList list;
	list.push_back("Text");

	return list;
}

// initializes static constant
bool TextAnnotInitializer::OPEN=true;
string TextAnnotInitializer::NAME="Comment";
string TextAnnotInitializer::STATE="Unmarked";
string TextAnnotInitializer::STATEMODEL="Marked";
string TextAnnotInitializer::CONTENTS="";
int TextAnnotInitializer::FLAGS=0;

bool TextAnnotInitializer::operator()(boost::shared_ptr<CDict> & annotDict, std::string annotType)
{
	if(annotType!="Text")
		return false;

	// Initializes common entries for annotation dictionary - Type, P, Rect fields are
	// already set
	boost::scoped_ptr<IProperty> defaultSubType(CNameFactory::getInstance("Text"));
	checkAndReplace(annotDict, "Subtype", *defaultSubType);

	boost::scoped_ptr<IProperty> defaultContents(CStringFactory::getInstance(CONTENTS));
	checkAndReplace(annotDict, "Contents", *defaultContents);

	boost::scoped_ptr<IProperty> defaultFlags(CIntFactory::getInstance(FLAGS));
	checkAndReplace(annotDict, "F", *defaultFlags);

	// initializes additional entries specific for text annotations according
	// static default values
	boost::scoped_ptr<IProperty> defaultOpen(CBoolFactory::getInstance(OPEN));
	checkAndReplace(annotDict, "Open", *defaultOpen);

	boost::scoped_ptr<IProperty> defaultName(CNameFactory::getInstance(NAME));
	checkAndReplace(annotDict, "Name", *defaultName);

	boost::scoped_ptr<IProperty> defaultState(CStringFactory::getInstance(STATE));
	checkAndReplace(annotDict, "State", *defaultState);

	boost::scoped_ptr<IProperty> defaultStateModel(CStringFactory::getInstance(STATEMODEL));
	checkAndReplace(annotDict, "StateModel", *defaultStateModel);

	return true;
}


IAnnotInitializator::SupportedList LinkAnnotInitializer::getSupportedList()const
{
	SupportedList list;
	list.push_back("Link");

	return list;
}

// initializes static constant
string LinkAnnotInitializer::CONTENTS="link";
string LinkAnnotInitializer::DEST="";
string LinkAnnotInitializer::H="N";

bool LinkAnnotInitializer::operator()(boost::shared_ptr<CDict> & annotDict, std::string annotType)
{
	if(annotType!="Link")
		return false;

	// Initializes common entries for annotation dictionary - Type, P, Rect fields are
	// already set
	boost::scoped_ptr<IProperty> defaultSubType(CNameFactory::getInstance("Link"));
	checkAndReplace(annotDict, "Subtype", *defaultSubType);

	boost::scoped_ptr<IProperty> defaultContents(CStringFactory::getInstance(CONTENTS));
	checkAndReplace(annotDict, "Contents", *defaultContents);

	boost::scoped_ptr<IProperty> defaultDest(CNameFactory::getInstance(DEST));
	checkAndReplace(annotDict, "Dest", *defaultDest);

	boost::scoped_ptr<IProperty> defaultH(CNameFactory::getInstance(H));
	checkAndReplace(annotDict, "H", *defaultH);

	return true;
}

} // namespace utils

// initialization of static
boost::shared_ptr<utils::IAnnotInitializator> 
CAnnotation::annotInit=boost::shared_ptr<utils::IAnnotInitializator>(new utils::UniversalAnnotInitializer());

boost::shared_ptr<CAnnotation> CAnnotation::createAnnotation(libs::Rectangle rect, string annotType)
{
using namespace debug;
using namespace utils;

	kernelPrintDbg(DBG_DBG, "");
	
	// creates new empty dictionary for annotation
	boost::shared_ptr<CDict> annotDictionary(CDictFactory::getInstance());

	// initializes annotation dictionary maintaining information:
	// Type of annotation dictionary has to be Annot (this is not strongly
	// required by specification, but it is cleaner to do initialize it)
	boost::scoped_ptr<IProperty> typeField(CNameFactory::getInstance("Annot"));
	checkAndReplace(annotDictionary, "Type", *typeField);

	// Rectangle of annotation is constructed from given parameter
	boost::shared_ptr<IProperty> rectField(getIPropertyFromRectangle(rect));
	checkAndReplace(annotDictionary, "Rect", *rectField);

	// last modified date field (M) is initialized to current local time
	time_t currTime;
	time(&currTime);
	struct tm currLocalTime;
	localtime_r(&currTime, &currLocalTime);
	boost::shared_ptr<IProperty> mField=getIPropertyFromDate(&currLocalTime);
	checkAndReplace(annotDictionary, "M", *mField);
	
	// initializes annotation dictionary according desired type - at least Type
	// and Subtype are initialized if initializator implementation doesn't
	// support given type
	bool initialized=false;
	if(annotInit)
		initialized=(*annotInit)(annotDictionary, annotType);

	if(!initialized)
		kernelPrintDbg(DBG_WARN, "Unable to initialize annotation dictionary with type="<<annotType);
	
	return boost::shared_ptr<CAnnotation>(new CAnnotation(annotDictionary));
}

void CAnnotation::invalidate()
{
using namespace boost;
using namespace utils;

	// does nothing for invalidated instances
	if(!isValid())
		return;
	
	// Uses this instance as newValue, but uses EmptyDeallocator to keep
	// instance alive when shared_ptr tries to destroy its content.
	// context is empty
	boost::shared_ptr<CAnnotation> current(this, EmptyDeallocator<CAnnotation>());
	notifyObservers(current, boost::shared_ptr<ChangeContext>());
}

CAnnotation::AnnotType CAnnotation::getType()const
{
using namespace debug;

	try
	{
		boost::shared_ptr<CName> subTypeName=annotDictionary->getProperty<CName>("Subtype");
		string typeName;
		subTypeName->getValue(typeName);
		return utils::annotTypeMapping(typeName);
	}catch(ElementNotFoundException &)
	{
		// Subtype field not present
		kernelPrintDbg(DBG_WARN, "Annotation dictionary doesn't contain Subtype field.");
		return Unknown;
	}catch(...) {
		// bad type
		return Unknown;
	}
}



} // end of pdfobjects namespace
