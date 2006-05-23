// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80

/*
 * $RCSfile$ 
 *
 * $Log$
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
#include"cobjecthelpers.h"

using namespace boost;
using namespace std;

namespace pdfobjects {

namespace utils {

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

/** Checks and replaces mapping in given dictionary.
 * @param annotDict Dictionary to check.
 * @param fieldName Field name to search.
 * @param fieldValue New value for this fieldName.
 *
 * Sets new value of given fieldName field and returns true if this property was
 * replaced, false if it was added.
 *
 * @return true if given fieldName's value has been overwirten, false otherwise.
 */
bool checkAndReplace(shared_ptr<CDict> annotDict, string fieldName, IProperty & fieldValue)
{
	shared_ptr<IProperty> value;
	try
	{
		value=annotDict->getProperty(fieldName);
		annotDict->setProperty(fieldName, fieldValue);
		return true;
	}catch(ElementNotFoundException &e)
	{
		// annotDict doesn't contain fieldName so one has to be added
		annotDict->addProperty(fieldName, fieldValue);
		return false;
	}
}

IAnnotInitializator::SupportedList TextAnnotInitializer::getSupportedList()const
{
	SupportedList list;
	list.push_back("Text");

	return list;
}

// initializes static constant
bool TextAnnotInitializer::OPEN=true;

bool TextAnnotInitializer::operator()(boost::shared_ptr<CDict> & annotDict, std::string annotType)const
{
	if(annotType!="Text")
		return false;

	IProperty * defaultSubType=CNameFactory::getInstance("Text");
	checkAndReplace(annotDict, "Subtype", *defaultSubType);
	delete defaultSubType;

	IProperty * defaultOpen=CBoolFactory::getInstance(OPEN);
	checkAndReplace(annotDict, "Open", *defaultOpen);
	delete defaultOpen;

	return true;
}

} // namespace utils

// initialization of static
shared_ptr<utils::IAnnotInitializator> 
CAnnotation::annotInit=shared_ptr<utils::IAnnotInitializator>(new utils::UniversalAnnotInitializer());

// TODO page has to be valid (pdf and reference) because annotDict is stored as
// reference inside Annots array. Meaningfull reference needs PDF
CAnnotation::CAnnotation(shared_ptr<CDict> annotDict, shared_ptr<CDict> page, string annotType)
	:valid(true)
{
using namespace debug;
using namespace utils;

	kernelPrintDbg(DBG_DBG, "");
	
	// page dictionary has to have valid pdf and reference, because 
	// annotations are indirect objects available by reference in 
	// Annots page dictionart
	if(!hasValidRef(page) || !hasValidPdf(page))
	{
		kernelPrintDbg(DBG_ERR, "Page dictionary is not in pdf. ");
		valid=false;
		// TODO handle	
	}
	CPdf * pdf=page->getPdf();
	
	shared_ptr<CArray> annotArray;

	// gets annotation array - if no presented, creates one
	try
	{
		shared_ptr<IProperty> annotArrayProp=page->getProperty("Annots");
		if(! isArray(*annotArrayProp))
		{
			kernelPrintDbg(DBG_WARN, "Annots field in Page dictionary is not an array.");
			// TODO handle
		}
		annotArray=IProperty::getSmartCObjectPtr<CArray>(annotArrayProp);
		
	}catch(ElementNotFoundException & e)
	{
		IProperty * array=new CArray();
		/* Annots entry not found, so ther has to be one created */
		page->addProperty("Annots", *array);
		delete array;

		// gets added array, because addProperty makes deep copy of given data
		annotArray=IProperty::getSmartCObjectPtr<CArray>(page->getProperty("Annots"));
		// TODO registers observer to array
	}

	// if annotDict doesn't belong to same pdf as page, adds it as indirect 
	// property to same pdf
	IndiRef annotRef=annotDict->getIndiRef();
	if(!hasValidPdf(annotDict))
		annotRef=pdf->addIndirectProperty(annotDict);
	
	// checks whether annotation's reference is in Annots array. If not inserts
	// it.
	shared_ptr<CRef> annotCRef(CRefFactory::getInstance(annotRef));
	vector<CArray::PropertyId> container;
	getPropertyId(annotArray, annotCRef, container);
	if(!container.size())
	{
		// adds annotation reference to the array
		// This will produce annotation observer notification
		kernelPrintDbg(DBG_DBG, "AnnotDict not found in Annots array. Appending to the pos="<<annotDict->getPropertyCount());
		CRef annotCRef(annotRef);
		annotArray->addProperty(annotCRef);
	}
	
	// annotation dictionary is in pdf now and also page contains reference to
	// it. We can resolve it from annotRef
	shared_ptr<IProperty> annotProp=pdf->getIndirectProperty(annotRef);
	this->annotDictionary=IProperty::getSmartCObjectPtr<CDict>(annotProp);

	// initializes annotation dictionary according desired type - at least Type
	// and Subtype are initialized if initializator implementation doesn't
	// support given type
	bool initialized=false;
	if(annotInit)
		initialized=(*annotInit)(this->annotDictionary, annotType);

	if(!initialized)
		kernelPrintDbg(DBG_WARN, "Unable to initialize annotation dictionary with type="<<annotType);
}

} // end of pdfobjects namespace
