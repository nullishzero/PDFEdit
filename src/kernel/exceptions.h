// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80
/*
 * $RCSfile$
 *
 * $Log$
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

class NotImplementedException;
class IndirectObjectNotFoundException;
class ElementNotFoundException;
class ElementBadTypeException;

struct XpdfInvalidObject;

struct CObjBadValue;
struct CObjInvalidObject;
struct CObjInvalidOperation;
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
	PdfOpenException(std::string msg):message(msg){};

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
	PdfOpenException(){};

	virtual ~PdfOpenException()throw()
	{
	}

	virtual const char * what()const throw()
	{
		return "No more indirect reference available";
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
	const size_t position;
	
public:
	/** Exception constructor without position specified.
	 */
	PageNotFoundException():position(0)
	{
	}
	
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

/** Exception is thrown when page tree is ambiguous.
 *
 * This means that it is not possible to get Node position in its parent Kids
 * array (because it is in this Kids array several times).
 *
 */
class AmbiguousPageTreeException: public PdfException
{
	// TODO keep intermediate node with such ambigues Kids array
public:
	/** Exception constructor.
	 */
	AmbiguousPageTreeException()
	{
	}

	virtual ~AmbiguousPageTreeException() throw()
	{
	}

	virtual const char * what()const throw()
	{
		std::string msg="PageTree is ambigues";

		// TODO add information about inter node

		return msg.c_str();
	}
};

/** Exception is thrown when change on read only document is about to be done.
 */
class ReadOnlyDocumentException: public PdfException
{
	std::string message;
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
	std::string feature;
public:
	/** Exception constructor.
	 * @param _feature Feature description which is not implemented.
	 */
	NotImplementedException(std::string _feature):feature(_feature)
	{
	}

	~NotImplementedException() throw()
	{
	}

	const char * what()const throw()
	{
		std::string msg="feature=\""+feature+"\" is not implemented";

		return msg.c_str();
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

class IndirectObjectNotFoundException: public CObjectException
{
	int num;
	int gen;

public:
	/** Exception constructor.
	 * @param _num Object number.
	 * @param _gen Generation number.
	 */
	IndirectObjectNotFoundException(int _num, int _gen):num(_num),gen(_gen)
	{
	}

	~IndirectObjectNotFoundException()throw()
	{
	}

	const char * what()const throw()
	{
		// FIXME
		//std::string msg="Indirect object with ref=[";
		//msg+=num+", "+gen+"] not found";

		return "FIXME";//msg.c_str();
	}

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


/** Exception thrown when cobject has or would have bad type.
 * TODO howto store bad type information
 */
class ElementBadTypeException: public CObjectException
{
	/** Element identifier.
	 */
	const std::string element;

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
