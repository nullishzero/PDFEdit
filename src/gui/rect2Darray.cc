#include "rect2Darray.h"
#include <stdlib.h>
#include "util.h"

namespace gui {

/* ************************************************************************************************
 *                                 BBoxOfObjectOnPage
 * ************************************************************************************************/

BBoxOfObjectOnPage::BBoxOfObjectOnPage() :
	QRect()
{
	ptr_up = ptr_down = ptr_right = ptr_left = ptr_nextLineFirst = ptr_prevLineLast = NULL;
	ptr_object = NULL;
}

BBoxOfObjectOnPage::BBoxOfObjectOnPage( const QRect &r, const void * po ) :			// r must be normalized
	QRect(r.left(),r.top(),r.width(),r.height()) 
{
	ptr_up = ptr_down = ptr_right = ptr_left = ptr_nextLineFirst = ptr_prevLineLast = NULL;
	ptr_object = po;
}

void BBoxOfObjectOnPage::setUpBBox( BBoxOfObjectOnPage * b ) {
	ptr_up = b;
}

void BBoxOfObjectOnPage::setDownBBox( BBoxOfObjectOnPage * b ) {
	ptr_down = b;
}

void BBoxOfObjectOnPage::setRightBBox( BBoxOfObjectOnPage * b ) {
	ptr_right = b;
}

void BBoxOfObjectOnPage::setLeftBBox( BBoxOfObjectOnPage * b ) {
	ptr_left = b;
}

void BBoxOfObjectOnPage::setNextLineFirstBBox( BBoxOfObjectOnPage * b ) {
	ptr_nextLineFirst = b;
}

void BBoxOfObjectOnPage::setPrevLineLastBBox( BBoxOfObjectOnPage * b ) {
	ptr_prevLineLast = b;
}

BBoxOfObjectOnPage::~BBoxOfObjectOnPage() {
}

/* ************************************************************************************************
 *                                 array of rectangles
 * ************************************************************************************************/

RectArray::RectArray() :
	QPtrList<BBoxOfObjectOnPage>()
{
	minY = std::numeric_limits<int>::max();
	setAutoDelete( true );
}

RectArray::~RectArray() {
}

void RectArray::myAppend ( const BBoxOfObjectOnPage * item ) {
	if ((item != NULL) && (! item->isNull()))
		minY = std::min( minY, item->y() );
	append( item );
}

int RectArray::getMinY() const  {
	return minY;
}

void RectArray::initAllBBoxPtr( RectArray * prev, RectArray * next ) {
	BBoxOfObjectOnPage * up = NULL,
					   * down = NULL,
					   * right = NULL,
					   * left = NULL,
					   * cur = NULL,
					   * nextLineFirst = NULL,
					   * prevLineLast = NULL,
					   * curNextLine = NULL,
					   * curPrevLine = NULL;
	QPtrListIterator<BBoxOfObjectOnPage>	* prevLineItems = NULL,
											* nextLineItems = NULL;
	if (prev != NULL) {
		prevLineItems = new QPtrListIterator<BBoxOfObjectOnPage> (*prev);
		prevLineLast = prevLineItems->toLast();
		curPrevLine = prevLineItems->toFirst();
	}
	if (next != NULL) {
		prevLineItems = new QPtrListIterator<BBoxOfObjectOnPage> (*next);
		nextLineFirst = curNextLine = next->current();
	}

	QPtrListIterator<BBoxOfObjectOnPage>	currentLine (*this);
	cur = currentLine.current();
	right = ++currentLine;
	while (cur != NULL) {
		cur->setPrevLineLastBBox( prevLineLast );
		cur->setNextLineFirstBBox( nextLineFirst );
		cur->setLeftBBox( left );
		cur->setRightBBox( right );

		while ((curPrevLine != NULL) && (cur->x() > curPrevLine->right()))
			curPrevLine = ++(* prevLineItems);
		if ((curPrevLine != NULL) &&
			((std::max(curPrevLine->left(), cur->left()) <= std::min(cur->right(), curPrevLine->right()))) ) {
			up = curPrevLine;
		} 

		while ((curNextLine != NULL) && (cur->x() > curNextLine->right()))
			curNextLine = ++(* nextLineItems);
		if ((curNextLine != NULL) &&
			((std::max(curNextLine->left(), cur->left()) <= std::min(cur->right(), curNextLine->right()))) ) {
			down = curNextLine;
		} 

		left = cur;
		cur = right;
		right = ++currentLine;
	}
}

/* ************************************************************************************************
 *                                 2D array of rectangles
 * ************************************************************************************************/

Rect2DArray::Rect2DArray() :
	QPtrList<RectArray>()
{
	setAutoDelete(true);
}

Rect2DArray::~Rect2DArray() {
}

void Rect2DArray::setAutoDeleteAll( bool ada ) {
	setAutoDelete( ada );
	QPtrListIterator<RectArray> it ( *this );
	RectArray * current;
	while ((current = it.current()) != NULL) {
		current->setAutoDelete( ada );
		++it;
	}
}

void Rect2DArray::sortAll() {
	sort();
	QPtrListIterator<RectArray> it ( *this );
	RectArray * current;
	while ((current = it.current()) != NULL) {
		current->sort();
		++it;
	}
}

void Rect2DArray::initAllBBoxPtr() {
	RectArray * prev = NULL;
	RectArray * current = NULL;
	RectArray * next = NULL;

	QPtrListIterator<RectArray> it ( *this );
	current = it.current();
	next = ++it;
	while (current != NULL) {
		current->initAllBBoxPtr( prev, next );
		prev = current;
		current = next;
		next = ++it;
	}
}

void Rect2DArray::myAppend( BBoxOfObjectOnPage * bbox ) {
	if (bbox == NULL)
		return;

	bool toAppend = true;
	QPtrListIterator<RectArray> it ( *this );
	RectArray * current;
	while (((current = it.current()) != NULL) && toAppend) {
		if ( current->getMinY() == bbox->top() ) {
			current->myAppend( bbox );
			toAppend = true;
		}
		++it;
	}
	if (toAppend) {
		current = new RectArray();
		current->myAppend( bbox );
	}
}

} // namespace gui
