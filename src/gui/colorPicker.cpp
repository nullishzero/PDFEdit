#include "colorpicker.h"
#include <vector>
#include <QColor>
#include <QColorDialog>

static std::vector<QColor> colors;

//class ItemList : public QAbstractListModel
//{
//	Q_OBJECT
//public:
//	ItemList(QObject *parent = 0) : QAbstractListModel(parent) {}
//
//	int rowCount(const QModelIndex &parent = QModelIndex()) const { return 5; }
//	QVariant data(const QModelIndex &index, int role) const {
//		if (!index.isValid())
//			return QVariant();
//
//		if (role == Qt::BackgroundRole)
//			return QColor(QColor::colorNames().at(index.row()));
//
//		if (role == Qt::DisplayRole)
//			return QString("Item %1").arg(index.row() + 1);
//		else
//			return QVariant();
//	}
//};//TODO test this

ColorPicker::ColorPicker(QWidget * parent) : QWidget(parent)
{ 
	this->ui.setupUi(this);
	connect(this->ui.colors, SIGNAL(pressed()), this, SLOT(setFromDialog()));
	/*setColor(QColor(255,0,0));
	setColor(QColor(0,255,0));
	setColor(QColor(0,0,255));
	setColor(QColor(255,255,0));
	setColor(QColor(255,0,255));
	setColor(QColor(0,255,255));
	setColor(QColor(255,255,255));
	connect(this->ui.colors, SIGNAL(currentIndexChanged(int)), this, SLOT(valueChanged(int)));*/
}
void ColorPicker::setFromDialog()
{
	QColor color = QColorDialog::getColor();
	if (!color.isValid())
		return;
	setColor(color);
}
int ColorPicker::getR() 
{ 
//	return qRed(colors[ui.colors->itemData(ui.colors->currentIndex() ).toInt()].rgb()); 
	return qRed(_color.rgb());
}
int ColorPicker::getG() 
{ 
	return qGreen(_color.rgb());
	//return qGreen(colors[ui.colors->itemData(ui.colors->currentIndex() ).toInt()].rgb()); 
}
int ColorPicker::getB() 
{ 
	return qBlue(_color.rgb());
	//return qBlue(colors[ui.colors->itemData(ui.colors->currentIndex() ).toInt()].rgb()); 
}
void ColorPicker::setColor(QColor color)
{
	/*int index = -1;
	for(int i =0; i < colors.size(); i++)
	{
		if (colors[i] == color)
		{
			index = i;
			break;
		}
	}
	if (index == -1)
	{
		index = colors.size();
		colors.push_back(color);
	}
	int index2 = ui.colors->findData(QVariant(index));
	if (index2 == -1)
	{
		QPixmap pixmap(QSize(16,16));
		QPainter painter(&pixmap);
		painter.fillRect(pixmap.rect(),color);
		ui.colors->insertItem(0,pixmap,"",QVariant(index));
		index2 = ui.colors->findData(QVariant(index));
	}
	ui.colors->setCurrentIndex(index2);*/
	QPixmap pixmap(QSize(32,32));
	QPainter painter(&pixmap);
	painter.fillRect(pixmap.rect(),color);
	this->ui.colors->setIcon(pixmap);
	_color = color;
}
QColor ColorPicker::getColor()
{
	/*int index = ui.colors->itemData(ui.colors->currentIndex()).toInt(); 
	return colors[index];*/
	return _color;
}

//void ColorPicker::valueChanged(int)
//{
//	emit ValueChangedSignal(getColor());
//}
