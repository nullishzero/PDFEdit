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

#include "kernel/cpagechanges.h"
#include "kernel/cobject.h"
#include "kernel/cpage.h"
#include "kernel/ccontentstream.h"
#include "kernel/cpagedisplay.h"
#include "kernel/pdfoperatorsiter.h"

//==========================================================
namespace pdfobjects {
//==========================================================

using namespace std;
using namespace boost;
using namespace observer;


//
//
//
template<typename Container>
void 
CPageChanges::displayChange (::OutputDev& out, const Container& cont) const
{

	boost::shared_ptr<CDict> fakeDict (IProperty::getSmartCObjectPtr<CDict>(_page->getDictionary()->clone()));
		assert (fakeDict);

	CPageContents::setContents (fakeDict, cont);

	//
	// Beware: from this point, fakeDict is not changed so xpdf can be set
	// otherwise the condition that when pdf is valid ref must be also is not
	// met
	//
	if (hasValidPdf (_page->getDictionary()))
		fakeDict->setPdf(_page->getDictionary()->getPdf());

	// Display page using our dictionary
	_page->display()->displayPage (out, fakeDict);
}
template void CPageChanges::displayChange<vector<boost::shared_ptr<CContentStream> > > (::OutputDev& out, const vector<boost::shared_ptr<CContentStream> >& cont) const;


//
//
//
void 
CPageChanges::displayChange (::OutputDev& out, const std::vector<size_t>& cs) const
{
	Changes css;
	for (std::vector<size_t>::const_iterator it = cs.begin(); it != cs.end(); ++it)
		css.push_back (_page->contents()->getContentStream (*it));
	displayChange (out, css);
}




//
//
//
boost::shared_ptr<CContentStream>
CPageChanges::getChange (size_t nthchange) const
{
	typedef vector<boost::shared_ptr<CContentStream> > CCs;
	
	CCs ccs;
	getChanges (ccs);
		if (nthchange >= ccs.size())
			throw OutOfRange ();
	
	return ccs[nthchange];
}

//
//
//
size_t
CPageChanges::getChangeCount () const
{
	Changes chs;
	getChanges (chs);
	return chs.size();
}


// =====================================================================================
namespace {
// =====================================================================================
	
	/** 
	 * Sort according to the time of change. 
	 * Least means the change was the last one.
	 */
	struct ccs_change_sorter 
	{
		bool operator() (boost::shared_ptr<CContentStream> frst, boost::shared_ptr<CContentStream> scnd)
		{
			typedef vector<boost::shared_ptr<PdfOperator> > Ops;
			static const bool FIRST_IS_OUR_LAST = true;
			static const bool SECOND_IS_OUR_LAST = false;
			
			Ops opFrst, opScnd;

			frst->getPdfOperators (opFrst);
			scnd->getPdfOperators (opScnd);
				assert (!opFrst.empty());
				assert (!opScnd.empty());
			
			ChangePdfOperatorIterator itFrst = PdfOperator::getIterator<ChangePdfOperatorIterator> (opFrst.front());
			ChangePdfOperatorIterator itScnd = PdfOperator::getIterator<ChangePdfOperatorIterator> (opScnd.front());
				assert (itFrst.valid());
				assert (itScnd.valid());

			time_t tmfrst = ContentsChangeTag::getTime (itFrst.getCurrent());
			time_t tmscnd = ContentsChangeTag::getTime (itScnd.getCurrent());
			
			return (tmfrst > tmscnd) ? FIRST_IS_OUR_LAST : SECOND_IS_OUR_LAST;
		}
	};

// =====================================================================================
} // namespace
// =====================================================================================

//
//
//
void
CPageChanges::getChanges (Changes& cont) const
{
	cont.clear();
	typedef Changes CCs;
	CCs ccs;
	_page->contents()->getContentStreams (ccs);
	for (CCs::const_iterator it = ccs.begin(); it != ccs.end(); ++it)
	{
		vector<boost::shared_ptr<PdfOperator> > ops;
		(*it)->getPdfOperators (ops);
			// Empty contentstream is not our change
			if (ops.empty())
				continue;
		
		ChangePdfOperatorIterator chng = PdfOperator::getIterator<ChangePdfOperatorIterator> (ops.front());
		// Not containing our change tag meaning not our change
		if (!chng.valid()) 
			continue;

		cont.push_back (*it);
	}
	sort (cont.begin(), cont.end(), ccs_change_sorter());
}


//==========================================================
} // namespace pdfobjects
//==========================================================
