/** \file page.h define widget for displaing content of pdf */ 
#ifndef DISPLAY_PAGE_H
#define DISPLAY_PAGE_H

#include <QScrollArea>
#include <QMouseEvent>
#include <QShortcut>
#include <QWheelEvent>
#include <QMenu>
#include "ui_page.h"
#include "typedefs.h"

/** \brief main class representing the core do the shown document*/
/** this class contains generated image of the rendered document. It does not preform nothing with the PDF, only wuth its rendered image. It is used for showing the result of the operation the does not change PDF. for example selecting tet, extracting */
class DisplayPage : public QLabel
{
	Q_OBJECT

private:
	/** context menu for special actions. This is not used anymore */
	QMenu * menu; 
	/** \brief mode tellonghow this core should react to the user input lije dragging mouse */
	PageDrawMode _mode;
public:
	/** \brief constructor */
	DisplayPage(QWidget *parent = 0);
	/** \brief Destructor */
	~DisplayPage();

	/** \brief sets the rectangle that should be warn. If user will move mouse after this was called, the result is changing the draw rectangle accrding to user input.*/
	void drawAndTrackRectangle( QRect rect);
	/** \brief sets rendered image of the PDF document */
	void setImage( const QImage & image);

	/** \brief draws all rect in region with selected color */
	void setPixmapFromImage(QList<QRect> rects,QColor color);

	/** \brief return given image */
	const QImage & getImage()const
	{
		return _image;
	}
	/// sets image of the pdf page
	void setImg();
	/** \brief convert coords from this user space to the space that has opposite Y axis. this is never used */
	QPoint convertCoord(QPoint point);
private:
	bool _mousePressed;
	//annotations
	QList<QRect> _interactive;
	QList<QRect> _region;
	//change handling
	QImage _image;
	QImage _copyImg;

	//we have to remeber original size
	QSize _size;
	QRect _rect;
	QShortcut * _copy;
	QPoint _pos;
	QPoint _origRect;
	QColor _color;
	public slots:
	/** \brief marks selected position */ 
	void markPosition(QPoint point);
	
	/** \brief emits for parent change selected image, not used anymore. This was used aonly for custom contect menu */
	void changeImage();
	/** \brief emits signal for raising annotation dialog. from the point the s eas clicked ic cteared rect that will be set into rect value of thhe dictionary until otherwise */
	void annotation();
	//void fillRect( QVector<QRect>& reg, const QColor color);
	/** \brief handles press events*/
	void mousePressEvent(QMouseEvent * event);
	/** \brief fills rectangle with given color */
	void setPixmapFromImage(int x, int y, int x2, int y2, const QColor color);
/** \brief used for emitting for custom menu - not used anymore*/
	void insertText();
	/** \brief used for emitting for custom menu - not used anymore*/
	void deleteText();
	/** \brief used for emitting for custom menu - not used anymore*/
	void eraseText();
	/** \brief used for emitting for custom menu - not used anymore*/
	void changeText();
	/** \brief adds interactive place */
	void addPlace(QRect r);
	/** \brief used for emitting for custom menu - not used anymore*/
	void insertImage();
	/** \brief used for emitting for custom menu - not used anymore*/
	void deleteImage();
	/** \brief sets mode od the page */
	void setMode(PageDrawMode mode);
signals:
/** \brief copy to clipboard request caught*/
	void copySelectedSignal();
	/** this is emitted if user clicked on interactive place that could be link */
	void HandleLink(int i);
	/** shows content of anntation at deklared position */
	void ShowAnnotation(int i);
	/** used when there was custom menu */
	void ChangeImageSignal(QPoint p);
	/** used when there was custom menu */
	void DeleteAnnotationSignal(QPoint);
	/** used when there was custom menu */
	void AnnotationSignal(QPoint);
	/** used when there was custom menu */
	void DeleteImageSignal(QPoint);
	/** used when there was custom menu */
	void InsertImageSignal(QPoint);
	/** used when there was custom menu */
	void ChangeTextSignal();
	/** used when there was custom menu */
	void EraseTextSignal();
	/** used when there was custom menu */
	void DeleteTextSignal();
	/** used when there was custom menu */
	void InsertTextSignal(QPoint point);
	/** used when there was custom menu */
	void MouseClicked(QPoint p); 
	/** used when there was custom menu */
	void highlightText(int, int);
	/** used when there was custom menu */
	void MouseReleased(QPoint);
protected:
/** \brief repaint widget. Not used */
	virtual void paintEvent(QPaintEvent * event);
public:
/** \brief check if there is some interaktive place an emit signal to delete annotation at found index*/
	int deleteAnnotation(QPoint p);

	/// removes image from display. no need to call this when before setImg is called
	void unsetImg();
	/** \brief handles mouse movement. */
	/** \brief this is important for tracing user input when selecting text or insertin image */ 
	void mouseMoveEvent(QMouseEvent *);
	/** \brief handle release event. usually is t command to stop tracking mouse */
	void mouseReleaseEvent(QMouseEvent * event);
	/** \brief clears all active places */
	void clearLabels();
	/** \brief returns last rectangle that was set */
	QRect getRect();

public slots:
/** \brief copies text to the clipboard */
	void copyText();
/** \brief check is there is some interactove place */
	/** \return -1 if there is no interactive place, otherwise return index aof the active place. This index corresponds with the TabPage supported annotations list */ 
	int getPlace( QPoint point );
	/** \brief draws has on the place where text should be inserted. This is very ugly called method */
	void drawCircle( QPoint point );
	/// sets mode to fill regions. This is called when highlight text is performed
	void fillRect( QList<QRect> region, QColor color );
};

#endif // PAGE_H
