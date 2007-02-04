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
 * =====================================================================================
 *        Filename:  pdfopertors.cc
 *         Created:  03/24/2006 10:33:34 PM CET
 *          Author:  jmisutka (), 
 * =====================================================================================
 */

// static
#include "static.h"
#include "pdfoperators.h"
//
#include "pdfoperatorsiter.h"
#include "iproperty.h"
#include "cinlineimage.h"

#include "ccontentstream.h"

//==========================================================
namespace pdfobjects {
//==========================================================

using namespace std;
using namespace boost;
using namespace debug;


//==========================================================
// PdfOperator
//==========================================================

//
//
//
shared_ptr<CContentStream>
PdfOperator::getContentStream () const
{
	assert (contentstream);
	assert (contentstream == contentstream->getSmartPointer().get());
	return contentstream->getSmartPointer();
}
	

//==========================================================
// Concrete implementations of PdfOperator
//==========================================================

//
//
//
void 
SimpleGenericOperator::getStringRepresentation (std::string& str) const
{
	std::string tmp;
	for (Operands::const_iterator it = operands.begin(); it != operands.end (); ++it)
	{
		tmp.clear ();
		(*it)->getStringRepresentation (tmp);
		str += tmp + " ";
	}

	// Add operator string
	str += opText;
}
	

//
//
//
shared_ptr<PdfOperator> 
SimpleGenericOperator::clone ()
{
	// Clone operands
	Operands ops;
	for (Operands::iterator it = operands.begin (); it != operands.end(); ++it)
		ops.push_back ((*it)->clone());
	assert (ops.size () == operands.size());

	// Create clone
	return shared_ptr<PdfOperator> (new SimpleGenericOperator (opText,ops));
}


//==========================================================
// CompositePdfOperator
//==========================================================

//
//
//
void 
CompositePdfOperator::push_back (const boost::shared_ptr<PdfOperator> oper, boost::shared_ptr<PdfOperator> prev)
{
	assert (oper);
	kernelPrintDbg (debug::DBG_DBG, "");

	// If children are empty, we have to provide a prev because there is no
	// other way we can obtain shared_ptr to *this
	if (children.empty() && prev)
	{
		children.push_back (oper); 
		PdfOperator::putBehind (prev, oper);
		return;
	
	}else if (children.empty())
	{
		assert (!"Children are empty but prev was not specified.");
		throw CObjInvalidOperation ();
	}
	
	//
	// Change iterator
	//
	if (NULL == prev.get())
	{
		assert (!children.empty());
		assert (!isCompositeOp(children.back()));
		if (isCompositeOp(children.back()))
			throw CObjInvalidOperation ();
		//
		prev = children.back ();
	}
	assert (prev);

	// Insert into iterator list
	PdfOperator::putBehind (prev, oper);
	
	// Add to children
	children.push_back (oper); 
}

//
//
//
void
CompositePdfOperator::remove (boost::shared_ptr<PdfOperator> op)
{ 
	PdfOperators::iterator it =  find (children.begin(), children.end(), op);
	assert (it != children.end());
	if (it == children.end())
		throw CObjInvalidOperation ();
	
	// Erase it
	children.erase (it); 
}

//
//
//
void 
CompositePdfOperator::getChildren (PdfOperators& container) const
{
	container.clear ();
	copy (children.begin(), children.end (), back_inserter (container));
}

//
//
//
void 
CompositePdfOperator::insert_after (const boost::shared_ptr<PdfOperator> oper, 
									boost::shared_ptr<PdfOperator> newOper)
{
	PdfOperators::iterator it = std::find (children.begin(), children.end(), oper);
	children.insert (++it, newOper);
}


//
//
//
void
CompositePdfOperator::getStringRepresentation (std::string& str) const
{
	//
	// Get string representation of every child and append it
	//
	// Indicate that we are a composite
	string tmp;
	PdfOperators::const_iterator it = children.begin ();
	for (; it != children.end(); ++it)
	{
		tmp.clear ();
		(*it)->getStringRepresentation (tmp);
		str += tmp + " ";
	}
}


//==========================================================
// Concrete implementations of CompositePdfOperator
//==========================================================

//
// UnknownCompositePdfOperator
//

//
//
//
UnknownCompositePdfOperator::UnknownCompositePdfOperator 
	(const char* opBegin_, const char* opEnd_) : CompositePdfOperator (), opBegin (opBegin_), opEnd (opEnd_)
{
	utilsPrintDbg (DBG_DBG, "Unknown composite operator: " << opBegin_ << " " << opEnd_);

}

//
//
//
void
UnknownCompositePdfOperator::getStringRepresentation (string& str) const
{
	// Header
	str += opBegin; str += " ";
	
	// Delegate
	CompositePdfOperator::getStringRepresentation (str);	
}

//
//
//
shared_ptr<PdfOperator> 
UnknownCompositePdfOperator::clone ()
{
	shared_ptr<UnknownCompositePdfOperator> clone (new UnknownCompositePdfOperator(opBegin,opEnd));

	for (PdfOperators::iterator it = children.begin(); it != children.end(); ++it)
		clone->push_back ((*it)->clone(),getLastOperator(clone));
	
	// Create clone
	return clone;
}

//
// InlineImageCompositePdfOperator
//

//
//
//
InlineImageCompositePdfOperator::InlineImageCompositePdfOperator 
	(const char* opBegin_, const char* opEnd_, boost::shared_ptr<CInlineImage> im_) 
		: CompositePdfOperator (), opBegin (opBegin_), opEnd (opEnd_), inlineimage (im_)
{
	utilsPrintDbg (DBG_DBG, opBegin_ << " " << opEnd_);
}

//
//
//
void
InlineImageCompositePdfOperator::getStringRepresentation (string& str) const
{
	// Header
	str += opBegin; str += "\n";
	// 
	if (inlineimage)
	{
		std::string tmp;
		inlineimage->getStringRepresentation (tmp);	
		str += tmp;
	}else
	{
		assert (!"Bad inline image.");
		throw CObjInvalidObject ();
	}
	// Footer
	str += opEnd; str += "\n";

}

//
//
//
void
InlineImageCompositePdfOperator::getParameters (Operands& opers) const
{
	boost::shared_ptr<IProperty> ip = inlineimage;
	opers.push_back (ip);
}

//
//
//
shared_ptr<PdfOperator> 
InlineImageCompositePdfOperator::clone ()
{
	// Clone operands
	shared_ptr<CInlineImage> imgclone = IProperty::getSmartCObjectPtr<CInlineImage> (inlineimage->clone());
	// Create clone
	return shared_ptr<PdfOperator> (new InlineImageCompositePdfOperator (opBegin, opEnd, imgclone));
}


//==========================================================
// Helper funcions
//==========================================================

//
//\todo improve performance
//
boost::shared_ptr<CompositePdfOperator>
findCompositeOfPdfOperator (PdfOperator::Iterator it, boost::shared_ptr<PdfOperator> oper)
{
	boost::shared_ptr<CompositePdfOperator> composite;
	typedef PdfOperator::PdfOperators Opers;
	Opers opers;


	while (!it.isEnd())
	{
		// Have we found what we were looking for
		if (isCompositeOp(it))
		{
			opers.clear ();
			it.getCurrent()->getChildren (opers);
			if (opers.end() != std::find (opers.begin(), opers.end(), oper))
				return boost::dynamic_pointer_cast<CompositePdfOperator, PdfOperator> (it.getCurrent());
		}else
		{
			// This can happen only in the "first level" but that should be
			// handled in caller
			if (it.getCurrent() == oper)
			{
				assert (!"Found highest level operator, that should be handled in the caller of this function.");
				throw CObjInvalidObject ();
			}
		}

		it.next ();
	}

	//
	// We should have found the operator
	// -- this can happen in an incorrect script
	// 		that remembers reference to a removed object
	// 
	//assert (!"Operator not found...");
	throw CObjInvalidOperation ();
}

//
//
//
boost::shared_ptr<PdfOperator> getLastOperator (boost::shared_ptr<PdfOperator> oper)
{
	if (!isCompositeOp (oper) || 0 == oper->getChildrenCount())
		return oper;

	PdfOperator::PdfOperators opers;
	oper->getChildren (opers);
	assert (!opers.empty());
	boost::shared_ptr<PdfOperator> tmpop = opers.back();
	while (isCompositeOp (tmpop))
	{
		opers.back()->getChildren (opers);
		if (opers.empty())
			break;
		tmpop = opers.back();
	}

	return tmpop;
}


//
// Our changes
//
namespace {
	/** Change tag. */
	static const char* CHANGE_TAG_NAME = "DP";
	static const char* CHANGE_TAG_ID = "PdfEdit";
}

//
//
//
boost::shared_ptr<PdfOperator> createChangeTag ()
{
	PdfOperator::Operands opers;
	
	// Name or our application
	shared_ptr<CName> name (new CName (CHANGE_TAG_ID));
	opers.push_back (name);
	
	// Dict with our informatio
	shared_ptr<CDict> dict (new CDict ());
	time_t t = time (NULL);
	ostringstream oss;
	oss << t;
	CString tm (oss.str());
	dict->addProperty ("Time", tm);
	opers.push_back (dict);
	
	// Operator
	return shared_ptr<SimpleGenericOperator> (new SimpleGenericOperator (CHANGE_TAG_NAME, opers));
}

//
//
//
string
getChangeTagName ()
	{ return CHANGE_TAG_NAME; }
string
getChangeTagId ()
	{ return CHANGE_TAG_ID; }


//
//
//
time_t
getChangeTagTime (shared_ptr<PdfOperator> op)
{
	assert (op);

	// Check operator name
	assert (isPdfOp (op, CHANGE_TAG_NAME));
	if (!isPdfOp (op,CHANGE_TAG_NAME))
		throw CObjInvalidObject ();

	time_t time = 0;
	if (0 < op->getParametersCount())
	{
		PdfOperator::Operands ops;
		op->getParameters (ops);
		assert (!ops.empty());
		
		try {
			double tmp;
			if (isDict (ops.front()))
				utils::simpleValueFromString (utils::getStringFromDict (ops.front(),"Time"),tmp);
			if (numeric_limits<time_t>::max() > tmp)
				time = static_cast<time_t> (tmp);
			
		}catch (CObjectException&) { 
			utilsPrintDbg (debug::DBG_WARN, "Change tag is CORRUPTED!");
		}
	}
	
	return time;
}

	
//==========================================================
} // namespace pdfobjects
//==========================================================
