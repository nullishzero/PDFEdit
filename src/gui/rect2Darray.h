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
#ifndef __RECT2DARRAY_H__
#define __RECT2DARRAY_H__

#include <qtcompat.h>
#include <qrect.h>
#include QPTRLIST
#include QPTRCOLLECTION

namespace gui {

/* ************************************************************************************************
 *                                 BBoxOfObjectOnPage
 * ************************************************************************************************/

/*-
 */
template <typename T>
class BBoxOfObjectOnPage : public QRect {
	public:
		BBoxOfObjectOnPage();
		BBoxOfObjectOnPage( const BBoxOfObjectOnPage & b);
		BBoxOfObjectOnPage( const QRect &r, T po );			// r must be normalized
		virtual ~BBoxOfObjectOnPage();

		void setUpBBox( BBoxOfObjectOnPage<T> * b );
		void setDownBBox( BBoxOfObjectOnPage<T> * b );
		void setRightBBox( BBoxOfObjectOnPage<T> * b );
		void setLeftBBox( BBoxOfObjectOnPage<T> * b );
		void setNextLineFirstBBox( BBoxOfObjectOnPage<T> * b );
		void setPrevLineLastBBox( BBoxOfObjectOnPage<T> * b );
		void setFirstBBox( BBoxOfObjectOnPage<T> * b );
		void setLastBBox( BBoxOfObjectOnPage<T> * b );

		const BBoxOfObjectOnPage<T> * getUpBBox( ) const;
		const BBoxOfObjectOnPage<T> * getDownBBox( ) const;
		const BBoxOfObjectOnPage<T> * getRightBBox( ) const;
		const BBoxOfObjectOnPage<T> * getLeftBBox( ) const;
		const BBoxOfObjectOnPage<T> * getNextLineFirstBBox( ) const;
		const BBoxOfObjectOnPage<T> * getPrevLineLastBBox( ) const;
		const BBoxOfObjectOnPage<T> * getFirstBBox( ) const;
		const BBoxOfObjectOnPage<T> * getLastBBox( ) const;

		inline bool operator> ( const BBoxOfObjectOnPage<T> & second ) const {
			return (y() > second.y()) || ((y() == second.y()) && (x() > second.x()));
		};

		inline bool operator< ( const BBoxOfObjectOnPage<T> & second ) const {
			return (y() < second.y()) || ((y() == second.y()) && (x() < second.x()));
		};

		inline bool operator== ( const BBoxOfObjectOnPage<T> & second ) const {
			return (y() == second.y()) && (x() == second.x());
		};

		T getObject() const;
	private:
		T object;
		BBoxOfObjectOnPage<T>	* ptr_up,
								* ptr_down,
								* ptr_right,
								* ptr_left,
								* ptr_first,
								* ptr_last,
								* ptr_nextLineFirst,
								* ptr_prevLineLast;
};


/* ************************************************************************************************
 *                                 array of rectangles
 * ************************************************************************************************/

template <typename T>
class RectArray : public Q_PtrList< BBoxOfObjectOnPage<T> > {
	public:
		RectArray();
		~RectArray();
		void myAppend ( const BBoxOfObjectOnPage<T> * item );

		inline bool operator> ( const RectArray & second ) const {
			return getMinY() > second.getMinY();
		}
		inline bool operator< ( const RectArray & second ) const {
			return getMinY() < second.getMinY();
		}
		inline bool operator== ( const RectArray & second ) const {
			return getMinY() == second.getMinY();
		}
		int getMinY() const;
		int getMaxY() const;
		void initAllBBoxPtr( RectArray * prev, RectArray * next );
	protected:
	    virtual int compareItems( Q_PtrCollection::Item s1, Q_PtrCollection::Item s2 ) {
			if ( *((BBoxOfObjectOnPage<T>*)s1) == *((BBoxOfObjectOnPage<T>*)s2) )
				return 0;
			return ( *((BBoxOfObjectOnPage<T>*)s1) < *((BBoxOfObjectOnPage<T>*)s2) ? -1 : 1 );
		}
	
	private:
		int minY;
		int maxY;
};

/* ************************************************************************************************
 *                                 2D array of rectangles
 * ************************************************************************************************/

template <typename T>
class Rect2DArray : public Q_PtrList< RectArray<T> > {
	public:
		Rect2DArray();
		~Rect2DArray();
	
		void setAutoDeleteAll( bool ada );
		void sortAll();
		void initAllBBoxPtr();
		void myAppend( BBoxOfObjectOnPage<T> * bbox );
	protected:
	    virtual int compareItems( Q_PtrCollection::Item s1, Q_PtrCollection::Item s2 ) {
			if ( *((RectArray<T>*)s1) == *((RectArray<T>*)s2) )
				return 0;
			return ( *((RectArray<T>*)s1) < *((RectArray<T>*)s2) ? -1 : 1 );
		}
};

} // namespace gui



#include "rect2Darray.h"
#include <stdlib.h>
#include "util.h"
#include "utils/debug.h"

#include <limits>
#include <assert.h>

namespace gui {

/* ************************************************************************************************
 *                                 BBoxOfObjectOnPage
 * ************************************************************************************************/

template <typename T>
BBoxOfObjectOnPage<T>::BBoxOfObjectOnPage() :
	QRect()
{
	ptr_up = ptr_down = ptr_right = ptr_left = ptr_nextLineFirst = ptr_prevLineLast = NULL;
	// object ?;
}

template <typename T>
BBoxOfObjectOnPage<T>::BBoxOfObjectOnPage( const BBoxOfObjectOnPage<T> & b) :
	QRect(b.topLeft(),b.bottomRight())
{
	object		= b.object;
	ptr_up		= b.ptr_up;
	ptr_down	= b.ptr_down;
	ptr_right	= b.ptr_right;
	ptr_left	= b.ptr_left;
	ptr_first	= b.ptr_first;
	ptr_last	= b.ptr_last;
	ptr_nextLineFirst	= b.ptr_nextLineFirst;
	ptr_prevLineLast	= b.ptr_prevLineLast;
}

template <typename T>
BBoxOfObjectOnPage<T>::BBoxOfObjectOnPage( const QRect &r, T po ) :			// r must be normalized
	QRect(r.topLeft(),r.bottomRight())
//	QRect(r.left(),r.top(),r.width(),r.height())
{
	ptr_up = ptr_down = ptr_right = ptr_left = ptr_nextLineFirst = ptr_prevLineLast = NULL;
	object = po;
}

template <typename T>
void BBoxOfObjectOnPage<T>::setUpBBox( BBoxOfObjectOnPage * b ) {
	ptr_up = b;
}

template <typename T>
void BBoxOfObjectOnPage<T>::setDownBBox( BBoxOfObjectOnPage * b ) {
	ptr_down = b;
}

template <typename T>
void BBoxOfObjectOnPage<T>::setRightBBox( BBoxOfObjectOnPage * b ) {
	ptr_right = b;
}

template <typename T>
void BBoxOfObjectOnPage<T>::setLeftBBox( BBoxOfObjectOnPage * b ) {
	ptr_left = b;
}

template <typename T>
void BBoxOfObjectOnPage<T>::setNextLineFirstBBox( BBoxOfObjectOnPage * b ) {
	ptr_nextLineFirst = b;
}

template <typename T>
void BBoxOfObjectOnPage<T>::setPrevLineLastBBox( BBoxOfObjectOnPage * b ) {
	ptr_prevLineLast = b;
}
template <typename T>
void BBoxOfObjectOnPage<T>::setFirstBBox( BBoxOfObjectOnPage * b ) {
	ptr_first = b;
}
template <typename T>
void BBoxOfObjectOnPage<T>::setLastBBox( BBoxOfObjectOnPage * b ) {
	ptr_last = b;
}

template <typename T>
const BBoxOfObjectOnPage<T> * BBoxOfObjectOnPage<T>::getUpBBox( ) const {
	return ptr_up;
}

template <typename T>
const BBoxOfObjectOnPage<T> * BBoxOfObjectOnPage<T>::getDownBBox( ) const {
	return ptr_down;
}

template <typename T>
const BBoxOfObjectOnPage<T> * BBoxOfObjectOnPage<T>::getRightBBox( ) const {
	return ptr_right;
}

template <typename T>
const BBoxOfObjectOnPage<T> * BBoxOfObjectOnPage<T>::getLeftBBox( ) const {
	return ptr_left;
}

template <typename T>
const BBoxOfObjectOnPage<T> * BBoxOfObjectOnPage<T>::getNextLineFirstBBox( ) const {
	return ptr_nextLineFirst;
}

template <typename T>
const BBoxOfObjectOnPage<T> * BBoxOfObjectOnPage<T>::getPrevLineLastBBox( ) const {
	return ptr_prevLineLast;
}
template <typename T>
const BBoxOfObjectOnPage<T> * BBoxOfObjectOnPage<T>::getFirstBBox( ) const {
	return ptr_first;
}
template <typename T>
const BBoxOfObjectOnPage<T> * BBoxOfObjectOnPage<T>::getLastBBox( ) const {
	return ptr_last;
}


template <typename T>
T BBoxOfObjectOnPage<T>::getObject() const {
	return object;
}

template <typename T>
BBoxOfObjectOnPage<T>::~BBoxOfObjectOnPage() {
}

/* ************************************************************************************************
 *                                 array of rectangles
 * ************************************************************************************************/

template <typename T>
RectArray<T>::RectArray() :
	Q_PtrList< BBoxOfObjectOnPage<T> >()
{
	minY = std::numeric_limits<int>::max();
	maxY = std::numeric_limits<int>::min();
	this->setAutoDelete( true );
}

template <typename T>
RectArray<T>::~RectArray() {
}

template <typename T>
void RectArray<T>::myAppend ( const BBoxOfObjectOnPage<T> * item ) {
	if (! item->isNull()) {  //TODO
		minY = std::min( minY, item->top() );
		maxY = std::max( maxY, item->bottom() );
	}
	append( item );
}

template <typename T>
int RectArray<T>::getMinY() const {
	return minY;
}

template <typename T>
int RectArray<T>::getMaxY() const {
	return maxY;
}

template <typename T>
void RectArray<T>::initAllBBoxPtr( RectArray * prev, RectArray * next ) {
	BBoxOfObjectOnPage<T>	* up = NULL,
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
	Q_PtrListIterator< BBoxOfObjectOnPage<T> >	* prevLineItems = NULL,
												* nextLineItems = NULL;
	if (prev != NULL) {
		prevLineItems = new Q_PtrListIterator< BBoxOfObjectOnPage<T> > (*prev);
		prevLineLast = prevLineItems->toLast();
		curPrevLine = prevLineItems->toFirst();
	}
	if (next != NULL) {
		nextLineItems = new Q_PtrListIterator< BBoxOfObjectOnPage<T> > (*next);
		nextLineFirst = curNextLine = nextLineItems->toFirst();
	}

	Q_PtrListIterator< BBoxOfObjectOnPage<T> >	currentLine (*this);
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

template <typename T>
Rect2DArray<T>::Rect2DArray() :
	Q_PtrList< RectArray<T> >()
{
	this->setAutoDelete(true);
}

template <typename T>
Rect2DArray<T>::~Rect2DArray() {
}

template <typename T>
void Rect2DArray<T>::setAutoDeleteAll( bool ada ) {
	this->setAutoDelete( ada );
	Q_PtrListIterator< RectArray<T> > it ( *this );
	RectArray<T> * current = it.toFirst();
	while ((current = it.current()) != NULL) {
		current->setAutoDelete( ada );
		++it;
	}
}

template <typename T>
void Rect2DArray<T>::sortAll() {
	this->sort();
	Q_PtrListIterator< RectArray<T> > it ( *this );
	RectArray<T> * current = it.toFirst();
	while ((current = it.current()) != NULL) {
		current->sort();
		++it;
	}
}

template <typename T>
void Rect2DArray<T>::initAllBBoxPtr() {
	RectArray<T> * prev = NULL;
	RectArray<T> * current = NULL;
	RectArray<T> * next = NULL;

	sortAll();

	Q_PtrListIterator< RectArray<T> > it ( *this );
	current = it.toFirst();
	next = ++it;
	while (current != NULL) {
		current->initAllBBoxPtr( prev, next );
		prev = current;
		current = next;
		next = ++it;
	}
}

template <typename T>
void Rect2DArray<T>::myAppend( BBoxOfObjectOnPage<T> * bbox ) {
	if (bbox == NULL)
		return;

	bool toAppend = true;
	Q_PtrListIterator< RectArray<T> > it ( *this );
	RectArray<T> * current = it.toFirst();
	while (((current = it.current()) != NULL) && toAppend) {
		if ( current->getMinY() == bbox->top() ) {
			current->myAppend( bbox );
			toAppend = false;
		}
		++it;
	}
	if (toAppend) {
		current = new RectArray<T>();
		current->myAppend( bbox );
		append( current );
	}
}

} // namespace gui
#endif
