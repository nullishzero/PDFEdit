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
#include "kernel/cobject.h"
#include "kernel/cobjecthelpers.h"
#include "kernel/contentschangetag.h"
#include "kernel/pdfoperators.h"

//==========================================================
namespace pdfobjects {
//==========================================================

using namespace std;
using namespace boost;


// Initialization
const char* ContentsChangeTag::CHANGE_TAG_NAME = "DP";
const char* ContentsChangeTag::CHANGE_TAG_ID = "PdfEdit";

//
//
//
boost::shared_ptr<PdfOperator> 
ContentsChangeTag::create ()
{
	PdfOperator::Operands opers;
	
	// Name or our application
	boost::shared_ptr<CName> name (new CName (CHANGE_TAG_ID));
	opers.push_back (name);
	
	// Dict with our informatio
	boost::shared_ptr<CDict> dict (new CDict ());
	time_t t = time (NULL);
	ostringstream oss;
	oss << t;
	CString tm (oss.str());
	dict->addProperty ("Time", tm);
	opers.push_back (dict);
	
	// Operator
	return createOperator (CHANGE_TAG_NAME, opers);
}


//
//
//
time_t
ContentsChangeTag::getTime (boost::shared_ptr<PdfOperator> op)
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
			if (isDict (ops.front())) {
				double tmp;
				utils::simpleValueFromString (utils::getStringFromDict (ops.front(),"Time"),tmp);
				if (numeric_limits<time_t>::max() > tmp)
					time = static_cast<time_t> (tmp);
			}
			
		}catch (CObjectException&) { 
			utilsPrintDbg (debug::DBG_WARN, "Change tag is CORRUPTED!");
		}
	}
	
	return time;
}


//==========================================================
} // namespace pdfobjects
//==========================================================
