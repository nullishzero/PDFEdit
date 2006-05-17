// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80
/*
 * =====================================================================================
 *		  Filename:  iproperty.cc
 *	   Description:  IProperty.
 *		   Created:  12/04/2006 00:48:14 PM CET
 *			Author:  jmisutka (), 
 * =====================================================================================
 */

#include "static.h"

#include "iproperty.h"
#include "cobject.h"
#include "cpdf.h"

//=====================================================================================
namespace pdfobjects {
//=====================================================================================


//
// Constructor
//
IProperty::IProperty (CPdf* _pdf) : mode(mdUnknown), pdf(_pdf), wantDispatch (true)
{
	//kernelPrintDbg (debug::DBG_DBG, "IProperty () constructor.");
	
	ref.num = ref.gen = 0;
}

IProperty::IProperty (CPdf* _pdf, const IndiRef& rf) : ref(rf), mode(mdUnknown), pdf(_pdf), wantDispatch (true)
{
	//kernelPrintDbg (debug::DBG_DBG, "IProperty () constructor.");
}

//
// Deep copy
//
boost::shared_ptr<IProperty>
IProperty::clone () const
{
	boost::shared_ptr<IProperty> ip (doClone ());
	assert (typeid (*ip) == typeid (*this) && "doClone INCORRECTLY overriden!!" );
	
	return ip;
}


//
// Set/Get pdf
//
void 
IProperty::setPdf (CPdf* p)
{
	// modify existing association with a pdf? or we just want to reset the pdf
	assert (NULL == pdf || NULL == p);	
	if (NULL != pdf && NULL != p)
		throw CObjInvalidOperation ();
	
	pdf = p;
}


//
// Dispatch change
//
void 
IProperty::dispatchChange () const
{
	assert (hasValidPdf (this));
	assert (hasValidRef (this));
	if (!hasValidPdf (this))
		throw CObjInvalidObject ();

	// If we do not want to dispatch methods return
	if (!wantDispatch)
		return;
	
	//
	// If this is an indirect object inform xref about the change
	// else find the closest indirect object and call dispatchChange on that object
	//
	boost::shared_ptr<IProperty> indiObj;
	if (utils::objHasParent (*this, indiObj))
	{
		if (indiObj)
		{
			assert (getIndiRef() == indiObj->getIndiRef());

			indiObj->dispatchChange ();
			
		}else
		{
			assert (!"Bad parent.");
			throw ElementBadTypeException ("Bad pointer type.");
		}
	}else
	{
		// Indicate to pdf that it should change this object
		CPdf* pdf = IProperty::getPdf ();
		assert (pdf);
		pdf->changeIndirectProperty (indiObj);
	}
}


//
//
//
std::ostream & operator << (std::ostream & out, PropertyType type)
{
	using namespace std;

	string stringType;
	switch(type)
	{
		case pNull:
			stringType=getStringType<pNull>();
			break;
		case pBool:
			stringType=getStringType<pBool>();
			break;
		case pInt:
			stringType=getStringType<pInt>();
			break;
		case pReal:
			stringType=getStringType<pReal>();
			break;
		case pString:
			stringType=getStringType<pString>();
			break;
		case pName:
			stringType=getStringType<pName>();
			break;
		case pRef:
			stringType=getStringType<pRef>();
			break;
		case pArray:
			stringType=getStringType<pArray>();
			break;
		case pDict:
			stringType=getStringType<pDict>();
			break;
		case pStream:
			stringType=getStringType<pStream>();
			break;
		case pOther:
			stringType=getStringType<pOther>();
			break;
		case pOther1:
			stringType=getStringType<pOther1>();
			break;
		case pOther2:
			stringType=getStringType<pOther2>();
			break;
		case pOther3:
			stringType=getStringType<pOther3>();
			break;
	}
	out<<stringType;
	return out;
}

std::ostream& operator<< (std::ostream& out, ::ObjType type)
{
	using namespace std;

	string stringType;
	switch(type)
	{
		case objBool:			// boolean
			stringType=getStringType<pBool>();
			break;
		case objInt:			// integer
			stringType=getStringType<pInt>();
			break;
		case objReal:			// real
			stringType=getStringType<pReal>();
			break;
		case objString:			// string
			stringType=getStringType<pString>();
			break;
		case objName:			// name
			stringType=getStringType<pName>();
			break;
		case objNull:			// null
			stringType=getStringType<pNull>();
			break;

		// complex objects
		case objArray:			// array
			stringType=getStringType<pArray>();
			break;
		case objDict:			// dictionary
			stringType=getStringType<pDict>();
			break;
		case objStream:			// stream
			stringType=getStringType<pStream>();
			break;
		case objRef:			// indirect reference
			stringType=getStringType<pRef>();
			break;

		// special objects
		case objCmd:			// command name
			stringType=getStringType<pOther>();
			break;
		case objError:			// error return from Lexer
			stringType=getStringType<pOther1>();
			break;
		case objEOF:			// end of file return from Lexer
			stringType=getStringType<pOther2>();
			break;
		case objNone:			// uninitialized object
			stringType=getStringType<pOther3>();
			break;
	}

	out<<stringType;
	return out;
}

std::ostream & operator << (std::ostream & out, const IndiRef & ref)
{
	out<< "ref=[";
	out<<ref.num;
	out<<", ";
	out<<ref.gen;
	out<<"]";
	return out;
}

std::ostream & operator << (std::ostream & out, const ::Ref & ref)
{
	IndiRef indiRef(ref.num, ref.gen);
	return out<<indiRef;
}

//=====================================================================================
} // namespace pdfobjects
//=====================================================================================
