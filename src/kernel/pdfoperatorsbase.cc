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

// static
#include "kernel/static.h"
#include "kernel/pdfoperatorsbase.h"
//
#include "kernel/pdfoperatorsiter.h"
#include "kernel/iproperty.h"

#include "kernel/ccontentstream.h"

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
boost::shared_ptr<CContentStream>
PdfOperator::getContentStream () const
{
		assert (_contentstream);
		assert (_contentstream == _contentstream->getSmartPointer().get());
	return _contentstream->getSmartPointer();
}
	
void 
PdfOperator::putBehind (boost::shared_ptr<PdfOperator> behindWhich, boost::shared_ptr<PdfOperator> which)
{
	if (behindWhich && which)
	{
		if (!behindWhich->_next().expired())
		{ // we are not at the end
		
				assert (behindWhich->_next().lock());
			which->setNext (behindWhich->_next());
			behindWhich->_next().lock()->setPrev (ListItem (which));
			which->setPrev (ListItem (behindWhich));
			behindWhich->setNext (ListItem (which));
			
		}else
		{ // we are at the end
			
			behindWhich->setNext (ListItem (which));
			which->setPrev (ListItem (behindWhich));
		}
	
	}else
		throw CObjInvalidOperation ();
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
	if (_children.empty() && prev)
	{
		_children.push_back (oper); 
		PdfOperator::putBehind (prev, oper);
		return;
	
	}else if (_children.empty())
	{
		assert (!"Children are empty but prev was not specified.");
		throw CObjInvalidOperation ();
	}
	
	//
	// Change iterator
	//
	if (!(prev))
	{
			assert (!_children.empty());
			assert (!isCompositeOp(_children.back()));
			if (isCompositeOp(_children.back()))
				throw CObjInvalidOperation ();
		//
		prev = _children.back ();
	}
		assert (prev);

	// Insert into iterator list
	PdfOperator::putBehind (prev, oper);
	
	// Add to children
	_children.push_back (oper); 
}

//
//
//
void
CompositePdfOperator::remove (boost::shared_ptr<PdfOperator> op)
{ 
	PdfOperators::iterator it =  find (_children.begin(), _children.end(), op);
		assert (it != _children.end());
		if (it == _children.end())
			throw CObjInvalidOperation ();
	// Erase it
	_children.erase (it); 
}

//
//
//
void 
CompositePdfOperator::getChildren (PdfOperators& container) const
{
	container.clear ();
	copy (_children.begin(), _children.end (), back_inserter (container));
}

//
//
//
void 
CompositePdfOperator::insert_after (const boost::shared_ptr<PdfOperator> oper, 
									boost::shared_ptr<PdfOperator> newOper)
{
	PdfOperators::iterator it = std::find (_children.begin(), _children.end(), oper);
	_children.insert (++it, newOper);
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
	PdfOperators::const_iterator it = _children.begin ();
	for (; it != _children.end(); ++it)
	{
		tmp.clear ();
		(*it)->getStringRepresentation (tmp);
		str += tmp + " ";
	}
}

void 
CompositePdfOperator::init_operands (boost::shared_ptr<observer::IObserver<IProperty> > observer, boost::weak_ptr<CPdf> pdf, IndiRef* rf)
{ 
	for (PdfOperators::iterator it = _children.begin (); it != _children.end(); ++it)
		(*it)->init_operands (observer, pdf, rf);
}


//==========================================================
// Helper functions
//==========================================================


//
//
//
boost::shared_ptr<PdfOperator> 
getLastOperator (boost::shared_ptr<PdfOperator> oper)
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


//==========================================================
} // namespace pdfobjects
//==========================================================
