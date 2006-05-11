// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80

/*
 * $RCSfile$ 
 *
 * $Log$
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

namespace pdfobjects {

// TODO page has to be valid (pdf and reference) because annotDict is stored as
// reference inside Annots array. Meaningfull reference needs PDF
CAnnotation::CAnnotation(shared_ptr<CDict> annotDict, shared_ptr<CDict> page):annotDictionary(annotDict), valid(true)
{
using namespace debug;
using namespace utils;

	kernelPrintDbg(DBG_DBG, "");
	
	// page dictionary has to have valid pdf, because annotations are indirect
	// objects available by reference in Annots page dictionart
	if(isInValidPdf(*page))
	{
		kernelPrintDbg(DBG_ERR, "Page dictionary is not in pdf. ");
		// TODO handle	
	}
	
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
	if(isInValidPdf(annotDict))
		annotRef=pdf->addIndirectProperty(annotDict);
	
	CRef annotCRef(annotDict);
	vector<CArray::PropertyId> container;
	getPropertyId(annotArray, annotCRef, container);
	if(!container.size())
	{
		// adds annotation reference to the array
		// TODO may be invalid reference and then addIndirectProperty has to be
		// called
		// This will produce annotation observer notification
		kernelPrintDbg(DBG_DBG, "AnnotDict not found in Annots array. Appending to the pos="<<annotDict->getPropertyCount());
		CRef annotCRef(annotRef);
		annotArray->addProperty(annotCRef);
	}
	
}

} // end of pdfobjects namespace
