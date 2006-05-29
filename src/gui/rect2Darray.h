#ifndef __RECT2DARRAY_H__
#define __RECT2DARRAY_H__

#include <qrect.h>
#include <qptrlist.h>

namespace gui {

/* ************************************************************************************************
 *                                 BBoxOfObjectOnPage
 * ************************************************************************************************/

class BBoxOfObjectOnPage : public QRect {
	public:
		BBoxOfObjectOnPage();
		BBoxOfObjectOnPage( const QRect &r, const void * po = NULL );			// r must be normalized
		virtual ~BBoxOfObjectOnPage();

		void setUpBBox( BBoxOfObjectOnPage * b );
		void setDownBBox( BBoxOfObjectOnPage * b );
		void setRightBBox( BBoxOfObjectOnPage * b );
		void setLeftBBox( BBoxOfObjectOnPage * b );
		void setNextLineFirstBBox( BBoxOfObjectOnPage * b );
		void setPrevLineLastBBox( BBoxOfObjectOnPage * b );

		inline bool operator> ( const BBoxOfObjectOnPage & second ) const {
			return x() > second.x();
		};

		inline bool operator< ( const BBoxOfObjectOnPage & second ) const {
			return x() < second.x();
		};

		inline bool operator== ( const BBoxOfObjectOnPage & second ) const {
			return x() == second.x();
		};
	private:
		const void * ptr_object;
		BBoxOfObjectOnPage * ptr_up,
						   * ptr_down,
						   * ptr_right,
						   * ptr_left,
						   * ptr_nextLineFirst,
						   * ptr_prevLineLast;
};


/* ************************************************************************************************
 *                                 array of rectangles
 * ************************************************************************************************/

class RectArray : public QPtrList<BBoxOfObjectOnPage> {
	public:
		RectArray();
		~RectArray();
		void myAppend ( const BBoxOfObjectOnPage * item );

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
		void initAllBBoxPtr( RectArray * prev, RectArray * next );
	private:
		int minY;
};

/* ************************************************************************************************
 *                                 2D array of rectangles
 * ************************************************************************************************/

class Rect2DArray : public QPtrList< RectArray > {
	public:
		Rect2DArray();
		~Rect2DArray();
	
		void setAutoDeleteAll( bool ada );
		void sortAll();
		void initAllBBoxPtr();
		void myAppend( BBoxOfObjectOnPage * bbox );
};

} // namespace gui

#endif
