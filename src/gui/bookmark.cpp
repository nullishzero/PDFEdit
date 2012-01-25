#include "bookmark.h"
#include <kernel/pdfoperatorsiter.h>
//#include <kernel/pdfedit-core-dev.h>
//#include <kernel/indiref.h>

using namespace pdfobjects;

int Bookmark::getDest()const
{
	return _page;
}

void Bookmark::setSubsection( pdfobjects::IndiRef ref ) 
{
	_ref = ref;
	_loaded = false;
	setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
}
void Bookmark::addSubsection(QTreeWidgetItem * item)
{
	this->addChild(item);
	_loaded = true;
}
pdfobjects::IndiRef Bookmark::getIndiRef()const
{
	return _ref; //TODO co ak tam nebude REG ale priamo dict? Mozne!
}
void Bookmark::setPage( int i ) 
{
	_page = i;
}

bool Bookmark::loaded() 
{
	return _loaded;
}

void Bookmark::setX( double t )
{
	_x = t;
}

void Bookmark::setY( double t )
{
	_y = t;
}

void Bookmark::setZoom( double t )
{
	_zoom = t;
}

double Bookmark::getX()
{
	return _x;
}

double Bookmark::getY()
{
	return _y;
}

double Bookmark::getZoom()
{
	return _zoom;
}

AnalyzeItem::AnalyzeItem(QTreeWidget * parent, PdfProperty prop): QTreeWidgetItem(parent), 
_hasChild(false),_isProp(true),_type(pdfobjects::pNull),_loaded(true), _prop(prop)
{
	Init();
}
AnalyzeItem::AnalyzeItem(QTreeWidgetItem * parent, PdfProperty prop): QTreeWidgetItem(parent), 
_hasChild(false),_isProp(true),_type(pdfobjects::pNull),_loaded(true), _prop(prop)
{
	Init();
}
AnalyzeItem::AnalyzeItem(QTreeWidgetItem * parent, PdfOp prop): QTreeWidgetItem(parent), 
_hasChild(false),_isProp(false),_type(pdfobjects::pNull),_loaded(true), _op(prop)
{
	Init();
}
void AnalyzeItem::Init()
{
	setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);
	if (_prop)
	{
		_type = _prop->getType();
		if (_type == pArray || _type == pDict || _type == pStream || _type == pRef)
		{
			_hasChild = true;
			_loaded = false;
			setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
		}
	}
	else
	{
		_type = pOther; //pdfoperator
		if (_op->getParametersCount() || _op->getChildrenCount())
			setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
	}
	switch (_type)
	{
	case pArray:
		setText(1, "Array");
		setText(2, "<complex value>");
		break;
	case pStream:
		setText(1, "Stream");
		setText(2, "<complex value>");
		break;
	case pDict:
		setText(1, "Dictionary");
		setText(2, "<complex value>");
		break;
	case pRef:
		{
			std::string n;
			_prop->getStringRepresentation(n);
			setText(1, "Reference");
			setText(2, QString(n.c_str()));
			break;
		}
	case pString:
		setText(1, "String");
		setText(2, utils::getValueFromSimple<CString>(_prop).c_str());
		break;
	case pName:
		setText(1, "Name");
		setText(2, utils::getValueFromSimple<CName>(_prop).c_str());
		break;
	case pReal:
		setText(1, "Real");
		setText(2, QVariant(utils::getValueFromSimple<CReal>(_prop)).toString());
		break;
	case pInt:
		setText(1, "Integer");
		setText(2, QVariant(utils::getValueFromSimple<CInt>(_prop)).toString());
		break;
	case pBool:
		setText(1, "Boolean");
		setText(2, QVariant(utils::getValueFromSimple<CBool>(_prop)).toString());
		break;
	default:
		setText(1,"Operator");//TODO nejaky popisok loadujuci sa zo suboru
		setText(2,"<complex value>");
	}
	if (_type!= pOther)
	{
		IndiRef ind = _prop->getIndiRef();
		setText(3,QString("( gen: ")+ QVariant(ind.gen).toString() + QString(", num:") + QVariant(ind.num).toString() + QString(")"));
	}
	else
		setText(3,"-");
}

bool AnalyzeItem::loaded()
{
	return _loaded;
}

void AnalyzeItem::load()
{
	if (loaded())
		return;
	_loaded = true;
	if (_type == pOther)
		loadOperator();
	else
		loadProperty();
}

void AnalyzeItem::loadOperator()
{
	PdfOperator::Operands ops;
	_op->getParameters(ops);
	for ( int i =0; i< ops.size(); i++)
	{
		AnalyzeItem * item =  new AnalyzeItem(this, ops[i]);
		addChild((item));
	}
	pdfobjects::PdfOperator::PdfOperators operators;
	_op->getChildren(operators);
	for ( pdfobjects::PdfOperator::PdfOperators::iterator iter = operators.begin(); iter != operators.end(); iter++)
	{
		AnalyzeItem * item = new AnalyzeItem(this, *iter);
		addChild((item));
	}
}

void AnalyzeItem::loadProperty()
{
	//beware of cstream!
	switch (_type)
	{
	case pArray:
		{
			boost::shared_ptr<pdfobjects::CArray> arr = _prop->getSmartCObjectPtr<pdfobjects::CArray>(_prop);
			for ( int  i =0; i < arr->getPropertyCount(); i++)
			{
				AnalyzeItem * item = new AnalyzeItem(this, arr->getProperty(i));
				addChild(item);
				item->setText(0,QVariant(i).toString());
			}
			break;
		}
	case pStream: //TODO operators in cstream
		{
			boost::shared_ptr<pdfobjects::CStream> dict = _prop->getSmartCObjectPtr<pdfobjects::CStream>(_prop);
			std::vector<std::string> names;
			dict->getAllPropertyNames(names);
			for ( int i =0; i< names.size(); i++)
			{
				AnalyzeItem * item = new AnalyzeItem(this, dict->getProperty(names[i]));
				addChild(item);
				item->setText(0,QString(names[i].c_str()));
			}
			break;
		}
	case pDict:
		{
			boost::shared_ptr<pdfobjects::CDict> dict = _prop->getSmartCObjectPtr<pdfobjects::CDict>(_prop);
			std::vector<std::string> names;
			dict->getAllPropertyNames(names);
			for ( int i =0; i< names.size(); i++)
			{
				AnalyzeItem * item = new AnalyzeItem(this, dict->getProperty(names[i]));
				addChild(item);
				item->setText(0,QString(names[i].c_str()));
			}
			break;
		}
	case pRef:
		{
			PdfProperty p = utils::getReferencedObject(_prop);
			AnalyzeItem * item = new AnalyzeItem(this, p);
			addChild(item);
			item->setText(0,"Reference");
			break;
		}
		//TODO
	default:
		assert(false);
	}
}
