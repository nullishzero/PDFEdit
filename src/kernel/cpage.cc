/*
 * =====================================================================================
 *        Filename:  cpage.cc
 *     Description:  Pdf page object implemetation.
 *         Created:  03/20/2006 11:41:43 AM CET
 *          Author:  jmisutka (), 
 * =====================================================================================
 */

// static
#include "static.h"

// Page
#include "cpage.h"

#include "cobjecthelpers.h"

// =====================================================================================
namespace pdfobjects{
// =====================================================================================

using namespace std;
using namespace boost;
using namespace utils;

		
//
//
//
CPage::CPage ()
{
}

//
//
//
Rectangle
CPage::getMediabox () const
{
	// Get the array representing media box
	shared_ptr<IProperty> mbox = dictionary->getProperty ("MediaBox");
	assert (pArray == mbox->getType ());
	if (pArray != mbox->getType ())
		throw MalformedFormatExeption ("Page::MediaBox is not array.");

	Rectangle rc;

  	rc.xleft  =	getDoubleFromArray (mbox, 0);
	rc.xright = getDoubleFromArray (mbox, 1);
	rc.yleft  =	getDoubleFromArray (mbox, 2);
	rc.yright = getDoubleFromArray (mbox, 3);

	return rc;
}




// =====================================================================================
} /* namespace pdfobjects */
// =====================================================================================
