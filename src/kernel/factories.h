// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80

#ifndef _FACTORIES_H_
#define _FACTORIES_H_

#include "cobjectI.h"

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

namespace pdfobjects
{

/** Factory class for CInt creation.
 *
 * Use getInstance methods for instants creation.
 */
class CIntFactory
{
public:
	/** Creates default CInt instance.
	 *
	 * @return CInt instance with default value.
	 */
	static CInt * getInstance()
	{
		return new CInt();
	}

	/** Creates CInt with given value.
	 * @param value Value to use to initialize.
	 *
	 * @return CInt instance with given value.
	 */
	static CInt * getInstance(int value)
	{
		return new CInt(value);
	}

	/** Creates CInt for specific pdf from xpdf Object.
	 * @param pdf Pdf for which to create CInt.
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
	 * @return CInt instance.
	 */
	static CInt * getInstance(CPdf & pdf, IndiRef & indirefParent, Object & obj)
	{
		return new CInt(pdf, obj, indirefParent);
	}
};

/** Factory class for CReal creation.
 *
 * Use getInstance methods for instants creation.
 */
class CRealFactory
{
public:
	/** Creates default CReal instance.
	 *
	 * @return CReal instance with default value.
	 */
	static CReal * getInstance()
	{
		return new CReal();
	}

	/** Creates CReal with given value.
	 * @param value Value to use to initialize.
	 *
	 * @return CReal instance with given value.
	 */
	static CReal * getInstance(int value)
	{
		return new CReal(value);
	}

	/** Creates CReal for specific pdf from xpdf Object.
	 * @param pdf Pdf for which to create CReal.
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
	 * @return CReal instance.
	 */
	static CReal * getInstance(CPdf & pdf, IndiRef & indirefParent, Object & obj)
	{
		return new CReal(pdf, obj, indirefParent);
	}
};

/** Factory class for CString creation.
 *
 * Use getInstance methods for instants creation.
 */
class CStringFactory
{
public:
	/** Creates default CString instance.
	 *
	 * @return CString instance with default value.
	 */
	static CString * getInstance()
	{
		return new CString();
	}

	/** Creates CString with given value.
	 * @param value Value to use to initialize.
	 *
	 * @return CString instance with given value.
	 */
	static CString * getInstance(std::string value)
	{
		return new CString(value);
	}

	/** Creates CString for specific pdf from xpdf Object.
	 * @param pdf Pdf for which to create CString.
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
	 * @return CString instance.
	 */
	static CString * getInstance(CPdf & pdf, IndiRef & indirefParent, Object & obj)
	{
		return new CString(pdf, obj, indirefParent);
	}
};

/** Factory class for CName creation.
 *
 * Use getInstance methods for instants creation.
 */
class CNameFactory
{
public:
	/** Creates default CName instance.
	 *
	 * @return CName instance with default value.
	 */
	static CName * getInstance()
	{
		return new CName();
	}

	/** Creates CName with given value.
	 * @param value Value to use to initialize.
	 *
	 * @return CName instance with given value.
	 */
	static CName * getInstance(std::string value)
	{
		return new CName(value);
	}

	/** Creates CName for specific pdf from xpdf Object.
	 * @param pdf Pdf for which to create CName.
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
	 * @return CName instance.
	 */
	static CName * getInstance(CPdf & pdf, IndiRef & indirefParent, Object & obj)
	{
		return new CName(pdf, obj, indirefParent);
	}
};

/** Factory class for CBool creation.
 *
 * Use getInstance methods for instants creation.
 */
class CBoolFactory
{
public:
	/** Creates default CBool instance.
	 *
	 * @return CBool instance with default value.
	 */
	static CBool * getInstance()
	{
		return new CBool();
	}

	/** Creates CBool with given value.
	 * @param value Value to use to initialize.
	 *
	 * @return CBool instance with given value.
	 */
	static CBool * getInstance(int value)
	{
		return new CBool(value);
	}

	/** Creates CBool for specific pdf from xpdf Object.
	 * @param pdf Pdf for which to create CBool.
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
	 * @return CBool instance.
	 */
	static CBool * getInstance(CPdf & pdf, IndiRef & indirefParent, Object & obj)
	{
		return new CBool(pdf, obj, indirefParent);
	}
};

/** Factory class for CRef creation.
 *
 * Use getInstance methods for instants creation.
 */
class CRefFactory
{
public:
	/** Creates default CRef instance.
	 *
	 * @return CRef instance with default value.
	 */
	static CRef * getInstance()
	{
		return new CRef();
	}

	/** Creates CRef with given value.
	 * @param value Value to use to initialize.
	 *
	 * @return CRef instance with given value.
	 */
	static CRef * getInstance(IndiRef value)
	{
		return new CRef(value);
	}

	/** Creates CRef for specific pdf from xpdf Object.
	 * @param pdf Pdf for which to create CRef.
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
	 * @return CRef instance.
	 */
	static CRef * getInstance(CPdf & pdf, IndiRef & indirefParent, Object & obj)
	{
		return new CRef(pdf, obj, indirefParent);
	}
};

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
		return new CNull();
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
	static CDict * getInstance(CPdf & pdf, IndiRef & indirefParent, Object & obj)
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
	static CDict * getInstance(Object & obj)
	{
		// checks type
		if(obj.getType()!=objDict)
			throw ElementBadTypeException("CDict");

		return new CDict(obj);
	}
};

}
#endif
