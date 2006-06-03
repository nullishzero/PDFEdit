#include "rect2Darray.h"
#include <stdlib.h>
#include "util.h"
#include "debug.h"

#include <limits>
#include <assert.h>

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

BBoxOfObjectOnPage::BBoxOfObjectOnPage( BBoxOfObjectOnPage & b) :
	QRect(b.topLeft(),b.bottomRight())
{
	ptr_object	= b.getPtrToObject();
	ptr_up		= b.getUpBBox();
	ptr_down	= b.getDownBBox();
	ptr_right	= b.getRightBBox();
	ptr_left	= b.getLeftBBox();
	ptr_first	= b.getFirstBBox();
	ptr_last	= b.getLastBBox();
	ptr_nextLineFirst	= b.getNextLineFirstBBox();
	ptr_prevLineLast	= b.getPrevLineLastBBox();
}

BBoxOfObjectOnPage::BBoxOfObjectOnPage( const QRect &r, const void * po ) :			// r must be normalized
	QRect(r.topLeft(),r.bottomRight())
//	QRect(r.left(),r.top(),r.width(),r.height()) 
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
void BBoxOfObjectOnPage::setFirstBBox( BBoxOfObjectOnPage * b ) {
	ptr_first = b;
}
void BBoxOfObjectOnPage::setLastBBox( BBoxOfObjectOnPage * b ) {
	ptr_last = b;
}

BBoxOfObjectOnPage * BBoxOfObjectOnPage::getUpBBox( ) {
	return ptr_up;
}

BBoxOfObjectOnPage * BBoxOfObjectOnPage::getDownBBox( ) {
	return ptr_down;
}

BBoxOfObjectOnPage * BBoxOfObjectOnPage::getRightBBox( ) {
	return ptr_right;
}

BBoxOfObjectOnPage * BBoxOfObjectOnPage::getLeftBBox( ) {
	return ptr_left;
}

BBoxOfObjectOnPage * BBoxOfObjectOnPage::getNextLineFirstBBox( ) {
	return ptr_nextLineFirst;
}

BBoxOfObjectOnPage * BBoxOfObjectOnPage::getPrevLineLastBBox( ) {
	return ptr_prevLineLast;
}
BBoxOfObjectOnPage * BBoxOfObjectOnPage::getFirstBBox( ) {
	return ptr_first;
}
BBoxOfObjectOnPage * BBoxOfObjectOnPage::getLastBBox( ) {
	return ptr_last;
}


const void * BBoxOfObjectOnPage::getPtrToObject() {
	return ptr_object;
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
	if (! item->isNull()) {  //TODO
		minY = std::min( minY, item->y() );
	}
	assert( !((minY != item->y()) && (this->count() > 0)) );
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
					   * first = NULL,
					   * last = NULL,
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
		nextLineItems = new QPtrListIterator<BBoxOfObjectOnPage> (*next);
		nextLineFirst = curNextLine = nextLineItems->toFirst();
	}

	QPtrListIterator<BBoxOfObjectOnPage>	currentLine (*this);
	cur = currentLine.toFirst();
	first = this->getFirst();
	last = this->getLast();
	right = ++currentLine;
	while (cur != NULL) {
		if (prevLineLast) {
			assert( prevLineLast->top() != cur->top() );
		}
		if (nextLineFirst) {
			assert( nextLineFirst->top() != cur->top() );
		}
		cur->setPrevLineLastBBox( prevLineLast );
		cur->setNextLineFirstBBox( nextLineFirst );
		cur->setLeftBBox( left );
		cur->setRightBBox( right );
		cur->setFirstBBox( first );
		cur->setLastBBox( last );

		while ((curPrevLine != NULL) && (cur->x() > curPrevLine->right()))
			curPrevLine = ++(* prevLineItems);
		if ((curPrevLine != NULL) &&
			((std::max(curPrevLine->left(), cur->left()) <= std::min(cur->right(), curPrevLine->right()))) ) {
			up = curPrevLine;
		} else
			up = NULL;

		while ((curNextLine != NULL) && (cur->x() > curNextLine->right()))
			curNextLine = ++(* nextLineItems);
		if ((curNextLine != NULL) &&
			((std::max(curNextLine->left(), cur->left()) <= std::min(cur->right(), curNextLine->right()))) ) {
			down = curNextLine;
		} else
			down = NULL;

		cur->setUpBBox( up );
		cur->setDownBBox( down );

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
	RectArray * current = it.toFirst();
	while ((current = it.current()) != NULL) {
		current->setAutoDelete( ada );
		++it;
	}
}

void Rect2DArray::sortAll() {
	sort();
	QPtrListIterator<RectArray> it ( *this );
	RectArray * current = it.toFirst();
	while ((current = it.current()) != NULL) {
		current->sort();
		++it;
	}
}

void Rect2DArray::initAllBBoxPtr() {
	RectArray * prev = NULL;
	RectArray * current = NULL;
	RectArray * next = NULL;

	sortAll();

	QPtrListIterator<RectArray> it ( *this );
	current = it.toFirst();
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
	RectArray * current = it.toFirst();
	while (((current = it.current()) != NULL) && toAppend) {
		if ( current->getMinY() == bbox->top() ) {
			current->myAppend( bbox );
			toAppend = false;
		}
		++it;
	}
	if (toAppend) {
		current = new RectArray();
		current->myAppend( bbox );
		append( current );
	}
}

} // namespace gui
