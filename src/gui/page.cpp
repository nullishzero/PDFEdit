#include "page.h"
#include "globalfunctions.h"
#include <QPaintEvent>
#include <QPainter>
#include <QShortcut>

QRect DisplayPage::getRect()
{
	return _rect;
}
void DisplayPage::paintEvent(QPaintEvent * event)
{
	QLabel::paintEvent(event);
	switch (_mode)
	{
	case ModeDrawHighLight:
		{
			QPainter p(this);
			for ( int i = 0 ; i < _region.size(); i++)
			{
				p.setBrush(_color);
				p.drawRect(_region[i]);
			}
			break;
		}
	case ModeDrawPosition:
		{
			QPainter p(this);
			p.drawRect(_pos.x()-5, _pos.y()-2,10,4);
			p.drawRect(_pos.x()-2, _pos.y()-5,4,10);
			break;
		}
	case ModeDrawNothing: //do now draw anything
		break;
	case ModeTrackDrawingRect:
	case ModeTrackCompleteRectangle:
		{
			QPainter p(this);
			p.drawRoundedRect(_rect, 2.0,1.0);
			break;
		}
	default:
		assert(false);
	}
}
void DisplayPage::drawAndTrackRectangle(QRect rect)
{
	_mode = ModeTrackCompleteRectangle;
	_rect = rect;
	_origRect = _rect.topLeft();
	update();
}
void DisplayPage::setMode(PageDrawMode mode)
{
	_mode = mode;
}
DisplayPage::DisplayPage(QWidget *parent): QLabel(parent), _mousePressed(false),_mode(ModeDrawNothing)
{
	 setBackgroundRole(QPalette::Base);
     setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
	 setContextMenuPolicy(Qt::CustomContextMenu);
     setScaledContents(true);
	 menu = new QMenu();
	 menu->addAction("InsertText",this,SLOT(insertText()));
	 menu->addAction("Delete",this,SLOT(deleteText()));
	 menu->addAction("Erase", this, SLOT(eraseText()));
	 menu->addAction("Change",this,SLOT(changeText()));
	 menu->addAction("InsertImage",this,SLOT(insertImage()));
	 menu->addAction("DeleteImage",this,SLOT(deleteImage()));
	 menu->addAction("Annotate",this,SLOT(annotation()));
	 
	 _copy = new QShortcut(QKeySequence(tr("Ctrl+C", "Copy Text")),this);
	 /*_copy->setShortcut(QKeySequence(Qt::CTRL,Qt::Key_F));
	 addAction(_copy);*/
	 connect(_copy, SIGNAL(activated()), this, SLOT(copyText()));

	// menu->addAction("Delete Annotation",this,SLOT(deleteAnnotation()));
	 menu->addAction("Change image",this,SLOT(changeImage()));
	 setMouseTracking(true);
}

QPoint DisplayPage::convertCoord(QPoint point)
{
	return (QPoint(point.x(),this->size().height() - _pos.y()));
}
void DisplayPage::changeImage()
{
	QPoint p(_pos.x(), this->size().height() - _pos.y());
	emit ChangeImageSignal(p);//vieme o ktory obrazok ide
}
int DisplayPage::deleteAnnotation(QPoint point)
{
	//emit DeleteAnnotationSignal(_point);
	for (int i=0; i<_interactive.size(); i++ )
	{
		if (_interactive[i].contains(point))
		{
			_interactive.removeAt(i);
			return i;
		}
	}
	return -1;
}
void DisplayPage::annotation()
{
	emit AnnotationSignal(_pos); //TODO emit also rectangle
}
void DisplayPage::deleteImage()
{
	QPoint p(_pos.x(), this->size().height() - _pos.y());
	emit DeleteImageSignal(p);
}
void DisplayPage::insertImage()
{
	emit InsertImageSignal(_pos);
}
void DisplayPage::changeText()
{
	emit ChangeTextSignal();
}
void DisplayPage::eraseText()
{
	emit EraseTextSignal();
}
void DisplayPage::deleteText()
{
	emit DeleteTextSignal();
}
void DisplayPage::insertText()
{	
	emit InsertTextSignal(_pos);
}
DisplayPage::~DisplayPage(){}

void DisplayPage::setImage( const QImage & image )
{
	_image = image.copy();
	unsetImg();
}
void DisplayPage::markPosition(QPoint point)
{
	QImage resultImage(_copyImg.size(),QImage::Format_ARGB32_Premultiplied);
	QImage image("images/spot.gif");

	QPainter painter(&resultImage);
	painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
	painter.fillRect(resultImage.rect(), Qt::transparent);
	painter.drawImage(0, 0, _copyImg);
	painter.translate(point);
	painter.setCompositionMode(QPainter::CompositionMode_Source);
	painter.drawImage(0,0,image);
	painter.end();
	this->setPixmap(QPixmap::fromImage(resultImage));
}
void DisplayPage::setPixmapFromImage(QList<QRect> rects,QColor color)
{
	QImage resultImage(_copyImg.size(),QImage::Format_ARGB32_Premultiplied);

	QPainter painter(&resultImage);
	painter.setCompositionMode(QPainter::CompositionMode_Source);
	painter.fillRect(resultImage.rect(), Qt::transparent);
	for ( int i =0; i < rects.size(); i++)
		painter.fillRect(rects[i], color); //TODO len linky
	painter.setCompositionMode(QPainter::CompositionMode_DestinationOver);
	painter.drawImage(0, 0, _copyImg);
	painter.end();
	this->setPixmap(QPixmap::fromImage(resultImage));
}
//void DisplayPage::fillRect( QVector<QRect>& r, const QColor color)
//{
//	QImage resultImage(_copyImg.size(),QImage::Format_ARGB32_Premultiplied);
//	QPainter painter(&resultImage);
//	painter.setCompositionMode(QPainter::CompositionMode_Source);
//	painter.fillRect(resultImage.rect(), Qt::transparent);
//	for ( int i= 0; i < r.size(); i++)
//	{
//		painter.fillRect(r[i], color);
//	}
//	painter.setCompositionMode(QPainter::CompositionMode_DestinationOver);
//	painter.drawImage(0, 0, _copyImg);
//
//	painter.end();
//	this->setPixmap(QPixmap::fromImage(resultImage));
//}
	
void DisplayPage::setPixmapFromImage( int x1, int y1, int x2, int y2, const QColor color)
{
//	QPainter painter(&_copyImg); //mozno az na this?
	int sx = (x1 > x2) ? x1 - x2:x2 - x1;
	int sy = (y1 > y2) ? y1 - y2:y2 - y1;
	QRect r(min(x1,x2),min(y1,y2),sx,sy);
////	painter.setCompositionMode(QPainter::CompositionMode_Source);
//	painter.fillRect(r, color);
//	setImg();
	QList<QRect> rr;
	rr.append(r);
	setPixmapFromImage(rr,color);
	//needrepaint? TODO
}
void DisplayPage::setImg() //again st from image, for removing highligh and so
{
#ifdef _DEBUG
	setPixmapFromImage(_interactive,QColor(0,255, 0, 50));
#else 
	setPixmapFromImage(QList<QRect>(),QColor(0,255, 0, 50));
#endif // _DEBUG
	this->adjustSize();
	_size = this->pixmap()->size();
}
void DisplayPage::unsetImg() //against from image, for removing highligh and so
{
	_copyImg = _image.copy();
	setImg();
}
void DisplayPage::mousePressEvent(QMouseEvent * event)
{
	_pos = event->pos();
	//ak to bol lavy button, nerob nic)
	switch(event->button())
	{
	case Qt::RightButton:
		{
			//menu->exec(event->globalPos());
			return;
		}
	case Qt::LeftButton:
		{
			//pass parent the coordinates
			//chceme suradnice vzhladom na label //TODO co ak budeme v continuous mode?
			_mousePressed = true;
			mouseMoveEvent(event);
			break;
		}
	default:
		{
			//assert(false);
			break; //not implemented
		}
	}
}
void DisplayPage::mouseMoveEvent(QMouseEvent * event)
{
	if (_mousePressed)
	{
		if (event->button() == Qt::LeftButton)
			emit MouseClicked(event->pos());
		if (_mode == ModeTrackCompleteRectangle)
		{
			//zmenime len pociatok rectanglu - ten uz mame_
			QPoint diff = _pos - event->pos();
#ifdef _DEBUG
			QPoint ttt = diff;
#endif // _DEBUG
			diff = _origRect - diff;
			_rect.moveTo(diff);
			update();
		}
		if ( _mode == ModeTrackDrawingRect )
		{
			int left = min(_pos.x(),event->pos().x());
			int top = min(_pos.y(), event->pos().y());
			_rect = QRect(left, top, abs(_pos.x() - event->pos().x()),abs(_pos.y()- event->pos().y()));
			update();
		}
		return;
	}
	QPoint p = event->pos();
	for ( int i =0; i< _interactive.size();i++)
	{
		if (_interactive[i].contains(p))
			emit ShowAnnotation(i); //pre anotacie -> popup
	}
}

void DisplayPage::mouseReleaseEvent(QMouseEvent * event)
{
	_mousePressed = false;
//	event->ignore(); //posun to parentovi
	if (event->button() == Qt::LeftButton)
		emit MouseReleased(event->pos()); //:)
	if (event->button() == Qt::RightButton)
	{
		QPoint p = event->pos();
		for ( int i =0; i< _interactive.size();i++)
		{
			if (_interactive[i].contains(p))
			{
				emit HandleLink(i); //pre anotacie -> popup
				return;
			}
		}
	}
}

void DisplayPage::addPlace( QRect r )
{
	_interactive.append(r);
	assert(_interactive.back() == r);
	//FIXME zotriedit pre lepsi pristup
	//ukaz
}

void DisplayPage::clearLabels()
{
	_interactive.clear();
}

void DisplayPage::copyText()
{
	emit copySelectedSignal();
}

int DisplayPage::getPlace( QPoint point )
{
	for ( int i =0; i < this->_interactive.size(); i++)
	{
		if (_interactive[i].contains(point))
			return i;
	}
	return -1;
}

void DisplayPage::drawCircle( QPoint point )
{
	_pos = point;
	setMode (ModeDrawPosition);
	update();
}

void DisplayPage::fillRect( QList<QRect> region, QColor color )
{
	setMode(ModeDrawHighLight);
	_region = region;
	_color = color;
}
