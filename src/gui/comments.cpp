#include "comments.h"
#include "kernel/factories.h"
#include "globalfunctions.h"
#include "typedefs.h"
#include "kernel/factories.h"
#include "kernel/cdict.h"
#include "kernel/cobjecthelpers.h"
#include <QMessageBox>

void Comments::showEvent ( QShowEvent * event )
{
	this->ui.content->clear();
	this->setWindowTitle("Add annotation");
}
void Comments::setIndex( int i )
{
	_change = false;
	_index = i;
	switch (i)
	{
	case AText:
		this->ui.linkLabel->hide();
		this->ui.hFrame->hide();
		break;
	case AHighlight:
		this->ui.linkLabel->hide();
		this->ui.hFrame->show();
		break;
	case ALink:
		this->ui.linkLabel->show();
		this->ui.hFrame->hide();
		break;
	}
	onChange(i);
}
//toto sa zavola vzdy iva vtedy, ked ine o novy anotaciu
void Comments::setRectangle(float x, float y, int width, int height)
{
	onChange(_index);
	//konvert do nasich suradnic
	_rect.yleft = y-height; //big line
	_rect.xleft = x; //big line
	_rect.yright = y; //big line
	_rect.xright = x+width; //big line
	pdfobjects::CArray arr;
	arr.addProperty(*boost::shared_ptr<pdfobjects::IProperty>(pdfobjects::CRealFactory::getInstance(_rect.xleft)));
	arr.addProperty(*boost::shared_ptr<pdfobjects::IProperty>(pdfobjects::CRealFactory::getInstance(_rect.yleft)));
	arr.addProperty(*boost::shared_ptr<pdfobjects::IProperty>(pdfobjects::CRealFactory::getInstance(_rect.xright)));
	arr.addProperty(*boost::shared_ptr<pdfobjects::IProperty>(pdfobjects::CRealFactory::getInstance(_rect.yright)));
	_an->getDictionary()->setProperty("Rect",arr);
#ifdef _DEBUG
	std::string m;
	_an->getDictionary()->getStringRepresentation(m);
#endif // _DEBUG
}
void Comments::setRectangle(libs::Rectangle rectangle)
{
	_rect = rectangle;
	pdfobjects::CArray arr;
	arr.addProperty(*boost::shared_ptr<pdfobjects::IProperty>(pdfobjects::CRealFactory::getInstance(_rect.xleft)));
	arr.addProperty(*boost::shared_ptr<pdfobjects::IProperty>(pdfobjects::CRealFactory::getInstance(_rect.yleft)));
	arr.addProperty(*boost::shared_ptr<pdfobjects::IProperty>(pdfobjects::CRealFactory::getInstance(_rect.xright)));
	arr.addProperty(*boost::shared_ptr<pdfobjects::IProperty>(pdfobjects::CRealFactory::getInstance(_rect.yright)));
	_an->getDictionary()->setProperty("Rect",arr);
}
Comments::Comments(std::string name) :_index(0),_change(false),_name(name)
{
	_inits.push_back(InitName("Text", CAInit(new pdfobjects::utils::TextAnnotInitializer())));
	_inits.push_back(InitName("Link",CAInit(new pdfobjects::utils::LinkAnnotInitializer()))) ;
	_inits.push_back(InitName("Highlight",CAInit(new pdfobjects::utils::UniversalAnnotInitializer())));
	ui.setupUi(this);
	ui.author->setText(_name.c_str());
	//rect sa potom este upravi podla to, ci je to link a ako velky je
	{
		char b[] = { 'N','I','O','P', '\0' };
		for ( int i = 0; b[i]!= '\0'; i++)
		{
			//ui.annotLinkStyle->addItem(QString(b[i]));//TODO
		}
	}
	//this->ui.annotType->addItem(QString("Comment"),QVariant("Text"));
	//this->ui.annotType->addItem(QString("Link"),QVariant("Link"));
	//this->ui.annotType->addItem(QString("Highlight"),QVariant("Highlight"));
	//this->ui.annotType->addItem(QString("Strikeout"),QVariant("StrikeOut"));
}
void Comments::setPoints(std::vector<float> flts)
{
	for ( size_t i = 0; i< flts.size(); i++)
		points->addProperty(*boost::shared_ptr<pdfobjects::IProperty>(pdfobjects::CRealFactory::getInstance(flts[i])));
}
void Comments::onChange(int index)
{
	pdfobjects::CAnnotation::setAnnotInitializator(_inits[index].init);
	//prida ten typ na urcity rect
	_an = pdfobjects::CAnnotation::createAnnotation(_rect, _inits[index].name );
	//cela annotacia sa bude diat v page
}
void Comments::insertMarkup()//TODO hihligh s  textom
{
	_index = 2;
	pdfobjects::CAnnotation::setAnnotInitializator(_inits[_index].init);
	_an = pdfobjects::CAnnotation::createAnnotation(_rect, "Highlight");
	apply();
}
void Comments::apply()
{
	close();
	//ak je to highlight, potom parsni
	//zisti, ci je to text
	ADictionary d = _an->getDictionary();
	d->setProperty("Contents", *boost::shared_ptr<pdfobjects::IProperty>(pdfobjects::CStringFactory::getInstance(std::string(ui.content->toPlainText().toAscii().data()))));
	d->setProperty("T",*boost::shared_ptr<pdfobjects::IProperty>(pdfobjects::CStringFactory::getInstance(std::string("Created by ") + ui.author->text().toStdString())));//autor
	switch (_index)
	{
	case AText: //text, obycajny
		{//mame spravne KDE ma byt, zostava zistit obsah
			d->setProperty("Open", *PdfProperty(pdfobjects::CBoolFactory::getInstance(0)));
			d->setProperty("Subj", *PdfProperty(pdfobjects::CStringFactory::getInstance("Created by pdf Editor")));
			//shared_ptr<CDict> ap = createApp
#ifdef _DEBUG
			std::string m;
			_an->getDictionary()->getStringRepresentation(m);
#endif // _DEBUG
			if (!_change)
				emit (textAnnotation (_an));
			return;
		} 
	case ALink: //co ked je to URL? - linkova anotacia, vyriesit v texte. Tot sa odkazuje oba ramci textu
		{
			d->setProperty("Subtype", *boost::shared_ptr<pdfobjects::IProperty>(pdfobjects::CNameFactory::getInstance("Link")));
			//d->addProperty
			d->setProperty("H",*boost::shared_ptr<pdfobjects::IProperty>(pdfobjects::CNameFactory::getInstance("I")));
			//optional -> border
			//kam to ma skocit, toto bude iba destination
			//emit (annotation (_an));
			emit WaitForPosition(_an);
			return;
		}
	case AHighlight:
//	case AUnderline:
//	case AStrike:
		{
			//d->setProperty("Contents", *boost::shared_ptr<pdfobjects::IProperty>(pdfobjects::CStringFactory::getInstance("Content of Highligh")));
			d->setProperty("Subtype", *boost::shared_ptr<pdfobjects::IProperty>(pdfobjects::CNameFactory::getInstance(_inits[_index].name)));
			//farba
			pdfobjects::CArray arr;
			QColor color = this->ui.hColor->getColor();
			arr.addProperty(*boost::shared_ptr< pdfobjects::CReal>(pdfobjects::CRealFactory::getInstance(color.redF()))); //add color
			arr.addProperty(*boost::shared_ptr< pdfobjects::CReal>(pdfobjects::CRealFactory::getInstance(color.greenF()))); //add color
			arr.addProperty(*boost::shared_ptr< pdfobjects::CReal>(pdfobjects::CRealFactory::getInstance(color.blueF()))); //add color
			d->setProperty("C",arr);
			d->setProperty("F", *boost::shared_ptr<pdfobjects::CInt>(pdfobjects::CIntFactory::getInstance(4)));
			{
				pdfobjects::CArray rect;
				rect.addProperty(*(PdfProperty(pdfobjects::CIntFactory::getInstance(0))));
				rect.addProperty(*(PdfProperty(pdfobjects::CIntFactory::getInstance(0))));
				rect.addProperty(*(PdfProperty(pdfobjects::CIntFactory::getInstance(1))));
				d->setProperty("Border",rect); 
			}
			emit (annotationTextMarkup (_an)); //jedine, co chyba doplnit, je quadpoint -> doplni sa v taboage
			return;
		}
	default:
		throw "unimplemented";
	}
	assert(false);
	return;
}
void Comments::setDestination(pdfobjects::IndiRef ref)
{
	arr = boost::shared_ptr<pdfobjects::CArray>(new pdfobjects::CArray());
	//cislo spranky,resp refereci na bject
	arr->addProperty(*boost::shared_ptr<pdfobjects::IProperty>(pdfobjects::CRefFactory::getInstance(ref)));
	arr->addProperty(*boost::shared_ptr<pdfobjects::IProperty>(pdfobjects::CNameFactory::getInstance(std::string("Fit"))));
	arr->addProperty(*boost::shared_ptr<pdfobjects::IProperty>(pdfobjects::CIntFactory::getInstance(0)));
}//TODO pre zaciatok, potom bude aj na konkretnu cast dokumentu

void Comments::addLink( PdfAnnot an, pdfobjects::IndiRef ref, float x, float y )
{
	pdfobjects::CArray arr;
	arr.addProperty(*PdfProperty(pdfobjects::CRefFactory::getInstance(ref)));
	arr.addProperty(*PdfProperty(pdfobjects::CNameFactory::getInstance("XYZ")));
	arr.addProperty(*PdfProperty(pdfobjects::CIntFactory::getInstance(x)));
	arr.addProperty(*PdfProperty(pdfobjects::CIntFactory::getInstance(y)));
	arr.addProperty(*PdfProperty(pdfobjects::CIntFactory::getInstance(1)));
	//page, fitR, fitH..., 3 cisla. Cislo stranky dostane z PDF dokumentu
	an->getDictionary()->setProperty("Dest",arr); //array bude teraz platne, bolo nastavene pocas dialogu
#ifdef _DEBUG
	std::string m;
	an->getDictionary()->getStringRepresentation(m);
#endif // _DEBUG
//	emit annotation(_an);
}
//len pre textovu anotaciu
void Comments::fromDict( boost::shared_ptr<pdfobjects::CDict> annDict )
{
	PdfProperty p = annDict->getProperty("Contents");
	std::string str = pdfobjects::utils::getValueFromSimple<pdfobjects::CString>(p);
	QString s = QString::fromStdString(str);
	//ui.text->setText(s);
	boost::shared_ptr<pdfobjects::CDict> d = pdfobjects::IProperty::getSmartCObjectPtr<pdfobjects::CDict>(annDict->clone());
	/*d->delProperty("Subj");
	d->delProperty("T");*/
}

void Comments::loadAnnotation( PdfAnnot _annots )
{
	//check if we support this
	std::string type;
	type = pdfobjects::utils::getNameFromDict("Subtype",_annots->getDictionary());
	if ( type!= "Highlight" && type!= "Text" ) //link sa zmaze a prepise znova
		QMessageBox::warning(this, "Unsupported Annotation","This annotation can't be changed",QMessageBox::Ok,QMessageBox::Ok);
	_change = true;
	if (type == "Text")
		setIndex(AText);
	else if (type == "Highlight")
		setIndex(AHighlight);
	else if (type == "Link")
		setIndex(ALink);
	if (_annots->getDictionary()->containsProperty("Contents"))
	{
		type = pdfobjects::utils::getStringFromDict("Contents",_annots->getDictionary());
		this->ui.content->setText(QString(type.c_str()));
	}
	show();
	this->_an = _annots;
	this->setWindowTitle("Changing annotation");

}

QColor Comments::getHColor()
{
	QColor color(this->ui.hColor->getR(),this->ui.hColor->getG(),this->ui.hColor->getB(),50);
	return color;
}

void Comments::setHColor( QColor colo )
{
	this->ui.hColor->setColor(colo);
}
