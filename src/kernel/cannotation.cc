// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80

/*
 * $RCSfile$ 
 *
 * $Log$
 * Revision 1.6  2006/06/17 15:04:48  misuj1am
 *
 * -- include rem/add
 *
 * Revision 1.5  2006/06/05 08:57:32  hockm0bm
 * refactoring CObjectSimple
 *         - getPropertyValue -> getValue
 *         - writeValue -> setValue
 *
 * Revision 1.4  2006/06/02 16:54:06  hockm0bm
 * * checkAndReplace removed (and placed to cobjecthelpers.h)
 * * CAnnotation constructor with page, rect, annotType parameters removed
 * * createAnnotation static added
 * * getDictionary method added
 *
 * Revision 1.3  2006/05/30 21:00:25  hockm0bm
 * * annotTypeMapping utils method added
 * * default static values for TextAnnotInitializer
 * * TextAnnotInitializer functor
 *         - almost complete:
 *           Just some not very offten used fields are not done (but they
 *           have default value according pdf specification).
 *           It is not very important to add them
 * * CAnnotation initialization constructor
 *         - signature changed - uses CPage instead of page dictionary,
 *           annotDict parameter removed (one is created in constructor),
 *           rect parameter added (rectangle for annotation),
 *           adding to the page Annots array is done by CPage interface
 *         - initializes Type, Rect, P, M annotation dictionary fields
 * * CAnnotation class extends IObserverHandler
 *         - because of invalidation
 * * CAnnotation::invalidate method added
 *         - notifies all observers and sets valid flag to false
 * * CAnnotation::isValid method added
 * * CAnnotation::AnnotType enmumeration for known types
 * * CAnnotation::getType method added
 *
 * Revision 1.2  2006/05/23 19:12:03  hockm0bm
 * * IAnnotInitializator added
 * * UniversalAnnotInitializer implemented
 * * TextAnnotInitializer
 *         - skeleton implementation
 * * CAnnotation
 *         - static initializer
 *         - constructor with initializer
 *         - constructor with initialized dictionary
 *
 * Revision 1.1  2006/05/11 21:05:48  hockm0bm
 * first scratch of CAnnotation class
 *         - still not in kernel.pro
 *
 *
 */
#include"cannotation.h"
#include"debug.h"
#include"factories.h"

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
	for(AnnotList::const_iterator i=implList.begin(); i!=implList.end(); i++)
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
	shared_ptr<IAnnotInitializator> impl=implElem->second;

	// use registerd implementator
	return (*impl)(annotDict, annotType);
}

bool UniversalAnnotInitializer::registerInitializer(std::string annotType, shared_ptr<IAnnotInitializator> impl, bool forceNew)
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

bool TextAnnotInitializer::operator()(boost::shared_ptr<CDict> & annotDict, std::string annotType)const
{
	if(annotType!="Text")
		return false;

	// Initializes common entries for annotation dictionary - Type, P, Rect fields are
	// already set
	scoped_ptr<IProperty> defaultSubType(CNameFactory::getInstance("Text"));
	checkAndReplace(annotDict, "Subtype", *defaultSubType);

	scoped_ptr<IProperty> defaultContents(CStringFactory::getInstance(CONTENTS));
	checkAndReplace(annotDict, "Contents", *defaultContents);

	scoped_ptr<IProperty> defaultFlags(CIntFactory::getInstance(FLAGS));
	checkAndReplace(annotDict, "F", *defaultFlags);

	// initializes additional entries specific for text annotations according
	// static default values
	scoped_ptr<IProperty> defaultOpen(CBoolFactory::getInstance(OPEN));
	checkAndReplace(annotDict, "Open", *defaultOpen);

	scoped_ptr<IProperty> defaultName(CNameFactory::getInstance(NAME));
	checkAndReplace(annotDict, "Name", *defaultName);

	scoped_ptr<IProperty> defaultState(CStringFactory::getInstance(STATE));
	checkAndReplace(annotDict, "State", *defaultState);

	scoped_ptr<IProperty> defaultStateModel(CStringFactory::getInstance(STATEMODEL));
	checkAndReplace(annotDict, "StateModel", *defaultStateModel);

	return true;
}

} // namespace utils

// initialization of static
shared_ptr<utils::IAnnotInitializator> 
CAnnotation::annotInit=shared_ptr<utils::IAnnotInitializator>(new utils::UniversalAnnotInitializer());

shared_ptr<CAnnotation> CAnnotation::createAnnotation(Rectangle rect, string annotType)
{
using namespace debug;
using namespace utils;

	kernelPrintDbg(DBG_DBG, "");
	
	// creates new empty dictionary for annotation
	shared_ptr<CDict> annotDictionary(CDictFactory::getInstance());

	// initializes annotation dictionary maintaining information:
	// Type of annotation dictionary has to be Annot (this is not strongly
	// required by specification, but it is cleaner to do initialize it)
	scoped_ptr<IProperty> typeField(CNameFactory::getInstance("Annot"));
	checkAndReplace(annotDictionary, "Type", *typeField);

	// Rectangle of annotation is constructed from given parameter
	shared_ptr<IProperty> rectField(getIPropertyFromRectangle(rect));
	checkAndReplace(annotDictionary, "Rect", *rectField);

	// last modified date field (M) is initialized to current local time
	time_t currTime;
	time(&currTime);
	struct tm currLocalTime;
	localtime_r(&currTime, &currLocalTime);
	shared_ptr<IProperty> mField=getIPropertyFromDate(&currLocalTime);
	checkAndReplace(annotDictionary, "M", *mField);
	
	// initializes annotation dictionary according desired type - at least Type
	// and Subtype are initialized if initializator implementation doesn't
	// support given type
	bool initialized=false;
	if(annotInit)
		initialized=(*annotInit)(annotDictionary, annotType);

	if(!initialized)
		kernelPrintDbg(DBG_WARN, "Unable to initialize annotation dictionary with type="<<annotType);
	
	return shared_ptr<CAnnotation>(new CAnnotation(annotDictionary));
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
	shared_ptr<CAnnotation> current(this, EmptyDeallocator<CAnnotation>());
	notifyObservers(current, shared_ptr<BasicObserverContext>());
}

CAnnotation::AnnotType CAnnotation::getType()const
{
using namespace debug;

	try
	{
		shared_ptr<IProperty> subTypeProp=annotDictionary->getProperty("Subtype");
		if(!isName(subTypeProp))
		{
			// TODO may be problem if value is indirect (what is not very usual)
			kernelPrintDbg(DBG_WARN, "Subtype is not Name. Type="<<subTypeProp->getType());
			return Unknown;
		}
		shared_ptr<CName> subTypeName=IProperty::getSmartCObjectPtr<CName>(subTypeProp);
		string typeName;
		subTypeName->getValue(typeName);
		return utils::annotTypeMapping(typeName);
	}catch(ElementNotFoundException & e)
	{
		// Subtype field not present
		kernelPrintDbg(DBG_WARN, "Annotation dictionary doesn't contain Subtype field.");
		return Unknown;
	}
}



} // end of pdfobjects namespace
