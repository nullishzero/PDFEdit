// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80
/*
 * =====================================================================================
 *        Filename:  exceptions.h
 *     Description:  Exceptions.
 *         Created:  01/29/2006 08:06:57 PM CET
 *          Author:  jmisutka ()
 * =====================================================================================
 */

#ifndef _EXCEPTIONS_H
#define _EXCEPTIONS_H

// stl
#include <exception>


/**
 * @file exceptions.h
 *
 * Exceptions describtion.
 *
 * Kernel is using different types of exceptions for unexpected situation 
 * handling.
 * To keep system in them, they are separated to the following cathegories:
 * <ul>
 * <li> XpdfException - exceptions related to problems with xpdf code usage. 
 * <li> CObjectException - exceptions related with cobjects.
 * <li> PdfException - exceptions related with pdf corruptio, specification, pdf
 * usage problem (such as changing read-only document).
 * <li> ImplementationException - exceptions related with implementation
 * specific stuff.
 * </ul> 
 *
 * All these cathegories are base classes for specific exceptions. They are
 * std::exception subclasses.
 * <br>
 * TODO
 * Comparing to std::exception they add cause field which stands for low-level
 * exception which has caused throwing of this exception.
 */

/** XPDF exception base class.
 */
class XpdfException : public std::exception
{
};

/** Cobject exception base class.
 */
class CObjectException: public std::exception
{
};

/** Exception thrown when element can't be found in complex type.
 */
class ElementNotFoundException: public CObjectException
{
private:
	/** Complex typed name/identifier.
	 */
	std::string parent;

	/** Child which wasn't found in dictionary.
	 */
	std::string child;
public:
	/** Exception constructor.
	 * @param _parent Complex type.
	 * @param _child Element which hasn't been found.
	 *
	 * If name of the complex is not available in context, at least its type
	 * should be provided. _child stands for identifier which was searched.
	 */
	ElementNotFoundException(std::string _parent, std::string _child):parent(_parent), child(_child)
	{
	}

	virtual ~ElementNotFoundException() throw()
	{
	}

	/** Prints context information of exception.
	 *
	 * @return c string containing context message.
	 */
	const char * what()const throw()
	{
		std::string msg;
		msg=child + " not found in " + parent;
		return msg.c_str();
	}

	/** Gets parent value.
	 * @param parent Place where to store parent string.
	 */
	void getParent(std::string & parent)const
	{
		parent=this->parent;
	}
	
	/** Gets child value.
	 * @param child Place where to store child string.
	 */
	void getChild(std::string & child)const
	{
		child=this->child;
	}
};

/** Exception thrown when cobject has or whould have bad type.
 * TODO howto store bad type information
 */
class ElementBadTypeException: public CObjectException
{
	/** Element identifier.
	 */
	std::string element;

public:
	/** Exception constructor.
	 * @param _element Element id with bad type.
	 */
	ElementBadTypeException(std::string _element):element(_element)
	{
	}

	virtual ~ElementBadTypeException() throw()
	{
	}

	/** Prints context information of exception.
	 *
	 * @return c string containing context message.
	 */
	const char * what()const throw()
	{
		std::string str=element + "has bad type"; 
		return str.c_str();
	}
};

/** Pdf error exception base class.
 */
class PdfException: public std::exception
{
};

/** Exception is thrown when pdf structure doesn't confirm PDF specification
 * requiered for operation.
 *
 * TODO include also cause exception
 */
class MalformedFormatExeption: public PdfException
{
	std::string message;
public:
	MalformedFormatExeption(std::string msg):message(msg)
	{
	}

	virtual ~MalformedFormatExeption() throw()
	{
	}

	virtual const char * what()const throw()
	{
		return message.c_str();
	}

	/** Gets message from exception.
	 * @param msg String where to set message.
	 */
	void getMessage(std::string &msg)
	{
		msg=message;
	}
};

/** Exception is thrown when no page at postion can be found.
 */
class PageNotFoundException: public PdfException
{
	size_t position;
public:
	/** Exception constructor.
	 * @param pos Postion which couldn't be found.
	 */
	PageNotFoundException(size_t pos):position(pos)
	{
	}

	virtual ~PageNotFoundException() throw()
	{
	}

	virtual const char * what()const throw()
	{
		std::string msg="Page not found at pos "+position;

		return msg.c_str();
	}

	void getPosition(size_t & pos)
	{
		pos=position;
	}
};

// requiered element missing
// read-only mode


/** Implementation exception base class.
 */
class ImplementationException: public std::exception
{
};


// TODO these exceptions should be changed to inherit from XpdfException base
// exception class
/**
 * Exceptions occurs when (x)pdf object has incorrect type.
 */
struct ObjBadTypeE : std::exception
{
	char const* what() const throw() {return "(x)pdf object has bad type.";}
};

struct ObjBadValueE : std::exception
{
	char const* what() const throw() {return "(x)pdf object has bad value.";}
};

struct ObjInvalidObject : std::exception
{
	char const* what() const throw() {return "(x)pdf object is invalid.";}
};

struct ObjInvalidPositionInComplex : std::exception
{
	char const* what() const throw() {return "(x)pdf object position is invalid.";}
};

struct ObjInvalidOperation : std::exception
{
	char const* what() const throw() {return "Invalid operation.";}
};





#endif // _EXCEPTIONS_H
