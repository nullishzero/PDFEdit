/*                                                                              
 * PDFedit - free program for PDF document manipulation.                        
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko, 
 *                                              Miroslav Jahoda,       
 *                                              Jozef Misutka, 
 *                                              Martin Petricek                                             
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit                                                                      
 */ 
// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80
/*
 * $RCSfile$
 *
 * $Log$
 * Revision 1.30  2007/04/25 18:24:35  bilboq
 *
 * applied patch from Martin Michlmayr to fix compilation with gcc-4.3
 *
 * fixed some typos in comments
 *
 * Revision 1.29  2007/02/04 20:17:02  mstsxfx
 * Common Licence comment for all cc and h files available in doc/licence_header
 * file and its content to all cc and h files in src/{gui,kernel,utils}
 * directories.
 * Xpdf code, QSA and QOutputDevice code are not touched
 *
 * Revision 1.28  2006/09/04 18:25:27  petrm1am
 *
 * 2 typos in documentation (descriPtion !) + commented out "class COutline {}"
 * (not used anywhere and it causes empty doxygen page to be generated)
 *
 * Revision 1.27  2006/06/27 17:10:15  hockm0bm
 * gcc warnings removed (parameter shadows class member)
 *
 * Revision 1.26  2006/06/25 16:25:09  hockm0bm
 * doc update - doxygen warnings removed (if they are real problem)
 *
 * Revision 1.25  2006/06/24 09:59:04  hockm0bm
 * programming errors corrected
 *         - local variable returned - changed to class field
 *
 * Revision 1.24  2006/06/21 18:46:47  hockm0bm
 * typo fix
 *
 * Revision 1.23  2006/06/16 18:23:25  hockm0bm
 * NoPageRootException added
 *
 * Revision 1.22  2006/05/30 14:59:28  misuj1am
 *
 * -- iterator change, most(NOT ALL) exceptions handled during stream parsing
 * -- CStream::getStringRepre returns pdf representation of cstream object
 *
 * Revision 1.21  2006/05/27 21:05:45  misuj1am
 *
 * -- BiG UPGRADE:
 * 	-- many pdfs could not be parsed correctly because their ccs were located in more streams and
 * 	the split point was really insane (after an operand etc..) so CContentStream was modified to allow
 * 	more cstreams.. BUT we kept the advantage of more ccs in one page when a cstream makes a full valid
 * 	content stream
 * 		-- First version of CContentStream was doing the same BUT without the advantage
 * 	-- CContentStream was made a friend of PdfOper and because of this several operations could be made private..
 * 	this is due to the fact that GUI has used "forbidden" methods like PDfOper::remove...
 *
 * Revision 1.20  2006/05/16 17:50:58  hockm0bm
 * minor changes
 *
 * Revision 1.19  2006/05/08 10:31:29  hockm0bm
 * quick fix
 *         - copy & paste error
 *
 * Revision 1.18  2006/05/08 10:27:55  hockm0bm
 * IndirectObjectsExhausted exception added
 *
 * Revision 1.17  2006/05/07 10:03:12  misuj1am
 *
 * -- ADD: filter exception
 * -- filter handling improved
 *
 * Revision 1.16  2006/04/20 18:05:36  hockm0bm
 * AmbiguousPageTreeException spelling corrected
 *
 * Revision 1.15  2006/04/19 18:41:29  hockm0bm
 * AmbiguesPageTreeException added
 *
 * 
 */

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
	 */
	PageNotFoundException():message("Page not found")
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
