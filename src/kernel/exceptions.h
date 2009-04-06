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

#ifndef _EXCEPTIONS_H
#define _EXCEPTIONS_H

#include<exception>
#include<string>
#include<sstream>

/**
 * @file exceptions.h
 *
 * Exceptions description.
 *
 * Kernel is using different types of exceptions for unexpected situation 
 * handling.
 * To keep system in them, they are separated to the following cathegories:
 * <ul>
 * <li> XpdfException - exceptions related to problems with xpdf code usage. 
 * <li> CObjectException - exceptions related with cobjects.
 * <li> PdfException - exceptions related with pdf corruption, specification, pdf
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

//
// Forward declarations to make it readable
//
class PdfOpenException;
class MalformedFormatExeption;
class PageNotFoundException;
class ReadOnlyDocumentException;
class NoPageRootException;

class NotImplementedException;
class IndirectObjectNotFoundException;
class ElementNotFoundException;
class ElementBadTypeException;

class PermissionException;

struct CObjBadValue;
struct CObjInvalidObject;
struct CObjInvalidOperation;
struct MalformedContentStreamException;

struct XpdfInvalidObject;

struct OutOfRange;

struct FilterNotSupported;

/** XPDF exception base class.
 */
class XpdfException : public std::exception
{
};


/** Pdf error exception base class.
 */
class PdfException: public std::exception
{
};

/** General purpose exception for pdfedit-dev related
 * exceptions.
 * Thrown for example when pdfedit-dev core is not initialized
 * and its code is used.
 */
class PDFedit_devException: public std::exception
{
};

/** Cobject exception base class.
 */
class CObjectException: public std::exception
{
};

/** Pdf open exception.
 *
 * Exception is thrown if file open fail on what ever cause. Reason is in
 * message string.
 */
class PdfOpenException: public PdfException
{
	const std::string message;
public:
	/** Exception constructor.
	 * @param msg Cause message.
	 */
	PdfOpenException(std::string msg):message(msg){}

	virtual ~PdfOpenException()throw()
	{
	}

	virtual const char * what()const throw()
	{
		return message.c_str();
	}
	
	void getMessage(std::string & msg)const
	{
		msg=message;
	}
};

/** No more new indirect object available exception.
 * This exception is thrown if there is no more free indirect object available.
 */
class IndirectObjectsExhausted: public PdfException
{
public:
	/** Exception constructor.
	 */
	IndirectObjectsExhausted(){}

	virtual ~IndirectObjectsExhausted()throw()
	{
	}

	virtual const char * what()const throw()
	{
		return "No more indirect reference available";
	}
};

/** No page tree root dictionary exception.
 *
 * Exception is thrown if someone is tries to insert new page to document which
 * doesn't contain any page tree root dictionary.
 */
class NoPageRootException: public PdfException
{
public:
	/** Exception constructor.
	 */
	NoPageRootException(){}

	virtual ~NoPageRootException()throw()
	{
	}

	virtual const char * what()const throw()
	{
		return "No page tree root found";
	}
};

/** Exception is thrown when pdf structure doesn't confirm to PDF specification
 * required for operation.
 *
 * TODO include also cause exception
 */
class MalformedFormatExeption: public PdfException
{
	const std::string message;
	
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
	std::string message;
	
public:
	/** Exception constructor without position specified.
	 * This exception should be called when page is searched by 
	 * CPage instance.
	 */
	PageNotFoundException():position(0),message("Page not found")
	{
	}
	
	/** Exception constructor.
	 * @param pos Postion which couldn't be found.
	 */
	PageNotFoundException(size_t pos):position(pos)
	{
		std::ostringstream str;
		str<<"Page at "<<pos<<" not found";	
		message=str.str();
	}

	virtual ~PageNotFoundException() throw()
	{
	}

	virtual const char * what()const throw()
	{
		return message.c_str();
	}

	void getPosition(size_t & pos)
	{
		pos=position;
	}
};

/** Exception is thrown when page tree is ambiguous.
 *
 * This means that it is not possible to get Node position in its parent Kids
 * array (because it is in this Kids array several times).
 *
 */
class AmbiguousPageTreeException: public PdfException
{
	// TODO keep intermediate node with such ambigues Kids array
	const std::string msg;
public:
	/** Exception constructor.
	 */
	AmbiguousPageTreeException():msg("Page tree is ambiguous.")
	{
	}

	virtual ~AmbiguousPageTreeException() throw()
	{
	}

	virtual const char * what()const throw()
	{
		return msg.c_str();
	}
};

/** Exception is thrown when change on read only document is about to be done.
 */
class ReadOnlyDocumentException: public PdfException
{
	const std::string message;
public:
	/** Exception constructor.
	 * @param msg Context message of action.
	 */
	ReadOnlyDocumentException(std::string msg):message(msg)
	{
	}

	virtual ~ReadOnlyDocumentException() throw()
	{
	}

	virtual const char * what()const throw()
	{
		return message.c_str();
	}

	void getMessage(std::string & msg)
	{
		msg=message;
	}
};

// requiered element missing
// read-only mode


/** Implementation exception base class.
 */
class ImplementationException: public std::exception
{
};

/** Functionality is not implemented exception.
 */
class NotImplementedException: public ImplementationException
{
	const std::string feature;
	const std::string message;
public:
	/** Exception constructor.
	 * @param _feature Feature description which is not implemented.
	 */
	NotImplementedException(std::string _feature):feature(_feature), message("feature \""+feature+"\" is not implemented")
	{
	}

	~NotImplementedException() throw()
	{
	}

	const char * what()const throw()
	{
		return message.c_str();
	}

	void getFeature(std::string _feature)
	{
		_feature=feature;
	}
};

//==================================
// Concrete XpdfExceptions
//==================================

/**
 * Exception occurs when xpdf object is in invalid state.
 */
struct XpdfInvalidObject : public XpdfException
{
	char const* what() const throw() {return "Invalid Xpdf object.";}
};




//==================================
// Concrete CObjectExceptions
//==================================

/** Exception thrown when indirect object can't be found.
 */
class IndirectObjectNotFoundException: public CObjectException
{
	/** Object number.
	 */
	const int num;

	/** Generation number.
	 */
	const int gen;

	std::string message;
public:
	/** Exception constructor.
	 * @param _num Object number.
	 * @param _gen Generation number.
	 */
	IndirectObjectNotFoundException(int _num, int _gen):num(_num),gen(_gen)
	{
		std::ostringstream oss;
		oss<<"Indirect object with ref=["<<num<<", "<<gen<<"] not found";
		message=oss.str();
	}

	~IndirectObjectNotFoundException()throw()
	{
	}

	const char * what()const throw()
	{
		return message.c_str();
	}

	/** Returns object identifier.
	 * @param _num Object number value holder.
	 * @param _gen Generation number value holder.
	 *
	 * Sets current num and gen values to given parameters.
	 */
	void getReference(int & _num, int & _gen)
	{
		_num=num;
		_gen=gen;
	}
};

/** Exception thrown when element can't be found in complex type.
 */
class ElementNotFoundException: public CObjectException
{
private:
	/** Complex typed name/identifier.
	 */
	const std::string parent;

	/** Child which wasn't found in dictionary.
	 */
	const std::string child;
	
	std::string message;
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
		message=child+" not found in "+parent;
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
		return message.c_str();
	}

	/** Gets parent value.
	 * @param par Place where to store parent string.
	 */
	void getParent(std::string & par)const
	{
		par=this->parent;
	}
	
	/** Gets child value.
	 * @param ch Place where to store child string.
	 */
	void getChild(std::string & ch)const
	{
		ch=this->child;
	}
};


/** Exception thrown when cobject has or would have bad type.
 * TODO howto store bad type information
 */
class ElementBadTypeException: public CObjectException
{
	/** Element identifier.
	 */
	const std::string element;

	std::string message;
public:
	/** Exception constructor.
	 * @param _element Element id with bad type.
	 */
	ElementBadTypeException(std::string _element):element(_element)
	{
		message=element+" has bad type";
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
		return message.c_str();
	}
};

/** Exception for permission violation.
 */
class PermissionException: public CObjectException
{
	const std::string message;
public:
	/** Exception constructor.
	 * @param _message Feature description which is not implemented.
	 */
	PermissionException(std::string _message):message(_message)
	{
	}

	~PermissionException() throw()
	{
	}

	const char * what()const throw()
	{
		return message.c_str();
	}

	void getMessage(std::string _message)
	{
		_message=message;
	}
};

/**
 * Exception occurs when CObject has got unexpected value. 
 */
struct CObjBadValue : public CObjectException
{
	char const* what() const throw() {return "CObject has got bad value.";}
};

/**
 * Exception occurs when object is in an invalid state or when an object
 * is NULL but should not be.
 */
struct CObjInvalidObject : public CObjectException
{
	char const* what() const throw() {return "CObject is in bad state.";}
};

/**
 * Exception occurs when we want to perform an operation but the object is not
 * in required state.
 */
struct CObjInvalidOperation : public CObjectException
{
	char const* what() const throw() {return "Invalid operation on CObject.";}
};

/**
 * Exception occurs when we do an invalid cast of IProperty.
 */
struct CObjInvalidCast : public CObjectException
{
	char const* what() const throw() {return "Invalid cast of IProperty.";}
};

/**
 * Exception occurs when we do an invalid cast of IProperty.
 */
struct MalformedContentStreamException : public CObjectException
{
	char const* what() const throw() {return "Invalid content stream.";}
};






//==================================
// Filter expcetions
//==================================

struct FilterNotSupported : public std::exception
{
	const char* what() const throw() {return "One of filters is not supported.";}
};


//==================================
// Standard expcetions
//==================================

struct OutOfRange : public std::exception
{
	char const* what() const throw() {return "Out of range.";}
};

#endif // _EXCEPTIONS_H
