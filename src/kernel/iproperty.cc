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

#include "kernel/iproperty.h"
#include "kernel/cobject.h"
#include "kernel/cpdf.h"

//=====================================================================================
namespace pdfobjects {
//=====================================================================================


//
// Constructor
//
IProperty::IProperty (boost::weak_ptr<CPdf> _pdf) : mode(mdUnknown), pdf(_pdf), wantDispatch (true)
{
	ref.num = ref.gen = 0; 
}

//
// Constructor
//
IProperty::IProperty (boost::weak_ptr<CPdf> _pdf, const IndiRef& rf) 
	: ref(rf), mode(mdUnknown), pdf(_pdf), wantDispatch (true) {}

	
//
// Deep copy
//
boost::shared_ptr<IProperty>
IProperty::clone () const
{
	// Call virtual doClone() to get the right clone
	boost::shared_ptr<IProperty> ip (doClone ());
	if (typeid(*ip) != typeid (*this))
	{
		assert (!"doClone INCORRECTLY overriden!!" );
		throw CObjInvalidCast ();
	}
	assert (typeid (*ip) == typeid (*this));
	
	return ip;
}


//
// Set/Get pdf
//
void 
IProperty::setPdf (boost::weak_ptr<CPdf> p)
	{ pdf = p; }


void
IProperty::canChange () const
{
	boost::shared_ptr<CPdf> p = pdf.lock();
	if (p && wantDispatch)
		p->canChange ();
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
		assert (indiObj);
		assert (getIndiRef() == indiObj->getIndiRef());

		indiObj->dispatchChange ();
			
	}else
	{
		// Indicate to pdf that it should change this object
		boost::shared_ptr<CPdf> pdf = IProperty::getPdf ().lock ();
		assert (pdf);
		pdf->changeIndirectProperty (indiObj);
	}
}


//=====================================================================================
// Output functions
//=====================================================================================

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


//=====================================================================================
} // namespace pdfobjects
//=====================================================================================
