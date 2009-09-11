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

#ifndef _FACTORIES_H_
#define _FACTORIES_H_

#include "kernel/cobject.h"

/** 
 * @file factories.h
 *
 * This file collects facotry classes for cobjects. Each specialized cobject 
 * has its own factory class which has static factory methods for all available 
 * methods for cobject creation. 
 * <br>
 * Factories should be used allways rather than new operator, because they
 * implements instancing policy.
 */

//===========================================
namespace pdfobjects {
//===========================================

	
/** Generic factory class for cobjects.
 */
template<typename Type>
class CObjectFactory
{
public:
	/** 
	 * Creates default Type instance.
	 *
	 * @return Type instance with default (empty dictionary) value.
	 */
	static Type* 
	getInstance()
	{
		return new Type ();
	}

	/** Creates Type with given value.
	 * @param value Value to use to initialize.
	 *
	 * @return Type instance with given value.
	 */
	static Type* getInstance(const typename Type::Value& value)
	{
		return new Type (value);
	}

	/** 
	 * Creates Type for specific pdf from xpdf Object.
	 * @param pdf Pdf for which to create CDict.
	 * @param indirefParent Indirect reference to nearest indirect parent.
	 * @param obj Xpdf object to use for intialization.
	 *
	 * This should be used only internaly by kernel. Instance initialized this
	 * way is not checked in cobjects routines and so nonsense information may
	 * lead to mass. 
	 * <br>
	 * <b>REMARK</b>: <br>
	 * If you don't know what exactly these parameters mean, DON'T use this
	 * method.
	 * @return Type instance.
	 */
	static Type* 
	getInstance(boost::shared_ptr<CPdf> pdf, const IndiRef & indirefParent, const Object & obj)
	{
		return new Type(pdf, obj, indirefParent);
	}

	/** 
	 * Creates Type from xpdf Object.
	 * @param obj Xpdf Object instance (must by objDict).
	 *
	 * @throw ElementBadTypeException If given object doesn't represent
	 * dictionary object.
	 *
	 * @return Type instance.
	 */
	static Type* 
	getInstance(const Object & obj)
	{
		return new Type (obj);
	}
};

/** Factory class for xpdf Objects.
 *
 * Creates instance of xpdf Object class. Instances has to be deallocated by
 * gfree method or pdfobjects::xpdf::freeXpdfObject.
 */
class XPdfObjectFactory
{
public:
	/** Creates Object instance with objNull type.
	 *
	 * @return Object instance with objNull type.
	 */
	static Object * getInstance()
	{
		Object * instance=(Object *)gmalloc(sizeof(Object));
		instance->initNull();

		return instance;
	}
};

/** Factory class for CInt creation.
 *
 * Use getInstance methods for instants creation.
 */
typedef CObjectFactory<CInt> CIntFactory;

/** Factory class for CReal creation.
 *
 * Use getInstance methods for instants creation.
 */
typedef CObjectFactory<CReal> CRealFactory;

/** Factory class for CString creation.
 *
 * Use getInstance methods for instants creation.
 */
typedef CObjectFactory<CString> CStringFactory;

/** Factory class for CName creation.
 *
 * Use getInstance methods for instants creation.
 */
typedef CObjectFactory<CName> CNameFactory;

/** Factory class for CBool creation.
 *
 * Use getInstance methods for instants creation.
 */
typedef CObjectFactory<CBool> CBoolFactory;

/** Factory class for CRef creation.
 *
 * Use getInstance methods for instants creation.
 */
typedef CObjectFactory<CRef> CRefFactory;



/** Factory class for CNull creation.
 *
 * Use getInstance method for instants creation.
 */
class CNullFactory
{
public:
	/** Creates CNull instance.
	 *
	 * @return CNull instance.
	 */
	static CNull * getInstance()
	{
		return new CNull ();
	}

};

/** Factory class for CDict creation.
 *
 * Use getInstance methods for instants creation.
 */
class CDictFactory
{
public:
	/** Creates default CDict instance.
	 *
	 * @return CDict instance with default (empty dictionary) value.
	 */
	static CDict * getInstance()
	{
		return new CDict();
	}

	/** Creates CDict for specific pdf from xpdf Object.
	 * @param pdf Pdf for which to create CDict.
	 * @param indirefParent Indirect reference to nearest indirect parent.
	 * @param obj Xpdf object to use for intialization.
	 *
	 * This should be used only internaly by kernel. Instance initialized this
	 * way is not checked in cobjects routines and so nonsense information may
	 * lead to mass. 
	 * <br>
	 * <b>REMARK</b>: <br>
	 * If you don't know what exactly these parameters mean, DON'T use this
	 * method.
	 * @return CDict instance.
	 */
	static CDict * getInstance(boost::shared_ptr<CPdf>  pdf, const IndiRef & indirefParent, const Object & obj)
	{
		return new CDict(pdf, obj, indirefParent);
	}

	/** Creates CDict from xpdf Object.
	 * @param obj Xpdf Object instance (must by objDict).
	 *
	 * @throw ElementBadTypeException If given object doesn't represent
	 * dictionary object.
	 *
	 * @return CDict instance.
	 */
	static CDict * getInstance(const Object & obj)
	{
		// checks type
		if(obj.getType()!=objDict)
			throw ElementBadTypeException("CDict");

		return new CDict(obj);
	}
};

/** Factory class for CArray creation.
 *
 * Use getInstance methods for instants creation.
 */
class CArrayFactory
{
public:
	/** Creates default CArray instance.
	 *
	 * @return CArray instance with default (empty dictionary) value.
	 */
	static CArray * getInstance()
	{
		return new CArray();
	}

	/** Creates CArray for specific pdf from xpdf Object.
	 * @param pdf Pdf for which to create CArray.
	 * @param indirefParent Indirect reference to nearest indirect parent.
	 * @param obj Xpdf object to use for intialization.
	 *
	 * This should be used only internaly by kernel. Instance initialized this
	 * way is not checked in cobjects routines and so nonsense information may
	 * lead to mass. 
	 * <br>
	 * <b>REMARK</b>: <br>
	 * If you don't know what exactly these parameters mean, DON'T use this
	 * method.
	 * @return CArray instance.
	 */
	static CArray * getInstance(boost::shared_ptr<CPdf>  pdf, const IndiRef & indirefParent, const Object & obj)
	{
		return new CArray(pdf, obj, indirefParent);
	}

	/** Creates CArray from xpdf Object.
	 * @param obj Xpdf Object instance (must by objArray).
	 *
	 * @throw ElementBadTypeException If given object doesn't represent
	 * dictionary object.
	 *
	 * @return CArray instance.
	 */
	static CArray * getInstance(const Object & obj)
	{
		// checks type
		if(obj.getType()!=objArray)
			throw ElementBadTypeException("CArray");

		return new CArray(obj);
	}
};

/******************************************************************************
 * High level Cobjects factories
 *****************************************************************************/

//
// Forward declarations
// 
class CPage;

/** CPageFactory factory class.
 */
class CPageFactory
{
public:
	/** Creates CPage instance form page dictionary.
	 * @param pageDict Page dictionary (must be CDict and Type field must me
	 * /Page).
	 *
	 * @return CPage instance.
	 */
	static CPage * getInstance(boost::shared_ptr<CDict> pageDict);
};

 
//===========================================
} // namepsace pdfobjects
//===========================================
#endif
